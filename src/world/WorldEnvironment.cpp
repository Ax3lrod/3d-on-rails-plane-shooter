#include "WorldEnvironment.h"
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>
#include <cstdlib>
#include <cmath>

WorldEnvironment::WorldEnvironment()
    : goldRingMesh(Mesh::CreateRing(2.6f, 3.2f, 16, glm::vec3(1.0f, 0.85f, 0.2f))),
      silverRingMesh(Mesh::CreateRing(2.6f, 3.2f, 16, glm::vec3(0.3f, 0.85f, 1.0f))),
      asteroidMesh(Mesh::CreateAsteroid(2.4f, glm::vec3(0.55f, 0.52f, 0.48f))),
      canyonMesh(Mesh::CreateCanyonSection(60.0f, 34.0f, 16.0f,
                                           glm::vec3(0.58f, 0.40f, 0.30f),
                                           glm::vec3(0.72f, 0.36f, 0.24f))),
      rockArchMesh(Mesh::CreateRockArch(34.0f, 16.0f, 7.0f, glm::vec3(0.65f, 0.34f, 0.24f))),
      pillarMesh(Mesh::CreatePillar(1.5f, 14.0f, glm::vec3(0.48f, 0.38f, 0.32f))),
      nextSpawnZ(0.0f),
      despawnDistBehind(60.0f) {
    // Generate initial canyon corridor ahead
    GenerateChunk(30.0f, -420.0f);
}

void WorldEnvironment::GenerateChunk(float startZ, float endZ) {
    float canyonSliceLen = 60.0f;

    // 1. Generate seamless canyon slices
    for (float z = startZ; z >= endZ; z -= canyonSliceLen) {
        canyonSlices.push_back({glm::vec3(0.0f, 0.0f, z)});
    }

    // 2. Generate massive Rock Archways spanning the canyon
    for (float z = startZ - 80.0f; z >= endZ; z -= 180.0f) {
        rockArches.push_back({glm::vec3(0.0f, 0.0f, z), 34.0f, 16.0f});

        // Golden Ring nested directly under the arch!
        rings.push_back({glm::vec3(0.0f, -0.5f, z), 3.2f, 0.0f, true, false});
    }

    // 3. Generate Hazard Monolith Pillars in the trench
    for (float z = startZ - 40.0f; z >= endZ; z -= 75.0f) {
        // Positioned in left or right half of the corridor
        float rx = ((float)rand() / RAND_MAX > 0.5f ? 1.0f : -1.0f) * (3.0f + ((float)rand() / RAND_MAX) * 5.5f);
        pillars.push_back({
            glm::vec3(rx, 0.0f, z),
            1.6f,   // Collision radius
            14.0f,  // Height
            50.0f,  // Health
            false
        });
    }

    // 4. Generate Additional Energy Rings
    for (float z = startZ - 50.0f; z >= endZ; z -= 85.0f) {
        float rx = ((float)rand() / RAND_MAX * 2.0f - 1.0f) * 9.0f;
        float ry = -4.0f + ((float)rand() / RAND_MAX) * 8.0f;
        bool isGold = ((float)rand() / RAND_MAX) > 0.55f;
        rings.push_back({glm::vec3(rx, ry, z), 3.2f, 0.0f, isGold, false});
    }

    // 5. High-altitude floating asteroid debris
    for (float z = startZ - 30.0f; z >= endZ; z -= 95.0f) {
        float rx = ((float)rand() / RAND_MAX * 2.0f - 1.0f) * 11.0f;
        float ry = 3.5f + ((float)rand() / RAND_MAX) * 4.5f;
        float radius = 1.8f + ((float)rand() / RAND_MAX) * 1.5f;
        glm::vec3 rotSpeed(
            ((float)rand() / RAND_MAX * 2.0f - 1.0f) * 40.0f,
            ((float)rand() / RAND_MAX * 2.0f - 1.0f) * 50.0f,
            ((float)rand() / RAND_MAX * 2.0f - 1.0f) * 30.0f
        );
        asteroids.push_back({
            glm::vec3(rx, ry, z),
            glm::vec3(0.0f),
            rotSpeed,
            radius,
            false
        });
    }

    nextSpawnZ = endZ;
}

void WorldEnvironment::Update(float playerZ, float dt) {
    // Generate new canyon chunks ahead as player advances
    if (playerZ - 350.0f < nextSpawnZ) {
        GenerateChunk(nextSpawnZ, nextSpawnZ - 420.0f);
    }

    // Spin rings
    for (auto& r : rings) {
        r.rotation += 90.0f * dt;
        if (r.rotation > 360.0f) r.rotation -= 360.0f;
    }

    // Rotate asteroids
    for (auto& a : asteroids) {
        a.rotation += a.rotSpeed * dt;
    }

    // Cull objects behind player
    float cullZ = playerZ + despawnDistBehind;

    canyonSlices.erase(
        std::remove_if(canyonSlices.begin(), canyonSlices.end(),
                       [cullZ](const CanyonSlice& c) { return c.position.z > cullZ + 30.0f; }),
        canyonSlices.end()
    );

    rockArches.erase(
        std::remove_if(rockArches.begin(), rockArches.end(),
                       [cullZ](const RockArchway& a) { return a.position.z > cullZ; }),
        rockArches.end()
    );

    pillars.erase(
        std::remove_if(pillars.begin(), pillars.end(),
                       [cullZ](const HazardPillar& p) { return p.position.z > cullZ || p.destroyed; }),
        pillars.end()
    );

    rings.erase(
        std::remove_if(rings.begin(), rings.end(),
                       [cullZ](const RingGate& r) { return r.position.z > cullZ || r.collected; }),
        rings.end()
    );

    asteroids.erase(
        std::remove_if(asteroids.begin(), asteroids.end(),
                       [cullZ](const AsteroidObstacle& a) { return a.position.z > cullZ || a.destroyed; }),
        asteroids.end()
    );
}

void WorldEnvironment::Draw(const Shader& shader) const {
    shader.SetInt("uUseLighting", 1);
    shader.SetFloat("uAlpha", 1.0f);

    // 1. Draw Canyon Slices
    for (const auto& cs : canyonSlices) {
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, cs.position);
        shader.SetMat4("uModel", model);
        canyonMesh.Draw(shader);
    }

    // 2. Draw Rock Arches
    for (const auto& arch : rockArches) {
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, arch.position);
        shader.SetMat4("uModel", model);
        rockArchMesh.Draw(shader);
    }

    // 3. Draw Hazard Monolith Pillars
    for (const auto& pil : pillars) {
        if (pil.destroyed) continue;
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, pil.position);
        shader.SetMat4("uModel", model);
        pillarMesh.Draw(shader);
    }

    // 4. Draw Asteroids
    for (const auto& a : asteroids) {
        if (a.destroyed) continue;
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, a.position);
        model = glm::rotate(model, glm::radians(a.rotation.x), glm::vec3(1, 0, 0));
        model = glm::rotate(model, glm::radians(a.rotation.y), glm::vec3(0, 1, 0));
        model = glm::rotate(model, glm::radians(a.rotation.z), glm::vec3(0, 0, 1));
        model = glm::scale(model, glm::vec3(a.radius / 2.4f));
        shader.SetMat4("uModel", model);
        asteroidMesh.Draw(shader);
    }

    // 5. Draw Rings (unlit glowing)
    shader.SetInt("uUseLighting", 0);
    shader.SetFloat("uAlpha", 0.95f);
    for (const auto& r : rings) {
        if (r.collected) continue;
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, r.position);
        model = glm::rotate(model, glm::radians(r.rotation), glm::vec3(0, 0, 1));
        shader.SetMat4("uModel", model);
        if (r.isGold) {
            goldRingMesh.Draw(shader);
        } else {
            silverRingMesh.Draw(shader);
        }
    }
}

void WorldEnvironment::Clear() {
    canyonSlices.clear();
    rockArches.clear();
    pillars.clear();
    rings.clear();
    asteroids.clear();
    nextSpawnZ = 0.0f;
    GenerateChunk(30.0f, -420.0f);
}
