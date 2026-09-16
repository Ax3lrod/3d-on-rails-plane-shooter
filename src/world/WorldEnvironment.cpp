#include "WorldEnvironment.h"
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>
#include <cstdlib>
#include <cmath>

WorldEnvironment::WorldEnvironment()
    : currentSector(SectorStage::Sector1_Canyon),
      goldRingMesh(Mesh::CreateRing(2.6f, 3.2f, 16, glm::vec3(1.0f, 0.85f, 0.2f))),
      silverRingMesh(Mesh::CreateRing(2.6f, 3.2f, 16, glm::vec3(0.3f, 0.85f, 1.0f))),
      asteroidMesh(Mesh::CreateAsteroid(2.4f, glm::vec3(0.55f, 0.52f, 0.48f))),
      canyonMesh(Mesh::CreateCanyonSection(60.0f, 38.0f, 16.0f,
                                           glm::vec3(0.10f, 0.38f, 0.82f),   // Vibrant cobalt ocean water
                                           glm::vec3(0.22f, 0.70f, 0.98f),   // Shimmering azure water crest
                                           glm::vec3(0.92f, 0.68f, 0.72f))), // Ex-Zodiac Pastel Pink Mesa
      rockArchMesh(Mesh::CreateRockArch(38.0f, 16.0f, 7.5f, glm::vec3(0.92f, 0.68f, 0.72f))),
      pillarMesh(Mesh::CreatePillar(1.5f, 14.0f, glm::vec3(0.90f, 0.75f, 0.78f))),
      relayMesh(Mesh::CreateRadarRelay(2.4f, glm::vec3(0.35f, 0.40f, 0.45f), glm::vec3(0.85f, 0.88f, 0.92f), glm::vec3(1.0f, 0.2f, 0.15f))),
      debrisMesh(Mesh::CreateSpaceDebris(14.0f, 2.0f, glm::vec3(0.45f, 0.48f, 0.52f))),
      horizonMesh(Mesh::CreateArcadeHorizon(420.0f, 180.0f,
                                            glm::vec3(0.24f, 0.55f, 0.92f),   // Sega blue skies
                                            glm::vec3(0.72f, 0.88f, 0.98f),   // Radiant ocean horizon
                                            glm::vec3(0.68f, 0.75f, 0.92f))), // Distant pastel sawtooth peaks
      domeMesh(Mesh::CreateFloatingDome(5.5f, glm::vec3(0.92f, 0.95f, 1.0f), glm::vec3(0.25f, 0.95f, 1.0f))),
      turbineMesh(Mesh::CreateWindTurbine(26.0f, 9.5f, glm::vec3(0.92f, 0.94f, 0.96f), glm::vec3(0.98f, 0.98f, 1.0f))),
      nextSpawnZ(0.0f),
      despawnDistBehind(60.0f),
      lastPlayerZ(0.0f) {
    Clear();
}

void WorldEnvironment::SetSector(SectorStage sector) {
    currentSector = sector;
    Clear();
}

int WorldEnvironment::GetDestroyedRelayCount() const {
    int count = 0;
    for (const auto& r : secretRelays) {
        if (r.destroyed) count++;
    }
    return count;
}

void WorldEnvironment::GenerateChunk(float startZ, float endZ) {
    if (currentSector == SectorStage::Sector1_Canyon) {
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
            float rx = ((float)rand() / RAND_MAX > 0.5f ? 1.0f : -1.0f) * (3.0f + ((float)rand() / RAND_MAX) * 5.5f);
            pillars.push_back({
                glm::vec3(rx, 0.0f, z),
                1.6f,   // Collision radius
                14.0f,  // Height
                50.0f,  // Health
                false
            });
        }

        // 4. Generate Energy Rings
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

        // 6. Floating High-Tech Dome Pavilions (Ex-Zodiac Image 1)
        for (float z = startZ - 60.0f; z >= endZ; z -= 140.0f) {
            float side = ((rand() % 2) == 0) ? -1.0f : 1.0f;
            floatingDomes.push_back({
                glm::vec3(side * (24.0f + ((float)rand() / RAND_MAX) * 4.0f), 8.5f, z),
                5.5f
            });
        }

        // 7. Giant Rotating Wind Turbines (Ex-Zodiac Image 3)
        for (float z = startZ - 90.0f; z >= endZ; z -= 110.0f) {
            float side = ((rand() % 2) == 0) ? -1.0f : 1.0f;
            windTurbines.push_back({
                glm::vec3(side * (18.5f + ((float)rand() / RAND_MAX) * 5.0f), -7.5f, z),
                ((float)rand() / RAND_MAX) * 360.0f,
                55.0f + ((float)rand() / RAND_MAX) * 30.0f
            });
        }
    } else {
        // Sector 2: Deep Space Debris Field
        // No canyon walls - vast open space with dense asteroid storm & tumbling space station wreckage!
        for (float z = startZ - 20.0f; z >= endZ; z -= 45.0f) {
            float rx = ((float)rand() / RAND_MAX * 2.0f - 1.0f) * 22.0f;
            float ry = ((float)rand() / RAND_MAX * 2.0f - 1.0f) * 16.0f;
            float radius = 2.0f + ((float)rand() / RAND_MAX) * 3.5f;
            glm::vec3 rotSpeed(
                ((float)rand() / RAND_MAX * 2.0f - 1.0f) * 50.0f,
                ((float)rand() / RAND_MAX * 2.0f - 1.0f) * 60.0f,
                ((float)rand() / RAND_MAX * 2.0f - 1.0f) * 40.0f
            );
            asteroids.push_back({
                glm::vec3(rx, ry, z),
                glm::vec3(0.0f),
                rotSpeed,
                radius,
                false
            });
        }

        // Industrial Space Station Wreckage Trusses
        for (float z = startZ - 60.0f; z >= endZ; z -= 90.0f) {
            float rx = ((float)rand() / RAND_MAX * 2.0f - 1.0f) * 18.0f;
            float ry = ((float)rand() / RAND_MAX * 2.0f - 1.0f) * 12.0f;
            glm::vec3 rotSpeed(
                ((float)rand() / RAND_MAX * 2.0f - 1.0f) * 25.0f,
                ((float)rand() / RAND_MAX * 2.0f - 1.0f) * 35.0f,
                ((float)rand() / RAND_MAX * 2.0f - 1.0f) * 20.0f
            );
            spaceDebris.push_back({
                glm::vec3(rx, ry, z),
                glm::vec3(0.0f),
                rotSpeed,
                7.0f
            });
        }

        // Sector 2 Energy Rings (Floating in open space)
        for (float z = startZ - 40.0f; z >= endZ; z -= 70.0f) {
            float rx = ((float)rand() / RAND_MAX * 2.0f - 1.0f) * 12.0f;
            float ry = ((float)rand() / RAND_MAX * 2.0f - 1.0f) * 9.0f;
            bool isGold = ((float)rand() / RAND_MAX) > 0.45f;
            rings.push_back({glm::vec3(rx, ry, z), 3.4f, 0.0f, isGold, false});
        }
    }

    nextSpawnZ = endZ;
}

void WorldEnvironment::Update(float playerZ, float dt) {
    lastPlayerZ = playerZ;

    // Generate new chunks ahead as player advances
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

    // Rotate space debris
    for (auto& d : spaceDebris) {
        d.rotation += d.rotSpeed * dt;
    }

    // Rotate wind turbines
    for (auto& wt : windTurbines) {
        wt.rotation += wt.rotSpeed * dt;
        if (wt.rotation > 360.0f) wt.rotation -= 360.0f;
    }

    // Pulse secret relays
    for (auto& rel : secretRelays) {
        rel.pulseTimer += dt * 3.0f;
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

    spaceDebris.erase(
        std::remove_if(spaceDebris.begin(), spaceDebris.end(),
                       [cullZ](const SpaceDebris& d) { return d.position.z > cullZ; }),
        spaceDebris.end()
    );

    floatingDomes.erase(
        std::remove_if(floatingDomes.begin(), floatingDomes.end(),
                       [cullZ](const FloatingDomeObstacle& d) { return d.position.z > cullZ; }),
        floatingDomes.end()
    );

    windTurbines.erase(
        std::remove_if(windTurbines.begin(), windTurbines.end(),
                       [cullZ](const WindTurbineObstacle& wt) { return wt.position.z > cullZ; }),
        windTurbines.end()
    );
}

void WorldEnvironment::Draw(const Shader& shader) const {
    if (currentSector == SectorStage::Sector1_Canyon) {
        // 0. Draw Distant Arcade Horizon (Vibrant Multi-Band Sky Dome & Low-Poly Sawtooth Peaks)
        shader.SetInt("uUseLighting", 0);
        shader.SetInt("uUseFog", 0);
        shader.SetFloat("uAlpha", 1.0f);
        glm::mat4 horizonModel = glm::mat4(1.0f);
        horizonModel = glm::translate(horizonModel, glm::vec3(0.0f, 0.0f, lastPlayerZ));
        shader.SetMat4("uModel", horizonModel);
        horizonMesh.Draw(shader);

        shader.SetInt("uUseLighting", 1);
        shader.SetInt("uUseFog", 1);

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

        // 4. Draw Floating High-Tech Dome Pavilions (Ex-Zodiac Image 1)
        for (const auto& dome : floatingDomes) {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, dome.position);
            shader.SetMat4("uModel", model);
            domeMesh.Draw(shader);
        }

        // 5. Draw Giant Wind Turbines along plains (Ex-Zodiac Image 3)
        for (const auto& wt : windTurbines) {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, wt.position);
            model = glm::rotate(model, glm::radians(wt.rotation), glm::vec3(0.0f, 0.0f, 1.0f));
            shader.SetMat4("uModel", model);
            turbineMesh.Draw(shader);
        }

        // 6. Draw Secret Planetary Radar Relays
        for (const auto& rel : secretRelays) {
            if (rel.destroyed) continue;
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, rel.position);
            shader.SetMat4("uModel", model);
            relayMesh.Draw(shader);
        }
    } else {
        // Sector 2: Draw Space Debris Girders
        shader.SetInt("uUseLighting", 1);
        shader.SetInt("uUseFog", 1);
        shader.SetFloat("uAlpha", 1.0f);
        for (const auto& d : spaceDebris) {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, d.position);
            model = glm::rotate(model, glm::radians(d.rotation.x), glm::vec3(1, 0, 0));
            model = glm::rotate(model, glm::radians(d.rotation.y), glm::vec3(0, 1, 0));
            model = glm::rotate(model, glm::radians(d.rotation.z), glm::vec3(0, 0, 1));
            shader.SetMat4("uModel", model);
            debrisMesh.Draw(shader);
        }
    }

    // 7. Draw Asteroids (in both sectors)
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

    // 8. Draw Rings (unlit glowing)
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

    // Restore standard lighting and fog state for subsequent passes
    shader.SetInt("uUseLighting", 1);
    shader.SetInt("uUseFog", 1);
    shader.SetFloat("uAlpha", 1.0f);
}

void WorldEnvironment::Clear() {
    canyonSlices.clear();
    rockArches.clear();
    pillars.clear();
    rings.clear();
    asteroids.clear();
    secretRelays.clear();
    spaceDebris.clear();
    floatingDomes.clear();
    windTurbines.clear();
    nextSpawnZ = 0.0f;
    lastPlayerZ = 0.0f;

    if (currentSector == SectorStage::Sector1_Canyon) {
        // Spawn 3 Secret Planetary Radar Relays at archway coordinates
        // Relay 1: Archway 1 at Z = -260
        secretRelays.push_back({glm::vec3(0.0f, 1.8f, -260.0f), 2.5f, 60.0f, 60.0f, false, 0.0f});
        // Relay 2: Archway 2 at Z = -480 (elevated on left shelf)
        secretRelays.push_back({glm::vec3(-6.5f, 3.2f, -480.0f), 2.5f, 60.0f, 60.0f, false, 0.0f});
        // Relay 3: Archway 3 at Z = -740 (tucked on right ledge)
        secretRelays.push_back({glm::vec3(5.5f, 2.4f, -740.0f), 2.5f, 60.0f, 60.0f, false, 0.0f});

        GenerateChunk(30.0f, -420.0f);
    } else {
        // Sector 2: Deep Space chunk
        GenerateChunk(30.0f, -420.0f);
    }
}
