#include "BossWalkingRobot.h"
#include "SoundManager.h"
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>
#include <algorithm>

BossWalkingRobot::BossWalkingRobot()
    : hp(600.0f), maxHp(600.0f), stateTimer(0.0f), walkCycle(0.0f),
      yawAngle(0.0f), targetYaw(0.0f), cannonCooldown(3.0f),
      deathTimer(0.0f), isDead(false),
      arenaCenter(0.0f, 0.0f, -13500.0f),
      bodyMesh(Mesh::CreateBipedalWalkerMesh(3.5f,
          glm::vec3(0.22f, 0.24f, 0.28f),   // dark concrete body
          glm::vec3(0.16f, 0.18f, 0.22f),   // darker legs
          glm::vec3(0.95f, 0.25f, 0.15f))), // red glowing core
      shadowMesh(Mesh::CreateShadowDisc(8.0f, 12)),
      shellMesh(Mesh::CreateAsteroid(0.5f, glm::vec3(0.95f, 0.55f, 0.15f))) {}

void BossWalkingRobot::Reset() {
    hp = maxHp;
    isDead = false;
    stateTimer = 0.0f;
    walkCycle = 0.0f;
    yawAngle = 0.0f;
    shells.clear();
    currentPhase = Phase::Walk;
}

void BossWalkingRobot::Spawn(float playerZ) {
    Reset();
    arenaCenter = glm::vec3(0.0f, 0.0f, playerZ - 180.0f);
    transform.position = arenaCenter + glm::vec3(0.0f, 0.0f, -60.0f);
    transform.position.y = 0.0f;
}

void BossWalkingRobot::Update(float dt, float playerZ, const glm::vec3& playerPos,
                              ProjectileManager& projectiles, ParticleSystem& particles, Camera& camera,
                              SoundManager* audio) {
    if (isDead) return;

    stateTimer += dt;

    // Rotate to face player
    glm::vec3 toPlayer = playerPos - transform.position;
    if (glm::length(toPlayer) > 0.5f) {
        targetYaw = std::atan2(toPlayer.x, toPlayer.z);
    }
    float yawDiff = targetYaw - yawAngle;
    while (yawDiff >  glm::pi<float>()) yawDiff -= glm::two_pi<float>();
    while (yawDiff < -glm::pi<float>()) yawDiff += glm::two_pi<float>();
    yawAngle += yawDiff * std::min(dt * 1.8f, 1.0f);

    if (currentPhase == Phase::Death) {
        deathTimer += dt;
        transform.position.y -= dt * 3.5f;
        yawAngle += dt * 2.5f;
        if (deathTimer > 4.0f) isDead = true;
        return;
    }

    // Walk: sinusoidal side drift
    walkCycle += dt * (currentPhase == Phase::Cannon ? 1.8f : 1.0f);
    float driftSpeed = (currentPhase == Phase::Stomp) ? 0.3f : 1.0f;
    transform.position.x = arenaCenter.x + std::sin(walkCycle * 0.45f) * 80.0f * driftSpeed;
    transform.position.z = arenaCenter.z - 60.0f; // stays at fixed depth
    transform.position.y = 0.0f;

    // Cannon fire
    float cooldown = (currentPhase == Phase::Cannon) ? 1.4f : 3.0f;
    cannonCooldown -= dt;
    if (cannonCooldown <= 0.0f) {
        cannonCooldown = cooldown;
        // Fire 2 shells (one per shoulder)
        glm::vec3 toP = glm::normalize(playerPos - transform.position + glm::vec3(0.0f, 0.0f, 0.0f));
        float spread = (currentPhase == Phase::Stomp) ? 0.25f : 0.0f;
        for (int i = -1; i <= 1; i += 2) {
            float sx = transform.position.x + i * 11.0f;
            glm::vec3 sPos(sx, transform.position.y + 13.0f, transform.position.z);
            glm::vec3 sVel = toP * 38.0f + glm::vec3(i * spread * 20.0f, 0.0f, 0.0f);
            shells.push_back({sPos, sVel, 5.5f});
        }
        if (currentPhase == Phase::Stomp) {
            // Extra fan shells
            for (int i = -2; i <= 2; ++i) {
                float angle = i * 0.35f;
                glm::vec3 sVel = glm::vec3(std::sin(angle), -0.1f, std::cos(angle)) * 32.0f;
                shells.push_back({transform.position + glm::vec3(0.0f,8.0f,0.0f), sVel, 4.0f});
            }
        }
    }

    // Update shells
    for (auto& sh : shells) {
        sh.pos += sh.vel * dt;
        sh.life -= dt;
        // Collision with player
        if (glm::length(sh.pos - playerPos) < 2.0f) {
            // we can't damage player directly here easily without callback, but we can spawn explosion
            particles.SpawnExplosion(sh.pos, 5, glm::vec3(1.0f, 0.5f, 0.1f));
            sh.life = 0.0f; // mark for death
        }
    }
    shells.erase(std::remove_if(shells.begin(), shells.end(),
        [](const CannonShell& s) { return s.life <= 0.0f; }), shells.end());
        
    // Player collision
    if (glm::length(playerPos - transform.position) < 14.0f) {
        // Player takes damage (handled externally if possible, or just ignore since collision is usually in engine)
    }
}

void BossWalkingRobot::Draw(const Shader& shader) const {
    if (isDead && deathTimer > 4.0f) return;

    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, transform.position);
    model = glm::rotate(model, yawAngle, glm::vec3(0.0f, 1.0f, 0.0f));
    shader.SetMat4("uModel", model);
    bodyMesh.Draw(shader);

    // Shadow
    glm::mat4 shadowModel = glm::mat4(1.0f);
    shadowModel = glm::translate(shadowModel, glm::vec3(transform.position.x, -7.45f, transform.position.z));
    shader.SetMat4("uModel", shadowModel);
    shadowMesh.Draw(shader);

    // Shells
    for (const auto& sh : shells) {
        glm::mat4 sm = glm::translate(glm::mat4(1.0f), sh.pos);
        shader.SetMat4("uModel", sm);
        shellMesh.Draw(shader);
    }
}

int BossWalkingRobot::FindLockTarget(const glm::vec3& playerPos, const glm::vec3& aimPos, glm::vec3& outLockPos) const {
    if (isDead) return -1;
    // Lock onto the core head
    glm::vec3 headPos = transform.position + glm::vec3(0.0f, 15.0f, 0.0f);
    outLockPos = headPos;
    return 1;
}

bool BossWalkingRobot::CheckLaserHit(const glm::vec3& laserPos, float laserRadius, float damage,
                                   ParticleSystem& particles, Camera& camera, int& outScoreGained) {
    if (isDead) return false;
    
    // Check head core
    glm::vec3 headPos = transform.position + glm::vec3(0.0f, 15.0f, 0.0f);
    if (glm::distance(laserPos, headPos) < 5.0f + laserRadius) {
        hp -= damage * 2.0f; // Weak point
        particles.SpawnExplosion(laserPos, 5, glm::vec3(1.0f, 0.3f, 0.1f));
        if (hp <= 0.0f && currentPhase != Phase::Death) {
            currentPhase = Phase::Death;
            deathTimer = 0.0f;
            outScoreGained += 5000;
        } else if (hp < maxHp * 0.35f) {
            currentPhase = Phase::Cannon;
        } else if (hp < maxHp * 0.70f) {
            currentPhase = Phase::Stomp;
        }
        return true;
    }
    
    // Check body
    if (glm::distance(laserPos, transform.position) < 12.0f + laserRadius) {
        hp -= damage;
        particles.SpawnExplosion(laserPos, 3, glm::vec3(0.6f, 0.6f, 0.6f));
        if (hp <= 0.0f && currentPhase != Phase::Death) {
            currentPhase = Phase::Death;
            deathTimer = 0.0f;
            outScoreGained += 5000;
        } else if (hp < maxHp * 0.35f) {
            currentPhase = Phase::Cannon;
        } else if (hp < maxHp * 0.70f) {
            currentPhase = Phase::Stomp;
        }
        return true;
    }
    return false;
}

void BossWalkingRobot::ApplyShockwaveDamage(const glm::vec3& shockPos, float radius, float damage,
                                          ParticleSystem& particles, Camera& camera, int& outScoreGained) {
    if (isDead) return;
    if (glm::distance(shockPos, transform.position) < radius + 12.0f) {
        hp -= damage;
        if (hp <= 0.0f && currentPhase != Phase::Death) {
            currentPhase = Phase::Death;
            deathTimer = 0.0f;
            outScoreGained += 5000;
        } else if (hp < maxHp * 0.35f) {
            currentPhase = Phase::Cannon;
        } else if (hp < maxHp * 0.70f) {
            currentPhase = Phase::Stomp;
        }
    }
}
