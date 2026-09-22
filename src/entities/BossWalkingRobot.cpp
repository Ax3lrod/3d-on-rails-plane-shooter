#include "BossWalkingRobot.h"
#include "SoundManager.h"
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>
#include <algorithm>

BossWalkingRobot::BossWalkingRobot()
    : hp(1800.0f), maxHp(1800.0f), stateTimer(0.0f), walkCycle(0.0f),
      yawAngle(0.0f), targetYaw(0.0f), cannonCooldown(1.8f),
      stompCooldown(4.5f), beamCooldown(6.0f), beamBurstTimer(0.0f),
      beamBurstCount(0), deathTimer(0.0f), isDead(false),
      isWarningActive(false), arenaCenter(0.0f, -7.5f, -13180.0f),
      bodyMesh(Mesh::CreateBipedalWalkerMesh(3.8f,
          glm::vec3(0.22f, 0.24f, 0.28f),   // dark brutalist concrete armor
          glm::vec3(0.16f, 0.18f, 0.22f),   // heavy steel leg chassis
          glm::vec3(1.0f, 0.25f, 0.15f))),  // glowing red core weak point
      shadowMesh(Mesh::CreateShadowDisc(10.0f, 16)) {}

void BossWalkingRobot::Reset() {
    hp = maxHp;
    isDead = false;
    isWarningActive = false;
    stateTimer = 0.0f;
    walkCycle = 0.0f;
    yawAngle = 0.0f;
    cannonCooldown = 1.8f;
    stompCooldown = 4.5f;
    beamCooldown = 6.0f;
    beamBurstCount = 0;
    currentPhase = Phase::Walk;
}

void BossWalkingRobot::Spawn(float playerZ) {
    Reset();
    arenaCenter = glm::vec3(0.0f, -7.5f, playerZ - 180.0f);
    transform.position = arenaCenter + glm::vec3(0.0f, 0.0f, -40.0f);
    transform.position.y = -7.5f;
}

void BossWalkingRobot::Update(float dt, float playerZ, const glm::vec3& playerPos,
                              ProjectileManager& projectiles, ParticleSystem& particles, Camera& camera,
                              SoundManager* audio) {
    if (isDead) return;

    stateTimer += dt;

    // Smoothly turn chassis to track oncoming starfighter
    glm::vec3 toPlayer = playerPos - transform.position;
    if (glm::length(toPlayer) > 0.5f) {
        targetYaw = std::atan2(toPlayer.x, toPlayer.z);
    }
    float yawDiff = targetYaw - yawAngle;
    while (yawDiff >  glm::pi<float>()) yawDiff -= glm::two_pi<float>();
    while (yawDiff < -glm::pi<float>()) yawDiff += glm::two_pi<float>();
    float turnSpeed = (currentPhase == Phase::Berserk) ? 2.8f : 1.8f;
    yawAngle += yawDiff * std::min(dt * turnSpeed, 1.0f);

    // Death sequence: progressive explosions then sink into shattered arena floor
    if (currentPhase == Phase::Death) {
        deathTimer += dt;
        transform.position.y -= dt * 2.8f;
        yawAngle += dt * 1.5f;

        // Chain detonations across chassis
        if (rand() % 4 == 0) {
            float ox = ((rand() % 100) / 50.0f - 1.0f) * 12.0f;
            float oy = ((rand() % 100) / 100.0f) * 20.0f;
            float oz = ((rand() % 100) / 50.0f - 1.0f) * 8.0f;
            particles.SpawnExplosion(transform.position + glm::vec3(ox, oy, oz), 8, glm::vec3(1.0f, 0.6f, 0.2f));
            if (audio && (rand() % 3 == 0)) {
                audio->Play(SoundID::ExplosionLarge, 0.65f, 0.8f + (rand() % 20) * 0.01f);
            }
        }

        if (deathTimer > 4.5f) {
            isDead = true;
            isWarningActive = false;
        }
        return;
    }

    // Dynamic locomotion across the arena floor
    float walkSpeed = (currentPhase == Phase::Berserk) ? 1.6f : 1.0f;
    walkCycle += dt * walkSpeed;
    float strideX = std::sin(walkCycle * 0.55f) * 95.0f;
    float strideZ = std::cos(walkCycle * 0.35f) * 45.0f;
    transform.position.x = arenaCenter.x + strideX;
    transform.position.z = arenaCenter.z - 40.0f + strideZ;
    transform.position.y = -7.5f;

    // Heavy footstep impact & dust
    if (std::sin(walkCycle * 1.1f) > 0.95f) {
        float footX = (strideX > 0.0f) ? 7.0f : -7.0f;
        particles.SpawnExplosion(transform.position + glm::vec3(footX, 0.5f, 0.0f), 4, glm::vec3(0.6f, 0.5f, 0.4f));
    }

    // ATTACK 1: Dual Shoulder Plasma Cannons (lead targeting)
    float cannonRate = (currentPhase == Phase::Berserk) ? 0.75f : ((currentPhase == Phase::Stomp) ? 1.25f : 1.8f);
    cannonCooldown -= dt;
    if (cannonCooldown <= 0.0f) {
        cannonCooldown = cannonRate;

        // Muzzle positions on left & right shoulders
        float shoulderOffset = 11.5f;
        float cosY = std::cos(yawAngle), sinY = std::sin(yawAngle);
        glm::vec3 mLeft  = transform.position + glm::vec3(-shoulderOffset * cosY, 15.0f, shoulderOffset * sinY);
        glm::vec3 mRight = transform.position + glm::vec3( shoulderOffset * cosY, 15.0f, -shoulderOffset * sinY);

        // Aim slightly ahead of player for aggressive arcade threat
        glm::vec3 leadTarget = playerPos;
        projectiles.SpawnLaser(mLeft,  leadTarget + glm::vec3(-1.5f, 0.0f, 0.0f), false, 85.0f);
        projectiles.SpawnLaser(mRight, leadTarget + glm::vec3( 1.5f, 0.0f, 0.0f), false, 85.0f);
        particles.SpawnExplosion(mLeft,  4, glm::vec3(1.0f, 0.3f, 0.1f));
        particles.SpawnExplosion(mRight, 4, glm::vec3(1.0f, 0.3f, 0.1f));

        if (audio) {
            audio->Play(SoundID::EnemyLaser, 0.55f, 0.90f);
        }
    }

    // ATTACK 2: Earthquake Stomp & Expanding Ground Laser Shockwave (Phase 2 & 3)
    if (currentPhase >= Phase::Stomp) {
        stompCooldown -= dt;
        if (stompCooldown <= 0.0f) {
            stompCooldown = (currentPhase == Phase::Berserk) ? 3.8f : 5.2f;

            camera.TriggerShake(1.35f, 0.55f);
            if (audio) {
                audio->Play(SoundID::ExplosionLarge, 0.85f, 0.75f);
            }

            // Expanding 360-degree shockwave ring of 12 ground lasers
            const int pulses = 12;
            for (int p = 0; p < pulses; ++p) {
                float a = p * (glm::two_pi<float>() / pulses);
                glm::vec3 ringDir(std::cos(a), 0.0f, std::sin(a));
                glm::vec3 spawnPos = transform.position + ringDir * 8.0f + glm::vec3(0.0f, 1.2f, 0.0f);
                projectiles.SpawnLaserWithDir(spawnPos, ringDir, false, 65.0f, glm::vec3(1.0f, 0.45f, 0.15f), 1.2f);
            }
            particles.SpawnExplosion(transform.position + glm::vec3(0.0f, 1.0f, 0.0f), 16, glm::vec3(1.0f, 0.4f, 0.1f));
        }
    }

    // ATTACK 3: Overclocked Head Particle Beam Barrage (Phase 3 Berserk)
    if (currentPhase == Phase::Berserk) {
        isWarningActive = true;
        beamCooldown -= dt;
        if (beamCooldown <= 0.0f) {
            beamBurstCount = 5;
            beamBurstTimer = 0.0f;
            beamCooldown = 4.8f;
        }

        if (beamBurstCount > 0) {
            beamBurstTimer -= dt;
            if (beamBurstTimer <= 0.0f) {
                beamBurstTimer = 0.12f;
                beamBurstCount--;

                glm::vec3 headPos = transform.position + glm::vec3(0.0f, 18.0f, 0.0f);
                projectiles.SpawnLaser(headPos, playerPos, false, 115.0f);
                particles.SpawnExplosion(headPos, 6, glm::vec3(1.0f, 0.9f, 0.2f));
                camera.TriggerShake(0.6f, 0.15f);
                if (audio) {
                    audio->Play(SoundID::EnemyLaser, 0.60f, 1.35f);
                }
            }
        }
    }
}

void BossWalkingRobot::Draw(const Shader& shader) const {
    if (isDead && deathTimer > 4.5f) return;

    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, transform.position);
    model = glm::rotate(model, yawAngle, glm::vec3(0.0f, 1.0f, 0.0f));
    shader.SetMat4("uModel", model);
    bodyMesh.Draw(shader);

    // Dynamic ground shadow disc
    glm::mat4 shadowModel = glm::mat4(1.0f);
    shadowModel = glm::translate(shadowModel, glm::vec3(transform.position.x, -7.42f, transform.position.z));
    shader.SetMat4("uModel", shadowModel);
    shadowMesh.Draw(shader);
}

int BossWalkingRobot::FindLockTarget(const glm::vec3& playerPos, const glm::vec3& aimPos, glm::vec3& outLockPos) const {
    if (isDead) return -1;
    // Lock onto the glowing head core weak point
    glm::vec3 headPos = transform.position + glm::vec3(0.0f, 18.0f, 0.0f);
    outLockPos = headPos;
    return 1;
}

bool BossWalkingRobot::CheckLaserHit(const glm::vec3& laserPos, float laserRadius, float damage,
                                   ParticleSystem& particles, Camera& camera, int& outScoreGained) {
    if (isDead || currentPhase == Phase::Death) return false;

    // 1. Weak Point Hit Check: Head glowing core
    glm::vec3 headPos = transform.position + glm::vec3(0.0f, 18.0f, 0.0f);
    float distHead = glm::distance(laserPos, headPos);
    if (distHead < 4.5f + laserRadius) {
        // Critical hit! 2.5x damage on glowing core
        float effectiveDmg = damage * 2.5f;
        hp -= effectiveDmg;
        particles.SpawnExplosion(laserPos, 9, glm::vec3(1.0f, 0.85f, 0.2f));
        camera.TriggerShake(0.45f, 0.15f);

        if (hp <= 0.0f) {
            currentPhase = Phase::Death;
            deathTimer = 0.0f;
            outScoreGained += 15000;
        } else if (hp < maxHp * 0.35f) {
            currentPhase = Phase::Berserk;
        } else if (hp < maxHp * 0.70f) {
            currentPhase = Phase::Stomp;
        }
        return true;
    }

    // 2. Heavy Armored Chassis Hit Check: 0.35x damage
    float distBody = glm::distance(laserPos, transform.position + glm::vec3(0.0f, 9.0f, 0.0f));
    if (distBody < 12.0f + laserRadius) {
        float effectiveDmg = damage * 0.35f;
        hp -= effectiveDmg;
        particles.SpawnExplosion(laserPos, 3, glm::vec3(0.5f, 0.5f, 0.6f));

        if (hp <= 0.0f) {
            currentPhase = Phase::Death;
            deathTimer = 0.0f;
            outScoreGained += 15000;
        } else if (hp < maxHp * 0.35f) {
            currentPhase = Phase::Berserk;
        } else if (hp < maxHp * 0.70f) {
            currentPhase = Phase::Stomp;
        }
        return true;
    }

    return false;
}

void BossWalkingRobot::ApplyShockwaveDamage(const glm::vec3& shockPos, float radius, float damage,
                                          ParticleSystem& particles, Camera& camera, int& outScoreGained) {
    if (isDead || currentPhase == Phase::Death) return;
    if (glm::distance(shockPos, transform.position) < radius + 12.0f) {
        hp -= damage;
        particles.SpawnExplosion(transform.position + glm::vec3(0.0f, 8.0f, 0.0f), 12, glm::vec3(1.0f, 0.5f, 0.2f));
        if (hp <= 0.0f) {
            currentPhase = Phase::Death;
            deathTimer = 0.0f;
            outScoreGained += 15000;
        } else if (hp < maxHp * 0.35f) {
            currentPhase = Phase::Berserk;
        } else if (hp < maxHp * 0.70f) {
            currentPhase = Phase::Stomp;
        }
    }
}
