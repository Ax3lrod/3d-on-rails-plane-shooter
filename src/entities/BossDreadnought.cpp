#include "BossDreadnought.h"
#include "SoundManager.h"
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>
#include <cmath>
#include <cstdlib>

static float RandomBipolar() {
    return ((float)rand() / RAND_MAX) * 2.0f - 1.0f;
}

BossDreadnought::BossDreadnought()
    : transform(glm::vec3(0.0f, 15.0f, -600.0f)),
      state(BossState::Inactive),
      hullMesh(Mesh::CreateBossHull(glm::vec3(0.18f, 0.20f, 0.25f), glm::vec3(0.85f, 0.20f, 0.12f))),
      turretMesh(Mesh::CreateBossTurret(glm::vec3(0.32f, 0.34f, 0.38f))),
      shieldMesh(Mesh::CreateSphere(3.2f, 10, 14, glm::vec3(0.2f, 0.7f, 1.0f))),
      coreMesh(Mesh::CreateBossCore(glm::vec3(1.0f, 0.35f, 0.1f))),
      stateTime(0.0f),
      warningTimer(3.5f),
      approachProgress(0.0f),
      attackTimer(0.0f),
      spiralAttackTimer(0.0f),
      spiralAngle(0.0f),
      deathTimer(0.0f),
      nextExplosionTimer(0.0f),
      swayTime(0.0f),
      currentTurretYaw(0.0f),
      corePulseTime(0.0f) {

    // Initialize Subsystems
    leftTurret = {
        "PORT TURRET",
        glm::vec3(-10.0f, 0.2f, 0.0f),
        glm::vec3(0.0f),
        120.0f, 120.0f,
        2.6f, false, false,
        0.8f, 1.4f
    };

    rightTurret = {
        "STARBOARD TURRET",
        glm::vec3(10.0f, 0.2f, 0.0f),
        glm::vec3(0.0f),
        120.0f, 120.0f,
        2.6f, false, false,
        1.5f, 1.4f
    };

    shieldGen = {
        "SHIELD GENERATOR",
        glm::vec3(0.0f, 3.2f, -1.0f),
        glm::vec3(0.0f),
        150.0f, 150.0f,
        2.8f, false, false,
        2.0f, 3.2f
    };

    quantumCore = {
        "QUANTUM CORE",
        glm::vec3(0.0f, 0.2f, 4.0f),
        glm::vec3(0.0f),
        350.0f, 350.0f,
        3.2f, false, true, // Protected until shields & turrets fall
        0.4f, 1.8f
    };

    UpdateSubsystemsWorldPos();
}

void BossDreadnought::Reset() {
    state = BossState::Inactive;
    stateTime = 0.0f;
    warningTimer = 3.5f;
    approachProgress = 0.0f;
    attackTimer = 0.0f;
    spiralAttackTimer = 0.0f;
    spiralAngle = 0.0f;
    deathTimer = 0.0f;
    nextExplosionTimer = 0.0f;
    swayTime = 0.0f;

    leftTurret.health = leftTurret.maxHealth;
    leftTurret.destroyed = false;
    leftTurret.invulnerable = false;
    leftTurret.fireTimer = 0.8f;

    rightTurret.health = rightTurret.maxHealth;
    rightTurret.destroyed = false;
    rightTurret.invulnerable = false;
    rightTurret.fireTimer = 1.5f;

    shieldGen.health = shieldGen.maxHealth;
    shieldGen.destroyed = false;
    shieldGen.invulnerable = false;
    shieldGen.fireTimer = 2.0f;

    quantumCore.health = quantumCore.maxHealth;
    quantumCore.destroyed = false;
    quantumCore.invulnerable = true;
    quantumCore.fireTimer = 0.4f;

    transform.position = glm::vec3(0.0f, 15.0f, -600.0f);
    transform.rotation = glm::vec3(0.0f);
    UpdateSubsystemsWorldPos();
}

void BossDreadnought::Spawn(float playerZ) {
    if (state != BossState::Inactive) return;

    state = BossState::Approaching;
    stateTime = 0.0f;
    warningTimer = 3.5f;
    approachProgress = 0.0f;

    // Start high up in the fog corridor ahead of the player
    transform.position = glm::vec3(0.0f, 18.0f, playerZ - 240.0f);
    transform.rotation = glm::vec3(10.0f, 0.0f, 0.0f);
    UpdateSubsystemsWorldPos();
}

void BossDreadnought::UpdateSubsystemsWorldPos() {
    glm::mat4 model = transform.GetModelMatrix();
    leftTurret.worldPos = glm::vec3(model * glm::vec4(leftTurret.localOffset, 1.0f));
    rightTurret.worldPos = glm::vec3(model * glm::vec4(rightTurret.localOffset, 1.0f));
    shieldGen.worldPos = glm::vec3(model * glm::vec4(shieldGen.localOffset, 1.0f));
    quantumCore.worldPos = glm::vec3(model * glm::vec4(quantumCore.localOffset, 1.0f));
}

void BossDreadnought::Update(float dt, float playerZ, const glm::vec3& playerPos,
                            ProjectileManager& projectiles, ParticleSystem& particles, Camera& camera,
                            SoundManager* audio) {
    if (state == BossState::Inactive || state == BossState::Defeated) return;

    stateTime += dt;
    swayTime += dt;
    corePulseTime += dt * 5.0f;

    UpdateSubsystemsWorldPos();

    // Calculate dynamic turret aim yaw towards player starfighter
    glm::vec3 toPlayer = playerPos - transform.position;
    currentTurretYaw = glm::degrees(std::atan2(toPlayer.x, toPlayer.z));

    if (state == BossState::Approaching) {
        warningTimer -= dt;
        approachProgress += dt / 3.5f;
        if (approachProgress > 1.0f) approachProgress = 1.0f;

        // Smooth descent into engagement formation
        float targetZ = playerZ - 82.0f;
        float targetY = 1.0f;
        transform.position.z = glm::mix(playerZ - 240.0f, targetZ, approachProgress);
        transform.position.y = glm::mix(18.0f, targetY, approachProgress);
        transform.position.x = std::sin(swayTime * 1.5f) * 2.0f;
        transform.rotation.x = glm::mix(10.0f, 0.0f, approachProgress);

        // Low frequency engine shockwaves
        if (std::fmod(stateTime, 0.6f) < dt) {
            camera.TriggerShake(0.22f, 0.15f);
        }

        if (warningTimer <= 0.0f) {
            state = BossState::Phase1_Subsystems;
            camera.TriggerShake(0.8f, 0.45f);
        }
        return;
    }

    if (state == BossState::Phase1_Subsystems) {
        // Keep combat distance locked ahead of player
        transform.position.z = playerZ - 82.0f;

        // Evasive graceful figure-8 patrol weave
        float targetX = std::sin(swayTime * 0.85f) * 6.5f;
        float targetY = std::cos(swayTime * 1.25f) * 2.5f + 1.2f;
        transform.position.x += (targetX - transform.position.x) * 3.5f * dt;
        transform.position.y += (targetY - transform.position.y) * 3.5f * dt;

        // Aerodynamic bank into turns
        transform.rotation.z = -std::cos(swayTime * 0.85f) * 14.0f;
        transform.rotation.x = std::sin(swayTime * 1.25f) * 4.0f;

        // Check if all primary subsystems destroyed
        if (leftTurret.destroyed && rightTurret.destroyed && shieldGen.destroyed) {
            state = BossState::Phase2_ExposedCore;
            quantumCore.invulnerable = false;

            // Massive shield collapse shockwave
            particles.SpawnExplosion(shieldGen.worldPos, 60, glm::vec3(0.2f, 0.9f, 1.0f));
            particles.SpawnExplosion(quantumCore.worldPos, 50, glm::vec3(1.0f, 0.5f, 0.1f));
            camera.TriggerShake(1.4f, 0.8f);
            if (audio) {
                audio->Play(SoundID::BombExplosion, 1.0f);
            }
            return;
        }

        FirePhase1Attacks(playerPos, projectiles, dt, audio);
        return;
    }

    if (state == BossState::Phase2_ExposedCore) {
        transform.position.z = playerZ - 76.0f;

        // Aggressive erratic evasive maneuvers
        float targetX = std::sin(swayTime * 1.4f) * 8.0f;
        float targetY = std::cos(swayTime * 1.8f) * 3.2f + 1.0f;
        transform.position.x += (targetX - transform.position.x) * 4.5f * dt;
        transform.position.y += (targetY - transform.position.y) * 4.5f * dt;

        transform.rotation.z = -std::cos(swayTime * 1.4f) * 24.0f;
        transform.rotation.x = std::sin(swayTime * 1.8f) * 6.0f;

        // Emit critical damage sparks from destroyed sponsons
        if (std::fmod(stateTime, 0.15f) < dt) {
            particles.SpawnExplosion(leftTurret.worldPos, 6, glm::vec3(0.3f, 0.3f, 0.3f));
            particles.SpawnExplosion(rightTurret.worldPos, 6, glm::vec3(0.3f, 0.3f, 0.3f));
        }

        // Check for core destruction
        if (quantumCore.health <= 0.0f) {
            quantumCore.destroyed = true;
            state = BossState::Phase3_DeathSpiral;
            deathTimer = 0.0f;
            nextExplosionTimer = 0.05f;
            camera.TriggerShake(1.6f, 1.2f);
            if (audio) {
                audio->Play(SoundID::BombExplosion, 1.0f);
            }
            return;
        }

        FirePhase2Attacks(playerPos, projectiles, dt, audio);
        return;
    }

    if (state == BossState::Phase3_DeathSpiral) {
        UpdateDeathSpiral(dt, particles, camera);
    }
}

void BossDreadnought::FirePhase1Attacks(const glm::vec3& playerPos, ProjectileManager& projectiles, float dt, SoundManager* audio) {
    // 1. Port Turret Heavy Dual Bursts
    if (!leftTurret.destroyed) {
        leftTurret.fireTimer -= dt;
        if (leftTurret.fireTimer <= 0.0f) {
            leftTurret.fireTimer = leftTurret.fireInterval;
            glm::vec3 muzzleL = leftTurret.worldPos + glm::vec3(-0.5f, 0.2f, 1.5f);
            glm::vec3 muzzleR = leftTurret.worldPos + glm::vec3(0.5f, 0.2f, 1.5f);
            projectiles.SpawnLaser(muzzleL, playerPos, false, 85.0f);
            projectiles.SpawnLaser(muzzleR, playerPos, false, 85.0f);
            if (audio) audio->Play(SoundID::EnemyLaser, 0.5f);
        }
    }

    // 2. Starboard Turret Heavy Dual Bursts
    if (!rightTurret.destroyed) {
        rightTurret.fireTimer -= dt;
        if (rightTurret.fireTimer <= 0.0f) {
            rightTurret.fireTimer = rightTurret.fireInterval;
            glm::vec3 muzzleL = rightTurret.worldPos + glm::vec3(-0.5f, 0.2f, 1.5f);
            glm::vec3 muzzleR = rightTurret.worldPos + glm::vec3(0.5f, 0.2f, 1.5f);
            projectiles.SpawnLaser(muzzleL, playerPos, false, 85.0f);
            projectiles.SpawnLaser(muzzleR, playerPos, false, 85.0f);
            if (audio) audio->Play(SoundID::EnemyLaser, 0.5f);
        }
    }

    // 3. Shield Generator 3-Way Spread Plasma Salvo
    if (!shieldGen.destroyed) {
        shieldGen.fireTimer -= dt;
        if (shieldGen.fireTimer <= 0.0f) {
            shieldGen.fireTimer = shieldGen.fireInterval;
            glm::vec3 forwardDir = glm::normalize(playerPos - shieldGen.worldPos);

            // Center shot
            projectiles.SpawnLaserWithDir(shieldGen.worldPos, forwardDir, false, 72.0f,
                                         glm::vec3(0.2f, 0.85f, 1.0f), 0.95f);

            // Left spread (-16 deg)
            float angleLeft = glm::radians(-16.0f);
            glm::vec3 dirLeft = glm::vec3(
                forwardDir.x * std::cos(angleLeft) - forwardDir.z * std::sin(angleLeft),
                forwardDir.y,
                forwardDir.x * std::sin(angleLeft) + forwardDir.z * std::cos(angleLeft)
            );
            projectiles.SpawnLaserWithDir(shieldGen.worldPos, glm::normalize(dirLeft), false, 72.0f,
                                         glm::vec3(0.2f, 0.85f, 1.0f), 0.95f);

            // Right spread (+16 deg)
            float angleRight = glm::radians(16.0f);
            glm::vec3 dirRight = glm::vec3(
                forwardDir.x * std::cos(angleRight) - forwardDir.z * std::sin(angleRight),
                forwardDir.y,
                forwardDir.x * std::sin(angleRight) + forwardDir.z * std::cos(angleRight)
            );
            projectiles.SpawnLaserWithDir(shieldGen.worldPos, glm::normalize(dirRight), false, 72.0f,
                                         glm::vec3(0.2f, 0.85f, 1.0f), 0.95f);

            if (audio) audio->Play(SoundID::ChargedShotFire, 0.6f);
        }
    }
}

void BossDreadnought::FirePhase2Attacks(const glm::vec3& playerPos, ProjectileManager& projectiles, float dt, SoundManager* audio) {
    // Overdrive Core Bullet Hell
    attackTimer += dt;
    spiralAttackTimer += dt;

    // Pattern 1: Rapid 5-Way Fan Spread from Reactor Core
    quantumCore.fireTimer -= dt;
    if (quantumCore.fireTimer <= 0.0f) {
        quantumCore.fireTimer = quantumCore.fireInterval;

        glm::vec3 forwardDir = glm::normalize(playerPos - quantumCore.worldPos);
        float angles[5] = {-22.0f, -11.0f, 0.0f, 11.0f, 22.0f};

        for (int i = 0; i < 5; ++i) {
            float rad = glm::radians(angles[i]);
            glm::vec3 dir(
                forwardDir.x * std::cos(rad) - forwardDir.z * std::sin(rad),
                forwardDir.y + ((i % 2 == 1) ? 0.08f : -0.05f),
                forwardDir.x * std::sin(rad) + forwardDir.z * std::cos(rad)
            );
            projectiles.SpawnLaserWithDir(quantumCore.worldPos, glm::normalize(dir), false, 78.0f,
                                         glm::vec3(1.0f, 0.35f, 0.1f), 0.9f);
        }
        if (audio) audio->Play(SoundID::ChargedShotFire, 0.75f, 1.2f);
    }

    // Pattern 2: Rotating Spiral Barrage
    if (spiralAttackTimer >= 0.14f) {
        spiralAttackTimer = 0.0f;
        spiralAngle += 0.42f;

        float sx = std::cos(spiralAngle);
        float sy = std::sin(spiralAngle);
        glm::vec3 spiralDir = glm::normalize(glm::vec3(sx * 0.45f, sy * 0.35f, 1.0f));

        projectiles.SpawnLaserWithDir(quantumCore.worldPos, spiralDir, false, 65.0f,
                                     glm::vec3(1.0f, 0.85f, 0.2f), 0.8f);
        if (audio) audio->Play(SoundID::EnemyLaser, 0.30f, 1.3f);
    }
}

void BossDreadnought::UpdateDeathSpiral(float dt, ParticleSystem& particles, Camera& camera) {
    deathTimer += dt;
    nextExplosionTimer -= dt;

    // Cascading chain explosions along fuselage, wings, and engines
    if (nextExplosionTimer <= 0.0f) {
        nextExplosionTimer = 0.08f + ((float)rand() / RAND_MAX) * 0.07f;

        glm::vec3 randOffset(
            RandomBipolar() * 11.0f,
            RandomBipolar() * 2.8f,
            RandomBipolar() * 4.5f
        );

        glm::vec3 expPos = transform.position + randOffset;
        glm::vec3 expColor = (rand() % 2 == 0) ? glm::vec3(1.0f, 0.45f, 0.1f) : glm::vec3(1.0f, 0.85f, 0.2f);
        particles.SpawnExplosion(expPos, 22, expColor);
        camera.TriggerShake(0.45f, 0.2f);
    }

    // Uncontrolled death spiral physics
    transform.rotation.z += 35.0f * dt;
    transform.rotation.x += 16.0f * dt;
    transform.position.y -= 8.5f * dt;
    transform.position.z += 10.0f * dt;

    if (deathTimer >= 4.5f) {
        // Grand finale catastrophic explosion
        particles.SpawnExplosion(transform.position, 120, glm::vec3(1.0f, 0.9f, 0.5f));
        particles.SpawnExplosion(transform.position + glm::vec3(-6.0f, 0.0f, 0.0f), 80, glm::vec3(0.3f, 0.8f, 1.0f));
        particles.SpawnExplosion(transform.position + glm::vec3(6.0f, 0.0f, 0.0f), 80, glm::vec3(1.0f, 0.4f, 0.1f));
        camera.TriggerShake(2.0f, 1.5f);
        state = BossState::Defeated;
    }
}

int BossDreadnought::FindLockTarget(const glm::vec3& playerPos, const glm::vec3& aimPos, glm::vec3& outLockPos) const {
    if (state != BossState::Phase1_Subsystems && state != BossState::Phase2_ExposedCore) {
        return -1;
    }

    std::vector<const BossWeakpoint*> candidateList;
    if (state == BossState::Phase1_Subsystems) {
        if (!leftTurret.destroyed) candidateList.push_back(&leftTurret);
        if (!rightTurret.destroyed) candidateList.push_back(&rightTurret);
        if (!shieldGen.destroyed) candidateList.push_back(&shieldGen);
    } else {
        if (!quantumCore.destroyed) candidateList.push_back(&quantumCore);
    }

    int bestIndex = -1;
    float bestDistSq = 18.0f * 18.0f;

    for (size_t i = 0; i < candidateList.size(); ++i) {
        const auto* wp = candidateList[i];
        float dx = wp->worldPos.x - aimPos.x;
        float dy = wp->worldPos.y - aimPos.y;
        float distSq = dx * dx + dy * dy;

        if (distSq < bestDistSq) {
            bestDistSq = distSq;
            bestIndex = static_cast<int>(i);
            outLockPos = wp->worldPos;
        }
    }

    return bestIndex;
}

bool BossDreadnought::CheckLaserHit(const glm::vec3& laserPos, float laserRadius, float damage,
                                   ParticleSystem& particles, Camera& camera, int& outScoreGained) {
    if (state != BossState::Phase1_Subsystems && state != BossState::Phase2_ExposedCore) {
        return false;
    }

    outScoreGained = 0;

    // Helper lambda to damage a weakpoint
    auto TryHitWeakpoint = [&](BossWeakpoint& wp, int scoreReward) -> bool {
        if (wp.destroyed || wp.invulnerable) return false;

        float dist = glm::distance(laserPos, wp.worldPos);
        if (dist <= (laserRadius + wp.radius)) {
            wp.health -= damage;
            particles.SpawnExplosion(laserPos, 9, glm::vec3(1.0f, 0.85f, 0.25f));

            if (wp.health <= 0.0f) {
                wp.health = 0.0f;
                wp.destroyed = true;
                outScoreGained += scoreReward;
                particles.SpawnExplosion(wp.worldPos, 45, glm::vec3(1.0f, 0.4f, 0.1f));
                camera.TriggerShake(0.85f, 0.4f);
            }
            return true;
        }
        return false;
    };

    if (state == BossState::Phase1_Subsystems) {
        if (TryHitWeakpoint(leftTurret, 1000)) return true;
        if (TryHitWeakpoint(rightTurret, 1000)) return true;
        if (TryHitWeakpoint(shieldGen, 1500)) return true;

        // Hitting the invulnerable core or armored hull during Phase 1 deflects
        float distCore = glm::distance(laserPos, quantumCore.worldPos);
        if (distCore <= (laserRadius + quantumCore.radius + 1.0f)) {
            // Shield spark deflection
            particles.SpawnExplosion(laserPos, 8, glm::vec3(0.2f, 0.8f, 1.0f));
            return true;
        }
    } else if (state == BossState::Phase2_ExposedCore) {
        if (TryHitWeakpoint(quantumCore, 5000)) return true;
    }

    return false;
}

void BossDreadnought::ApplyShockwaveDamage(const glm::vec3& shockPos, float radius, float damage,
                                          ParticleSystem& particles, Camera& camera, int& outScoreGained) {
    if (state != BossState::Phase1_Subsystems && state != BossState::Phase2_ExposedCore) {
        return;
    }

    outScoreGained = 0;

    auto ApplyRadialHit = [&](BossWeakpoint& wp, int scoreReward) {
        if (wp.destroyed || wp.invulnerable) return;

        float dist = glm::distance(shockPos, wp.worldPos);
        if (dist <= radius) {
            wp.health -= damage;
            particles.SpawnExplosion(wp.worldPos, 16, glm::vec3(0.3f, 0.9f, 1.0f));

            if (wp.health <= 0.0f) {
                wp.health = 0.0f;
                wp.destroyed = true;
                outScoreGained += scoreReward;
                particles.SpawnExplosion(wp.worldPos, 45, glm::vec3(1.0f, 0.45f, 0.15f));
                camera.TriggerShake(1.0f, 0.5f);
            }
        }
    };

    if (state == BossState::Phase1_Subsystems) {
        ApplyRadialHit(leftTurret, 1000);
        ApplyRadialHit(rightTurret, 1000);
        ApplyRadialHit(shieldGen, 1500);
    } else if (state == BossState::Phase2_ExposedCore) {
        ApplyRadialHit(quantumCore, 5000);
    }
}

float BossDreadnought::GetTotalHealth() const {
    return leftTurret.health + rightTurret.health + shieldGen.health + quantumCore.health;
}

float BossDreadnought::GetMaxHealth() const {
    return leftTurret.maxHealth + rightTurret.maxHealth + shieldGen.maxHealth + quantumCore.maxHealth;
}

float BossDreadnought::GetHealthRatio() const {
    float maxH = GetMaxHealth();
    return (maxH > 0.0f) ? std::clamp(GetTotalHealth() / maxH, 0.0f, 1.0f) : 0.0f;
}

void BossDreadnought::Draw(const Shader& shader) const {
    if (state == BossState::Inactive || state == BossState::Defeated) return;

    shader.Activate();
    shader.SetInt("uUseLighting", 1);
    shader.SetFloat("uAlpha", 1.0f);

    // 1. Armored Dreadnought Main Hull
    shader.SetMat4("uModel", transform.GetModelMatrix());
    hullMesh.Draw(shader);

    // 2. Port Heavy Turret
    if (!leftTurret.destroyed) {
        glm::mat4 modelL = glm::mat4(1.0f);
        modelL = glm::translate(modelL, leftTurret.worldPos);
        modelL = glm::rotate(modelL, glm::radians(transform.rotation.z), glm::vec3(0, 0, 1));
        modelL = glm::rotate(modelL, glm::radians(currentTurretYaw), glm::vec3(0, 1, 0));
        shader.SetMat4("uModel", modelL);
        turretMesh.Draw(shader);
    }

    // 3. Starboard Heavy Turret
    if (!rightTurret.destroyed) {
        glm::mat4 modelR = glm::mat4(1.0f);
        modelR = glm::translate(modelR, rightTurret.worldPos);
        modelR = glm::rotate(modelR, glm::radians(transform.rotation.z), glm::vec3(0, 0, 1));
        modelR = glm::rotate(modelR, glm::radians(currentTurretYaw), glm::vec3(0, 1, 0));
        shader.SetMat4("uModel", modelR);
        turretMesh.Draw(shader);
    }

    // 4. Dorsal Shield Generator Dome (Energy Shield)
    if (!shieldGen.destroyed) {
        shader.SetInt("uUseLighting", 0);
        float shieldAlpha = 0.35f + 0.15f * std::sin(corePulseTime);
        shader.SetFloat("uAlpha", shieldAlpha);

        glm::mat4 modelShield = glm::mat4(1.0f);
        modelShield = glm::translate(modelShield, shieldGen.worldPos);
        float pulseScale = 1.0f + 0.05f * std::sin(corePulseTime);
        modelShield = glm::scale(modelShield, glm::vec3(pulseScale));
        shader.SetMat4("uModel", modelShield);
        shieldMesh.Draw(shader);

        shader.SetInt("uUseLighting", 1);
        shader.SetFloat("uAlpha", 1.0f);
    }

    // 5. Quantum Reactor Core
    if (!quantumCore.destroyed) {
        shader.SetInt("uUseLighting", 0);
        glm::mat4 modelCore = glm::mat4(1.0f);
        modelCore = glm::translate(modelCore, quantumCore.worldPos);

        float coreScale = (state == BossState::Phase2_ExposedCore)
                          ? (1.0f + 0.15f * std::sin(corePulseTime * 1.5f))
                          : 0.85f;
        modelCore = glm::scale(modelCore, glm::vec3(coreScale));
        shader.SetMat4("uModel", modelCore);

        if (state == BossState::Phase2_ExposedCore) {
            // Overheating glowing red core
            shader.SetVec3("uAmbientColor", glm::vec3(1.0f, 0.2f, 0.1f));
        } else {
            // Contained core behind shields
            shader.SetVec3("uAmbientColor", glm::vec3(0.2f, 0.6f, 0.9f));
        }
        coreMesh.Draw(shader);

        shader.SetInt("uUseLighting", 1);
    }
}
