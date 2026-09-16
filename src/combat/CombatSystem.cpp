#include "CombatSystem.h"
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>
#include <cmath>
#include <cstdlib>

// Random float helper [-1, 1]
static float RandomBipolar() {
    return ((float)rand() / RAND_MAX) * 2.0f - 1.0f;
}

// -------------------------------------------------------------
// ProjectileManager
// -------------------------------------------------------------
ProjectileManager::ProjectileManager()
    : laserMesh(Mesh::CreateLaserBolt(0.28f, 2.4f, glm::vec3(0.2f, 1.0f, 0.4f))),
      enemyLaserMesh(Mesh::CreateLaserBolt(0.35f, 2.8f, glm::vec3(1.0f, 0.2f, 0.15f))) {}

void ProjectileManager::SpawnLaser(const glm::vec3& origin, const glm::vec3& target, bool isPlayer, float speed) {
    glm::vec3 dir = glm::normalize(target - origin);
    glm::vec3 vel = dir * speed;
    glm::vec3 col = isPlayer ? glm::vec3(0.2f, 1.0f, 0.4f) : glm::vec3(1.0f, 0.25f, 0.15f);

    projectiles.push_back({
        origin,
        vel,
        col,
        isPlayer ? 0.7f : 0.85f,
        isPlayer ? 1.8f : 2.5f,
        isPlayer,
        true
    });
}

void ProjectileManager::Update(float dt) {
    for (auto& p : projectiles) {
        if (!p.active) continue;
        p.position += p.velocity * dt;
        p.lifetime -= dt;
        if (p.lifetime <= 0.0f) {
            p.active = false;
        }
    }

    projectiles.erase(
        std::remove_if(projectiles.begin(), projectiles.end(),
                       [](const Projectile& p) { return !p.active; }),
        projectiles.end()
    );
}

void ProjectileManager::Draw(const Shader& shader) const {
    shader.SetInt("uUseLighting", 0);
    shader.SetFloat("uAlpha", 1.0f);

    for (const auto& p : projectiles) {
        if (!p.active) continue;

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, p.position);

        glm::vec3 fwd = glm::normalize(p.velocity);
        glm::vec3 up = std::abs(fwd.y) > 0.99f ? glm::vec3(1, 0, 0) : glm::vec3(0, 1, 0);
        glm::vec3 right = glm::normalize(glm::cross(fwd, up));
        glm::vec3 actualUp = glm::cross(right, fwd);

        glm::mat4 rot(1.0f);
        rot[0] = glm::vec4(right, 0.0f);
        rot[1] = glm::vec4(actualUp, 0.0f);
        rot[2] = glm::vec4(-fwd, 0.0f);
        model = model * rot;

        shader.SetMat4("uModel", model);
        if (p.isPlayer) {
            laserMesh.Draw(shader);
        } else {
            enemyLaserMesh.Draw(shader);
        }
    }
}

void ProjectileManager::Clear() {
    projectiles.clear();
}

// -------------------------------------------------------------
// OrdnanceManager (Charged Shots, Smart Bombs & Shockwaves)
// -------------------------------------------------------------
OrdnanceManager::OrdnanceManager()
    : chargeBallMesh(Mesh::CreateSphere(1.2f, 10, 12, glm::vec3(0.15f, 1.0f, 0.6f))),
      bombMesh(Mesh::CreateCube(glm::vec3(1.2f, 1.2f, 2.6f), glm::vec3(0.2f, 0.7f, 1.0f))),
      shockwaveMesh(Mesh::CreateShockwave(1.0f, 0.45f, 32, glm::vec3(0.3f, 0.85f, 1.0f))) {}

void OrdnanceManager::SpawnChargedShot(const glm::vec3& origin, const glm::vec3& forwardDir,
                                       bool hasLock, const glm::vec3& lockTarget) {
    float speed = 160.0f;
    glm::vec3 dir = glm::normalize(forwardDir);

    chargedShots.push_back({
        origin,
        dir * speed,
        1.4f,       // Collision radius
        80.0f,      // Direct impact damage
        12.0f,      // AOE explosion radius
        hasLock,
        lockTarget,
        6.5f,       // Homing turn rate
        2.2f,       // Lifetime
        true
    });
}

void OrdnanceManager::SpawnSmartBomb(const glm::vec3& origin, const glm::vec3& forwardDir) {
    float speed = 75.0f;
    glm::vec3 dir = glm::normalize(forwardDir);

    smartBombs.push_back({
        origin,
        dir * speed,
        1.8f,
        2.2f, // Detonates after 2.2s or manually
        true
    });
}

void OrdnanceManager::TriggerShockwave(const glm::vec3& pos, float maxR, float dmg) {
    shockwaves.push_back({
        pos,
        1.5f,       // Starting radius
        maxR,       // Max bloom radius
        70.0f,      // Expansion velocity
        dmg,
        1.0f,
        true
    });
}

void OrdnanceManager::DetonateBomb(size_t index) {
    if (index < smartBombs.size() && smartBombs[index].active) {
        smartBombs[index].active = false;
        TriggerShockwave(smartBombs[index].position, 38.0f, 150.0f);
    }
}

void OrdnanceManager::Update(float dt) {
    // 1. Update Charged Plasma Shots (with homing steering)
    for (auto& cs : chargedShots) {
        if (!cs.active) continue;

        if (cs.hasHomingTarget) {
            glm::vec3 toTarget = cs.homingTarget - cs.position;
            float dist = glm::length(toTarget);
            if (dist > 1.0f) {
                glm::vec3 desiredDir = glm::normalize(toTarget);
                float speed = glm::length(cs.velocity);
                float turnT = 1.0f - std::exp(-cs.homingTurnSpeed * dt);
                glm::vec3 currentDir = glm::normalize(cs.velocity);
                glm::vec3 newDir = glm::normalize(glm::mix(currentDir, desiredDir, turnT));
                cs.velocity = newDir * speed;
            }
        }

        cs.position += cs.velocity * dt;
        cs.lifetime -= dt;
        if (cs.lifetime <= 0.0f) {
            cs.active = false;
        }
    }

    // 2. Update Smart Bombs
    for (size_t i = 0; i < smartBombs.size(); ++i) {
        auto& b = smartBombs[i];
        if (!b.active) continue;

        b.position += b.velocity * dt;
        b.lifetime -= dt;
        if (b.lifetime <= 0.0f) {
            DetonateBomb(i);
        }
    }

    // 3. Update Shockwaves
    for (auto& sw : shockwaves) {
        if (!sw.active) continue;

        sw.currentRadius += sw.expansionSpeed * dt;
        float progress = sw.currentRadius / sw.maxRadius;
        sw.alpha = std::max(0.0f, 1.0f - progress);

        if (sw.currentRadius >= sw.maxRadius) {
            sw.active = false;
        }
    }

    // Clean up
    chargedShots.erase(
        std::remove_if(chargedShots.begin(), chargedShots.end(),
                       [](const ChargedShot& s) { return !s.active; }),
        chargedShots.end()
    );

    smartBombs.erase(
        std::remove_if(smartBombs.begin(), smartBombs.end(),
                       [](const SmartBomb& b) { return !b.active; }),
        smartBombs.end()
    );

    shockwaves.erase(
        std::remove_if(shockwaves.begin(), shockwaves.end(),
                       [](const Shockwave& s) { return !s.active; }),
        shockwaves.end()
    );
}

void OrdnanceManager::Draw(const Shader& shader) const {
    shader.SetInt("uUseLighting", 0);

    // Draw Charged Plasma Shots
    for (const auto& cs : chargedShots) {
        if (!cs.active) continue;

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, cs.position);
        shader.SetMat4("uModel", model);
        shader.SetFloat("uAlpha", 1.0f);
        chargeBallMesh.Draw(shader);
    }

    // Draw Smart Bombs
    for (const auto& b : smartBombs) {
        if (!b.active) continue;

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, b.position);
        shader.SetMat4("uModel", model);
        shader.SetFloat("uAlpha", 1.0f);
        bombMesh.Draw(shader);
    }

    // Draw Shockwaves
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE); // Additive blending for blast bloom!
    for (const auto& sw : shockwaves) {
        if (!sw.active) continue;

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, sw.position);
        model = glm::scale(model, glm::vec3(sw.currentRadius));
        shader.SetMat4("uModel", model);
        shader.SetFloat("uAlpha", sw.alpha);
        shockwaveMesh.Draw(shader);
    }
    glDisable(GL_BLEND);
}

void OrdnanceManager::Clear() {
    chargedShots.clear();
    smartBombs.clear();
    shockwaves.clear();
}

// -------------------------------------------------------------
// ParticleSystem
// -------------------------------------------------------------
ParticleSystem::ParticleSystem()
    : particleMesh(Mesh::CreateCube(glm::vec3(0.25f), glm::vec3(1.0f, 0.8f, 0.3f))) {}

void ParticleSystem::SpawnExplosion(const glm::vec3& pos, int count, const glm::vec3& primaryColor) {
    for (int i = 0; i < count; ++i) {
        glm::vec3 dir(RandomBipolar(), RandomBipolar(), RandomBipolar());
        dir = glm::normalize(dir);
        float speed = 14.0f + ((float)rand() / RAND_MAX) * 32.0f;
        float lifetime = 0.45f + ((float)rand() / RAND_MAX) * 0.65f;
        float size = 0.35f + ((float)rand() / RAND_MAX) * 0.45f;

        glm::vec3 col = primaryColor;
        float r = (float)rand() / RAND_MAX;
        if (r > 0.65f) col = glm::vec3(1.0f, 1.0f, 0.6f);
        else if (r > 0.35f) col = glm::vec3(1.0f, 0.45f, 0.1f);

        particles.push_back({
            pos,
            dir * speed,
            col,
            lifetime,
            lifetime,
            size,
            true
        });
    }
}

void ParticleSystem::SpawnThrusterSparks(const glm::vec3& pos, const glm::vec3& shipVel) {
    for (int i = 0; i < 2; ++i) {
        glm::vec3 jitter(RandomBipolar() * 0.15f, RandomBipolar() * 0.15f, ((float)rand() / RAND_MAX) * 0.5f);
        glm::vec3 vel = shipVel * 0.2f + glm::vec3(jitter.x * 4.0f, jitter.y * 4.0f, 15.0f + jitter.z * 10.0f);
        particles.push_back({
            pos + glm::vec3(RandomBipolar() * 0.1f, RandomBipolar() * 0.1f, 0.0f),
            vel,
            glm::vec3(0.2f, 0.8f, 1.0f),
            0.18f,
            0.18f,
            0.15f,
            true
        });
    }
}

void ParticleSystem::SpawnChargeInwardSparks(const glm::vec3& centerPos, float chargeProgress) {
    int sparkCount = static_cast<int>(1 + chargeProgress * 3);
    for (int i = 0; i < sparkCount; ++i) {
        float spawnRadius = 2.4f - chargeProgress * 0.8f;
        glm::vec3 dir(RandomBipolar(), RandomBipolar(), RandomBipolar());
        dir = glm::normalize(dir);
        glm::vec3 sparkPos = centerPos + dir * spawnRadius;
        glm::vec3 inwardVel = -dir * (12.0f + chargeProgress * 8.0f);

        glm::vec3 col = (chargeProgress >= 1.0f) ? glm::vec3(0.2f, 1.0f, 0.8f) : glm::vec3(0.2f, 0.9f, 0.4f);

        particles.push_back({
            sparkPos,
            inwardVel,
            col,
            0.15f,
            0.15f,
            0.18f + chargeProgress * 0.15f,
            true
        });
    }
}

void ParticleSystem::Update(float dt) {
    for (auto& p : particles) {
        if (!p.active) continue;
        p.position += p.velocity * dt;
        p.lifetime -= dt;
        if (p.lifetime <= 0.0f) {
            p.active = false;
        }
    }

    particles.erase(
        std::remove_if(particles.begin(), particles.end(),
                       [](const Particle& p) { return !p.active; }),
        particles.end()
    );
}

void ParticleSystem::Draw(const Shader& shader) const {
    shader.SetInt("uUseLighting", 0);

    for (const auto& p : particles) {
        if (!p.active) continue;

        float lifeFraction = p.lifetime / p.maxLifetime;
        float currentScale = p.size * lifeFraction;

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, p.position);
        model = glm::scale(model, glm::vec3(currentScale));

        shader.SetMat4("uModel", model);
        shader.SetFloat("uAlpha", lifeFraction);
        particleMesh.Draw(shader);
    }
}

void ParticleSystem::Clear() {
    particles.clear();
}

// -------------------------------------------------------------
// TargetingReticle (Dual Reticle + Enemy Lock-on Diamond)
// -------------------------------------------------------------
TargetingReticle::TargetingReticle()
    : crosshairMesh(Mesh::CreateCrosshair(1.6f, glm::vec3(0.2f, 1.0f, 0.4f))),
      farPointMesh(Mesh::CreateCrosshair(0.9f, glm::vec3(1.0f, 0.85f, 0.2f))),
      lockOnMesh(Mesh::CreateLockOnDiamond(3.2f, glm::vec3(1.0f, 0.15f, 0.15f))) {}

void TargetingReticle::Draw(const Shader& shader, const glm::vec3& nearPos, const glm::vec3& farPos,
                            bool hasLockOn, const glm::vec3& lockTargetPos, float lockAngle) const {
    shader.SetInt("uUseLighting", 0);
    shader.SetFloat("uAlpha", 0.9f);

    // Near reticle
    glm::mat4 modelNear = glm::mat4(1.0f);
    modelNear = glm::translate(modelNear, nearPos);
    shader.SetMat4("uModel", modelNear);
    crosshairMesh.Draw(shader);

    // Far convergence point
    glm::mat4 modelFar = glm::mat4(1.0f);
    modelFar = glm::translate(modelFar, farPos);
    shader.SetMat4("uModel", modelFar);
    farPointMesh.Draw(shader);

    // Lock-on brackets on enemy target
    if (hasLockOn) {
        glm::mat4 modelLock = glm::mat4(1.0f);
        modelLock = glm::translate(modelLock, lockTargetPos);
        modelLock = glm::rotate(modelLock, glm::radians(lockAngle), glm::vec3(0, 0, 1));
        shader.SetMat4("uModel", modelLock);
        shader.SetFloat("uAlpha", 1.0f);
        lockOnMesh.Draw(shader);
    }
}
