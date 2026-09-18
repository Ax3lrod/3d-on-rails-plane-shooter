#include "BossTwinHelicopters.h"
#include "SoundManager.h"
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>
#include <cstdlib>
#include <algorithm>

BossTwinHelicopters::BossTwinHelicopters()
    : state(HeliBossState::Inactive),
      fuselageMesh(Mesh::CreateHelicopterFuselage(
          glm::vec3(0.26f, 0.32f, 0.24f), // Dark military olive camouflage
          glm::vec3(1.0f, 0.72f, 0.12f)   // Luminous gold cockpit canopy
      )),
      mainRotorMesh(Mesh::CreateHelicopterMainRotor(
          4.8f, glm::vec3(0.16f, 0.18f, 0.22f)
      )),
      tailRotorMesh(Mesh::CreateHelicopterTailRotor(
          1.3f, glm::vec3(0.18f, 0.20f, 0.24f)
      )),
      stateTime(0.0f),
      warningTimer(3.0f),
      approachProgress(0.0f),
      deathTimer(0.0f),
      nextExplosionTimer(0.0f) {
    Reset();
}

void BossTwinHelicopters::Reset() {
    state = HeliBossState::Inactive;
    stateTime = 0.0f;
    warningTimer = 3.0f;
    approachProgress = 0.0f;
    deathTimer = 0.0f;
    nextExplosionTimer = 0.0f;

    heliA.health = 350.0f;
    heliA.maxHealth = 350.0f;
    heliA.radius = 3.4f;
    heliA.destroyed = false;
    heliA.fireTimer = 0.8f;
    heliA.missileTimer = 2.0f;
    heliA.rotorAngle = 0.0f;
    heliA.orbitAngle = 0.0f;
    heliA.orbitRadius = 22.0f;
    heliA.targetHeight = 5.0f;

    heliB.health = 350.0f;
    heliB.maxHealth = 350.0f;
    heliB.radius = 3.4f;
    heliB.destroyed = false;
    heliB.fireTimer = 1.2f;
    heliB.missileTimer = 3.5f;
    heliB.rotorAngle = 0.0f;
    heliB.orbitAngle = glm::pi<float>();
    heliB.orbitRadius = 22.0f;
    heliB.targetHeight = 2.5f;
}

void BossTwinHelicopters::Spawn(float playerZ) {
    Reset();
    state = HeliBossState::Approaching;
    transform.position = glm::vec3(0.0f, 5.0f, playerZ - 210.0f);
    heliA.transform.position = transform.position + glm::vec3(-25.0f, 4.0f, 0.0f);
    heliB.transform.position = transform.position + glm::vec3( 25.0f, 2.0f, 0.0f);
}

void BossTwinHelicopters::Update(float dt, float playerZ, const glm::vec3& playerPos,
                                ProjectileManager& projectiles, ParticleSystem& particles, Camera& camera,
                                SoundManager* audio) {
    if (state == HeliBossState::Inactive || state == HeliBossState::Defeated) return;

    stateTime += dt;
    heliA.rotorAngle += 1450.0f * dt;
    heliB.rotorAngle += 1450.0f * dt;

    if (state == HeliBossState::Approaching) {
        warningTimer -= dt;
        approachProgress = std::min(1.0f, approachProgress + dt * 0.45f);
        float targetZ = playerZ - 75.0f;
        transform.position.z = glm::mix(playerZ - 210.0f, targetZ, approachProgress);

        heliA.transform.position = glm::mix(heliA.transform.position,
                                           glm::vec3(-16.0f, 5.0f, targetZ), dt * 3.0f);
        heliB.transform.position = glm::mix(heliB.transform.position,
                                           glm::vec3( 16.0f, 3.5f, targetZ), dt * 3.0f);

        if (warningTimer <= 0.0f) {
            state = HeliBossState::PincerPatrol;
        }
        return;
    }

    if (state == HeliBossState::DeathSpiral) {
        deathTimer += dt;
        nextExplosionTimer -= dt;
        if (nextExplosionTimer <= 0.0f) {
            nextExplosionTimer = 0.08f;
            glm::vec3 expPos = transform.position + glm::vec3(
                ((rand() % 100) / 50.0f - 1.0f) * 8.0f,
                ((rand() % 100) / 50.0f - 1.0f) * 4.0f,
                ((rand() % 100) / 50.0f - 1.0f) * 6.0f
            );
            particles.SpawnExplosion(expPos, 28, glm::vec3(1.0f, 0.6f, 0.1f));
            if (audio) audio->Play(SoundID::ExplosionSmall, 0.75f, 0.85f + (rand()%20)*0.01f);
        }

        heliA.transform.position.y -= 12.0f * dt;
        heliB.transform.position.y -= 12.0f * dt;
        heliA.transform.rotation.z += 360.0f * dt;
        heliB.transform.rotation.z -= 360.0f * dt;

        if (deathTimer >= 3.2f) {
            state = HeliBossState::Defeated;
            particles.SpawnExplosion(transform.position, 80, glm::vec3(1.0f, 0.8f, 0.3f));
        }
        return;
    }

    // Maintain boss center ahead of player along Z
    float targetBossZ = playerZ - 65.0f;
    transform.position.z = glm::mix(transform.position.z, targetBossZ, dt * 4.0f);

    bool aAlive = !heliA.destroyed;
    bool bAlive = !heliB.destroyed;

    if (!aAlive && !bAlive) {
        state = HeliBossState::DeathSpiral;
        camera.TriggerShake(1.2f, 0.5f);
        return;
    }

    if (aAlive ^ bAlive) {
        state = HeliBossState::EnrageSolo;
    }

    // Update individual helicopter kinematics & attacks
    auto updateHeli = [&](HelicopterUnit& h, float orbitDir, bool isAlpha) {
        if (h.destroyed) return;

        h.orbitAngle += orbitDir * (state == HeliBossState::EnrageSolo ? 1.6f : 1.1f) * dt;
        float ox = std::cos(h.orbitAngle) * h.orbitRadius;
        float oy = h.targetHeight + std::sin(h.orbitAngle * 1.5f) * 2.8f;
        float oz = transform.position.z + std::sin(h.orbitAngle) * 8.0f;

        h.transform.position = glm::mix(h.transform.position, glm::vec3(ox, oy, oz), dt * 3.5f);

        // Turn to aim minigun nose toward player
        float dx = playerPos.x - h.transform.position.x;
        float dz = playerPos.z - h.transform.position.z;
        float dy = playerPos.y - h.transform.position.y;
        float yaw = glm::degrees(std::atan2(dx, dz));
        float pitch = glm::degrees(std::atan2(dy, std::sqrt(dx*dx + dz*dz)));
        float rollBank = -std::sin(h.orbitAngle) * 28.0f;

        h.transform.rotation.y = glm::mix(h.transform.rotation.y, yaw, dt * 5.0f);
        h.transform.rotation.x = glm::mix(h.transform.rotation.x, pitch, dt * 4.0f);
        h.transform.rotation.z = glm::mix(h.transform.rotation.z, rollBank, dt * 4.0f);

        // Minigun Barrage
        h.fireTimer -= dt;
        if (h.fireTimer <= 0.0f) {
            h.fireTimer = (state == HeliBossState::EnrageSolo ? 0.12f : 0.22f);
            glm::vec3 chinMuzzle = h.transform.position + glm::vec3(0.0f, -0.6f, 2.5f);
            projectiles.SpawnLaser(chinMuzzle, playerPos, false, 95.0f);
            if (audio) audio->Play(SoundID::EnemyLaser, 0.38f, 1.15f + ((rand() % 10) * 0.02f));
        }

        // Rocket Pod Salvo
        h.missileTimer -= dt;
        if (h.missileTimer <= 0.0f) {
            h.missileTimer = (state == HeliBossState::EnrageSolo ? 1.8f : 3.2f);
            float side = isAlpha ? -1.8f : 1.8f;
            glm::vec3 podPos = h.transform.position + glm::vec3(side, -0.2f, 0.8f);
            projectiles.SpawnLaser(podPos, playerPos + glm::vec3(
                ((rand()%100)/50.0f - 1.0f)*3.0f,
                ((rand()%100)/50.0f - 1.0f)*2.0f, 0.0f), false, 75.0f);
            if (audio) audio->Play(SoundID::EnemyLaser, 0.55f, 0.85f);
        }
    };

    updateHeli(heliA,  1.0f, true);
    updateHeli(heliB, -1.0f, false);
}

void BossTwinHelicopters::Draw(const Shader& shader) const {
    if (state == HeliBossState::Inactive || state == HeliBossState::Defeated) return;

    shader.SetInt("uUseLighting", 1);
    shader.SetFloat("uAlpha", 1.0f);

    auto drawHeli = [&](const HelicopterUnit& h) {
        if (h.destroyed) return;

        // Fuselage
        glm::mat4 mBase = h.transform.GetModelMatrix();
        shader.SetMat4("uModel", mBase);
        fuselageMesh.Draw(shader);

        // Main Rotor spinning at top of cabin
        glm::mat4 mRotor = mBase;
        mRotor = glm::translate(mRotor, glm::vec3(0.0f, 1.15f, 0.3f));
        mRotor = glm::rotate(mRotor, glm::radians(h.rotorAngle), glm::vec3(0, 1, 0));
        shader.SetMat4("uModel", mRotor);
        mainRotorMesh.Draw(shader);

        // Tail Rotor spinning at vertical fin
        glm::mat4 mTail = mBase;
        mTail = glm::translate(mTail, glm::vec3(0.12f, 0.85f, -6.6f));
        mTail = glm::rotate(mTail, glm::radians(h.rotorAngle * 1.5f), glm::vec3(1, 0, 0));
        shader.SetMat4("uModel", mTail);
        tailRotorMesh.Draw(shader);
    };

    drawHeli(heliA);
    drawHeli(heliB);
}

int BossTwinHelicopters::FindLockTarget(const glm::vec3& playerPos, const glm::vec3& aimPos, glm::vec3& outLockPos) const {
    float bestDistSq = 14.0f * 14.0f;
    int target = -1;

    auto checkHeli = [&](const HelicopterUnit& h, int id) {
        if (h.destroyed) return;
        float zDist = playerPos.z - h.transform.position.z;
        if (zDist < 10.0f || zDist > 160.0f) return;
        float dx = h.transform.position.x - aimPos.x;
        float dy = h.transform.position.y - aimPos.y;
        float dsq = dx*dx + dy*dy;
        if (dsq < bestDistSq) {
            bestDistSq = dsq;
            target = id;
            outLockPos = h.transform.position;
        }
    };

    checkHeli(heliA, 1);
    checkHeli(heliB, 2);
    return target;
}

bool BossTwinHelicopters::CheckLaserHit(const glm::vec3& laserPos, float laserRadius, float damage,
                                       ParticleSystem& particles, Camera& camera, int& outScoreGained) {
    if (state == HeliBossState::Inactive || state == HeliBossState::Approaching || state == HeliBossState::Defeated) {
        return false;
    }

    auto hitHeli = [&](HelicopterUnit& h, const std::string& name) -> bool {
        if (h.destroyed) return false;
        float dist = glm::distance(laserPos, h.transform.position);
        if (dist <= (h.radius + laserRadius)) {
            h.health -= damage;
            particles.SpawnExplosion(laserPos, 6, glm::vec3(1.0f, 0.8f, 0.2f));
            outScoreGained += 50;

            if (h.health <= 0.0f) {
                h.destroyed = true;
                particles.SpawnExplosion(h.transform.position, 45, glm::vec3(1.0f, 0.5f, 0.1f));
                camera.TriggerShake(0.6f, 0.35f);
                outScoreGained += 1500;
            }
            return true;
        }
        return false;
    };

    if (hitHeli(heliA, "VIPER-1")) return true;
    if (hitHeli(heliB, "VIPER-2")) return true;
    return false;
}

void BossTwinHelicopters::ApplyShockwaveDamage(const glm::vec3& shockPos, float radius, float damage,
                                             ParticleSystem& particles, Camera& camera, int& outScoreGained) {
    if (state == HeliBossState::Inactive || state == HeliBossState::Approaching || state == HeliBossState::Defeated) {
        return;
    }

    auto shockHeli = [&](HelicopterUnit& h) {
        if (h.destroyed) return;
        float dist = glm::distance(shockPos, h.transform.position);
        if (dist <= radius) {
            h.health -= damage;
            particles.SpawnExplosion(h.transform.position, 14, glm::vec3(0.4f, 0.8f, 1.0f));
            outScoreGained += 100;
            if (h.health <= 0.0f) {
                h.destroyed = true;
                particles.SpawnExplosion(h.transform.position, 45, glm::vec3(1.0f, 0.5f, 0.1f));
                camera.TriggerShake(0.6f, 0.35f);
                outScoreGained += 1500;
            }
        }
    };

    shockHeli(heliA);
    shockHeli(heliB);
}

float BossTwinHelicopters::GetHealthRatio() const {
    float cur = (heliA.destroyed ? 0.0f : heliA.health) + (heliB.destroyed ? 0.0f : heliB.health);
    float maxH = heliA.maxHealth + heliB.maxHealth;
    return std::clamp(cur / maxH, 0.0f, 1.0f);
}
