#include "BossMegaTank.h"
#include "SoundManager.h"
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>
#include <cstdlib>
#include <algorithm>

BossMegaTank::BossMegaTank()
    : state(TankBossState::Inactive),
      chassisMesh(Mesh::CreateMegaTankChassis(
          glm::vec3(0.20f, 0.22f, 0.25f), // Dark track steel
          glm::vec3(0.38f, 0.44f, 0.36f)  // Military olive armor plates
      )),
      turretMesh(Mesh::CreateMegaTankTurret(
          glm::vec3(0.32f, 0.38f, 0.30f), // Cast steel turret
          glm::vec3(0.18f, 0.20f, 0.22f)  // Dark gunmetal barrels
      )),
      coreMesh(Mesh::CreateMegaTankCore(
          glm::vec3(1.0f, 0.4f, 0.12f)    // Luminous molten orange reactor
      )),
      leftTreadHp(250.0f),
      rightTreadHp(250.0f),
      leftTurretHp(300.0f),
      rightTurretHp(300.0f),
      coreHp(450.0f),
      leftTreadDestroyed(false),
      rightTreadDestroyed(false),
      leftTurretDestroyed(false),
      rightTurretDestroyed(false),
      coreDestroyed(false),
      turretYawL(0.0f),
      turretYawR(0.0f),
      corePulseTime(0.0f),
      fireTimerL(1.0f),
      fireTimerR(1.8f),
      mortarTimer(4.0f),
      chargeLaserTimer(6.0f),
      stateTime(0.0f),
      warningTimer(3.2f),
      approachProgress(0.0f),
      deathTimer(0.0f),
      nextExplosionTimer(0.0f) {
    Reset();
}

void BossMegaTank::Reset() {
    state = TankBossState::Inactive;
    stateTime = 0.0f;
    warningTimer = 3.2f;
    approachProgress = 0.0f;
    deathTimer = 0.0f;
    nextExplosionTimer = 0.0f;

    leftTreadHp = 250.0f;
    rightTreadHp = 250.0f;
    leftTurretHp = 300.0f;
    rightTurretHp = 300.0f;
    coreHp = 450.0f;

    leftTreadDestroyed = false;
    rightTreadDestroyed = false;
    leftTurretDestroyed = false;
    rightTurretDestroyed = false;
    coreDestroyed = false;

    turretYawL = 0.0f;
    turretYawR = 0.0f;
    corePulseTime = 0.0f;
    fireTimerL = 1.0f;
    fireTimerR = 1.8f;
    mortarTimer = 4.0f;
    chargeLaserTimer = 6.0f;
}

void BossMegaTank::Spawn(float playerZ) {
    Reset();
    state = TankBossState::Approaching;
    transform.position = glm::vec3(0.0f, -7.5f, playerZ - 180.0f);
}

void BossMegaTank::Update(float dt, float playerZ, const glm::vec3& playerPos,
                          ProjectileManager& projectiles, ParticleSystem& particles, Camera& camera,
                          SoundManager* audio) {
    if (state == TankBossState::Inactive || state == TankBossState::Defeated) return;

    stateTime += dt;
    corePulseTime += dt;
    transform.position.y = -7.5f; // Locked to ground

    if (state == TankBossState::Approaching) {
        warningTimer -= dt;
        approachProgress = std::min(1.0f, approachProgress + dt * 0.45f);
        float targetZ = playerZ - 70.0f;
        transform.position.z = glm::mix(playerZ - 180.0f, targetZ, approachProgress);

        if (warningTimer <= 0.0f) {
            state = TankBossState::CombatPatrol;
        }
        return;
    }

    if (state == TankBossState::DeathSpiral) {
        deathTimer += dt;
        nextExplosionTimer -= dt;
        if (nextExplosionTimer <= 0.0f) {
            nextExplosionTimer = 0.09f;
            glm::vec3 expPos = transform.position + glm::vec3(
                ((rand() % 100) / 50.0f - 1.0f) * 7.0f,
                ((rand() % 100) / 100.0f) * 4.5f,
                ((rand() % 100) / 50.0f - 1.0f) * 8.0f
            );
            particles.SpawnExplosion(expPos, 24, glm::vec3(1.0f, 0.5f, 0.1f));
            if (audio) audio->Play(SoundID::ExplosionSmall, 0.7f, 0.8f + (rand()%20)*0.01f);
        }

        if (deathTimer >= 3.5f) {
            state = TankBossState::Defeated;
            particles.SpawnExplosion(transform.position + glm::vec3(0, 2, 0), 90, glm::vec3(1.0f, 0.8f, 0.2f));
        }
        return;
    }

    // Check destruction of core or all weapons
    if (coreDestroyed || (leftTurretDestroyed && rightTurretDestroyed && leftTreadDestroyed && rightTreadDestroyed)) {
        state = TankBossState::DeathSpiral;
        camera.TriggerShake(1.5f, 0.6f);
        return;
    }

    // Ground movement speed based on treads
    float speed = 10.0f;
    if (leftTreadDestroyed && rightTreadDestroyed) speed = 0.0f;
    else if (leftTreadDestroyed || rightTreadDestroyed) speed = 4.5f;

    float targetZ = playerZ - 65.0f;
    transform.position.z = glm::mix(transform.position.z, targetZ, dt * 2.5f);

    // Lateral strafing across the arena
    if (speed > 0.0f) {
        float strafeX = std::sin(stateTime * 0.45f) * 16.0f;
        transform.position.x = glm::mix(transform.position.x, strafeX, dt * 1.8f);
    }

    // Turret aim tracking
    glm::vec3 turretPosL = transform.position + glm::vec3(-3.2f, 2.5f, 1.5f);
    glm::vec3 turretPosR = transform.position + glm::vec3( 3.2f, 2.5f, 1.5f);

    float targetYawL = glm::degrees(std::atan2(playerPos.x - turretPosL.x, playerPos.z - turretPosL.z));
    float targetYawR = glm::degrees(std::atan2(playerPos.x - turretPosR.x, playerPos.z - turretPosR.z));
    turretYawL = glm::mix(turretYawL, targetYawL, dt * 3.5f);
    turretYawR = glm::mix(turretYawR, targetYawR, dt * 3.5f);

    // Left Turret Heavy Shells
    if (!leftTurretDestroyed) {
        fireTimerL -= dt;
        if (fireTimerL <= 0.0f) {
            fireTimerL = 1.8f;
            glm::vec3 m0 = turretPosL + glm::vec3(-0.75f, 0.5f, 4.5f);
            glm::vec3 m1 = turretPosL + glm::vec3( 0.75f, 0.5f, 4.5f);
            projectiles.SpawnLaser(m0, playerPos + glm::vec3(-1.0f, 0, 0), false, 80.0f);
            projectiles.SpawnLaser(m1, playerPos + glm::vec3( 1.0f, 0, 0), false, 80.0f);
            if (audio) audio->Play(SoundID::EnemyLaser, 0.60f, 0.70f);
        }
    }

    // Right Turret 3-Way Spread
    if (!rightTurretDestroyed) {
        fireTimerR -= dt;
        if (fireTimerR <= 0.0f) {
            fireTimerR = 2.4f;
            for (float ox : {-2.5f, 0.0f, 2.5f}) {
                projectiles.SpawnLaser(turretPosR, playerPos + glm::vec3(ox, 0, 0), false, 75.0f);
            }
            if (audio) audio->Play(SoundID::EnemyLaser, 0.55f, 0.85f);
        }
    }

    // High-angle mortar barrages
    mortarTimer -= dt;
    if (mortarTimer <= 0.0f) {
        mortarTimer = 4.5f;
        glm::vec3 mortarPos = transform.position + glm::vec3(0.0f, 3.5f, -3.0f);
        projectiles.SpawnLaser(mortarPos, playerPos + glm::vec3(0.0f, 6.0f, -10.0f), false, 65.0f);
        if (audio) audio->Play(SoundID::EnemyLaser, 0.70f, 0.60f);
    }
}

void BossMegaTank::Draw(const Shader& shader) const {
    if (state == TankBossState::Inactive || state == TankBossState::Defeated) return;

    shader.SetInt("uUseLighting", 1);
    shader.SetFloat("uAlpha", 1.0f);

    // Chassis Base
    glm::mat4 mChassis = transform.GetModelMatrix();
    shader.SetMat4("uModel", mChassis);
    chassisMesh.Draw(shader);

    // Left Turret
    if (!leftTurretDestroyed) {
        glm::mat4 mTL = mChassis;
        mTL = glm::translate(mTL, glm::vec3(-3.2f, 2.4f, 1.5f));
        mTL = glm::rotate(mTL, glm::radians(turretYawL), glm::vec3(0, 1, 0));
        shader.SetMat4("uModel", mTL);
        turretMesh.Draw(shader);
    }

    // Right Turret
    if (!rightTurretDestroyed) {
        glm::mat4 mTR = mChassis;
        mTR = glm::translate(mTR, glm::vec3( 3.2f, 2.4f, 1.5f));
        mTR = glm::rotate(mTR, glm::radians(turretYawR), glm::vec3(0, 1, 0));
        shader.SetMat4("uModel", mTR);
        turretMesh.Draw(shader);
    }

    // Glowing Overheating Core at Rear Deck
    if (!coreDestroyed) {
        glm::mat4 mCore = mChassis;
        mCore = glm::translate(mCore, glm::vec3(0.0f, 3.8f, -4.5f));
        float pulse = 1.0f + std::sin(corePulseTime * 6.0f) * 0.15f;
        mCore = glm::scale(mCore, glm::vec3(pulse));
        shader.SetMat4("uModel", mCore);
        coreMesh.Draw(shader);
    }
}

int BossMegaTank::FindLockTarget(const glm::vec3& playerPos, const glm::vec3& aimPos, glm::vec3& outLockPos) const {
    float bestDistSq = 18.0f * 18.0f;
    int target = -1;

    auto testTarget = [&](const glm::vec3& pos, bool destroyed, int id) {
        if (destroyed) return;
        float zDist = playerPos.z - pos.z;
        if (zDist < 10.0f || zDist > 180.0f) return;
        float dx = pos.x - aimPos.x;
        float dy = pos.y - aimPos.y;
        float dsq = dx*dx + dy*dy;
        if (dsq < bestDistSq) {
            bestDistSq = dsq;
            target = id;
            outLockPos = pos;
        }
    };

    testTarget(transform.position + glm::vec3(-3.2f, 2.5f, 1.5f), leftTurretDestroyed, 1);
    testTarget(transform.position + glm::vec3( 3.2f, 2.5f, 1.5f), rightTurretDestroyed, 2);
    testTarget(transform.position + glm::vec3(0.0f, 3.8f, -4.5f), coreDestroyed, 3);
    testTarget(transform.position + glm::vec3(-6.0f, 0.5f, 0.0f), leftTreadDestroyed, 4);
    testTarget(transform.position + glm::vec3( 6.0f, 0.5f, 0.0f), rightTreadDestroyed, 5);

    return target;
}

bool BossMegaTank::CheckLaserHit(const glm::vec3& laserPos, float laserRadius, float damage,
                                ParticleSystem& particles, Camera& camera, int& outScoreGained) {
    if (state == TankBossState::Inactive || state == TankBossState::Approaching || state == TankBossState::Defeated) {
        return false;
    }

    auto hitSubsystem = [&](glm::vec3 pos, float r, float& hp, bool& destroyed, int scoreVal) -> bool {
        if (destroyed) return false;
        if (glm::distance(laserPos, pos) <= (r + laserRadius)) {
            hp -= damage;
            particles.SpawnExplosion(laserPos, 6, glm::vec3(1.0f, 0.7f, 0.2f));
            outScoreGained += 50;

            if (hp <= 0.0f) {
                destroyed = true;
                particles.SpawnExplosion(pos, 35, glm::vec3(1.0f, 0.4f, 0.1f));
                camera.TriggerShake(0.8f, 0.4f);
                outScoreGained += scoreVal;
            }
            return true;
        }
        return false;
    };

    if (hitSubsystem(transform.position + glm::vec3(0, 3.8f, -4.5f), 2.2f, coreHp, coreDestroyed, 2500)) return true;
    if (hitSubsystem(transform.position + glm::vec3(-3.2f, 2.5f, 1.5f), 2.4f, leftTurretHp, leftTurretDestroyed, 1200)) return true;
    if (hitSubsystem(transform.position + glm::vec3( 3.2f, 2.5f, 1.5f), 2.4f, rightTurretHp, rightTurretDestroyed, 1200)) return true;
    if (hitSubsystem(transform.position + glm::vec3(-6.0f, 1.0f, 0.0f), 3.2f, leftTreadHp, leftTreadDestroyed, 800)) return true;
    if (hitSubsystem(transform.position + glm::vec3( 6.0f, 1.0f, 0.0f), 3.2f, rightTreadHp, rightTreadDestroyed, 800)) return true;

    return false;
}

void BossMegaTank::ApplyShockwaveDamage(const glm::vec3& shockPos, float radius, float damage,
                                      ParticleSystem& particles, Camera& camera, int& outScoreGained) {
    if (state == TankBossState::Inactive || state == TankBossState::Approaching || state == TankBossState::Defeated) {
        return;
    }

    auto shockSub = [&](glm::vec3 pos, float& hp, bool& destroyed, int scoreVal) {
        if (destroyed) return;
        if (glm::distance(shockPos, pos) <= radius) {
            hp -= damage;
            particles.SpawnExplosion(pos, 12, glm::vec3(0.4f, 0.8f, 1.0f));
            outScoreGained += 80;
            if (hp <= 0.0f) {
                destroyed = true;
                particles.SpawnExplosion(pos, 35, glm::vec3(1.0f, 0.4f, 0.1f));
                camera.TriggerShake(0.8f, 0.4f);
                outScoreGained += scoreVal;
            }
        }
    };

    shockSub(transform.position + glm::vec3(0, 3.8f, -4.5f), coreHp, coreDestroyed, 2500);
    shockSub(transform.position + glm::vec3(-3.2f, 2.5f, 1.5f), leftTurretHp, leftTurretDestroyed, 1200);
    shockSub(transform.position + glm::vec3( 3.2f, 2.5f, 1.5f), rightTurretHp, rightTurretDestroyed, 1200);
    shockSub(transform.position + glm::vec3(-6.0f, 1.0f, 0.0f), leftTreadHp, leftTreadDestroyed, 800);
    shockSub(transform.position + glm::vec3( 6.0f, 1.0f, 0.0f), rightTreadHp, rightTreadDestroyed, 800);
}

float BossMegaTank::GetHealthRatio() const {
    float cur = (coreDestroyed ? 0.0f : coreHp) +
                (leftTurretDestroyed ? 0.0f : leftTurretHp) +
                (rightTurretDestroyed ? 0.0f : rightTurretHp) +
                (leftTreadDestroyed ? 0.0f : leftTreadHp) +
                (rightTreadDestroyed ? 0.0f : rightTreadHp);
    float maxH = 450.0f + 300.0f + 300.0f + 250.0f + 250.0f;
    return std::clamp(cur / maxH, 0.0f, 1.0f);
}
