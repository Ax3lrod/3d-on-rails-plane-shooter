#include "BossMechaWorm.h"
#include "SoundManager.h"
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>
#include <cstdlib>
#include <algorithm>

BossMechaWorm::BossMechaWorm()
    : state(WormBossState::Inactive),
      headMesh(Mesh::CreateWormHead(
          glm::vec3(0.25f, 0.28f, 0.34f), // Armored cybernetic carapace
          glm::vec3(0.85f, 0.15f, 0.18f), // Crimson mandibles
          glm::vec3(1.0f, 0.25f, 0.12f)   // Glowing optic cluster
      )),
      segmentMesh(Mesh::CreateWormSegment(
          2.3f,
          glm::vec3(0.28f, 0.32f, 0.38f), // Segment armor scale
          glm::vec3(0.22f, 0.85f, 1.0f)   // Ventral electric power node
      )),
      headHealth(400.0f),
      headMaxHealth(400.0f),
      totalMaxHealth(1400.0f),
      breachTimer(0.0f),
      breachDuration(4.5f),
      fireTimer(1.0f),
      mandibleAngle(0.0f),
      dustSpawnTimer(0.0f),
      stateTime(0.0f),
      warningTimer(3.2f),
      approachProgress(0.0f),
      deathTimer(0.0f),
      nextExplosionTimer(0.0f) {
    Reset();
}

void BossMechaWorm::Reset() {
    state = WormBossState::Inactive;
    headHealth = 400.0f;
    headMaxHealth = 400.0f;
    totalMaxHealth = 1400.0f;
    stateTime = 0.0f;
    warningTimer = 3.2f;
    approachProgress = 0.0f;
    deathTimer = 0.0f;
    nextExplosionTimer = 0.0f;
    breachTimer = 0.0f;
    fireTimer = 1.0f;
    mandibleAngle = 0.0f;
    dustSpawnTimer = 0.0f;

    segments.clear();
    for (int i = 0; i < 10; ++i) {
        WormSegmentState s;
        s.position = glm::vec3(0.0f, -14.0f, -i * 3.5f);
        s.rotation = glm::vec3(0.0f);
        s.health = 100.0f;
        s.destroyed = false;
        segments.push_back(s);
    }
}

void BossMechaWorm::Spawn(float playerZ) {
    Reset();
    state = WormBossState::Approaching;
    transform.position = glm::vec3(0.0f, -14.0f, playerZ - 180.0f);

    for (size_t i = 0; i < segments.size(); ++i) {
        segments[i].position = transform.position - glm::vec3(0.0f, 0.0f, (i + 1) * 3.5f);
    }
}

void BossMechaWorm::Update(float dt, float playerZ, const glm::vec3& playerPos,
                           ProjectileManager& projectiles, ParticleSystem& particles, Camera& camera,
                           SoundManager* audio) {
    if (state == WormBossState::Inactive || state == WormBossState::Defeated) return;

    stateTime += dt;
    mandibleAngle = std::sin(stateTime * 8.0f) * 12.0f;

    if (state == WormBossState::Approaching) {
        warningTimer -= dt;
        approachProgress = std::min(1.0f, approachProgress + dt * 0.4f);
        float targetZ = playerZ - 100.0f;
        transform.position.z = glm::mix(playerZ - 180.0f, targetZ, approachProgress);

        dustSpawnTimer -= dt;
        if (dustSpawnTimer <= 0.0f) {
            dustSpawnTimer = 0.15f;
            particles.SpawnExplosion(glm::vec3(transform.position.x, -7.3f, transform.position.z), 10, glm::vec3(0.7f, 0.6f, 0.4f));
        }

        if (warningTimer <= 0.0f) {
            state = WormBossState::SubterraneanBurrow;
            breachTimer = 2.0f;
        }
        return;
    }

    if (state == WormBossState::DeathSpiral) {
        deathTimer += dt;
        nextExplosionTimer -= dt;
        if (nextExplosionTimer <= 0.0f) {
            nextExplosionTimer = 0.07f;
            int idx = rand() % (segments.size() + 1);
            glm::vec3 expPos = (idx == 0) ? transform.position : segments[idx - 1].position;
            expPos += glm::vec3(
                ((rand()%100)/50.0f - 1.0f)*2.5f,
                ((rand()%100)/50.0f - 1.0f)*2.5f,
                ((rand()%100)/50.0f - 1.0f)*2.5f
            );
            particles.SpawnExplosion(expPos, 25, glm::vec3(1.0f, 0.45f, 0.1f));
            if (audio) audio->Play(SoundID::ExplosionSmall, 0.7f, 0.8f + (rand()%20)*0.01f);
        }

        if (deathTimer >= 3.8f) {
            state = WormBossState::Defeated;
            particles.SpawnExplosion(transform.position, 100, glm::vec3(1.0f, 0.85f, 0.3f));
        }
        return;
    }

    if (headHealth <= 0.0f) {
        state = WormBossState::DeathSpiral;
        camera.TriggerShake(1.5f, 0.6f);
        return;
    }

    // Kinematic Behavior State Machine
    if (state == WormBossState::SubterraneanBurrow) {
        breachTimer -= dt;
        transform.position.y = -13.0f;
        transform.position.z += 25.0f * dt;
        transform.position.x = std::sin(stateTime * 1.2f) * 12.0f;

        // Erupting sand geysers on ground surface
        dustSpawnTimer -= dt;
        if (dustSpawnTimer <= 0.0f) {
            dustSpawnTimer = 0.12f;
            particles.SpawnExplosion(glm::vec3(transform.position.x, -7.3f, transform.position.z), 12, glm::vec3(0.85f, 0.72f, 0.45f));
        }

        if (breachTimer <= 0.0f) {
            state = WormBossState::BreachAscent;
            breachTimer = 0.0f;
            breachDuration = 4.2f;
            breachStartPos = transform.position;
            breachApexPos = glm::vec3(-transform.position.x * 0.8f, 22.0f, playerZ - 40.0f);
            breachEndPos = glm::vec3(transform.position.x * 0.5f, -14.0f, playerZ + 60.0f);

            camera.TriggerShake(0.8f, 0.4f);
            particles.SpawnExplosion(glm::vec3(transform.position.x, -7.3f, transform.position.z), 40, glm::vec3(0.9f, 0.75f, 0.5f));
            if (audio) audio->Play(SoundID::ExplosionLarge, 0.8f, 0.65f);
        }
    } else if (state == WormBossState::BreachAscent || state == WormBossState::BreachDescent) {
        breachTimer += dt;
        float t = breachTimer / breachDuration;

        if (t >= 1.0f) {
            state = WormBossState::SubterraneanBurrow;
            breachTimer = 2.5f;
            particles.SpawnExplosion(glm::vec3(transform.position.x, -7.3f, transform.position.z), 40, glm::vec3(0.9f, 0.75f, 0.5f));
            if (audio) audio->Play(SoundID::ExplosionLarge, 0.8f, 0.65f);
        } else {
            // Quadratic Bezier leap across sky
            glm::vec3 p0 = breachStartPos;
            glm::vec3 p1 = breachApexPos;
            glm::vec3 p2 = breachEndPos;
            glm::vec3 nextPos = (1.0f - t) * (1.0f - t) * p0 + 2.0f * (1.0f - t) * t * p1 + t * t * p2;

            glm::vec3 vel = nextPos - transform.position;
            transform.position = nextPos;

            // Align head to velocity vector
            float yaw = glm::degrees(std::atan2(vel.x, vel.z));
            float horizDist = std::sqrt(vel.x*vel.x + vel.z*vel.z);
            float pitch = glm::degrees(std::atan2(vel.y, horizDist));
            transform.rotation.y = yaw;
            transform.rotation.x = pitch;

            // Attack during peak of breach
            fireTimer -= dt;
            if (fireTimer <= 0.0f && transform.position.y > 5.0f) {
                fireTimer = 0.4f;
                glm::vec3 mouthPos = transform.position + glm::vec3(0.0f, 0.0f, 3.2f);
                projectiles.SpawnLaser(mouthPos, playerPos, false, 85.0f);
                if (audio) audio->Play(SoundID::EnemyLaser, 0.5f, 0.9f);
            }
        }
    }

    // Inverse Kinematics spine constraint: segments follow preceding node
    float segSpacing = 3.2f;
    glm::vec3 leaderPos = transform.position;

    for (size_t i = 0; i < segments.size(); ++i) {
        glm::vec3 diff = segments[i].position - leaderPos;
        float d = glm::length(diff);
        if (d > 0.001f) {
            segments[i].position = leaderPos + (diff / d) * segSpacing;
            // Face along segment direction
            float syaw = glm::degrees(std::atan2(-diff.x, -diff.z));
            float sdist = std::sqrt(diff.x*diff.x + diff.z*diff.z);
            float spitch = glm::degrees(std::atan2(-diff.y, sdist));
            segments[i].rotation.y = syaw;
            segments[i].rotation.x = spitch;
        }
        leaderPos = segments[i].position;
    }
}

void BossMechaWorm::Draw(const Shader& shader) const {
    if (state == WormBossState::Inactive || state == WormBossState::Defeated) return;

    shader.SetInt("uUseLighting", 1);
    shader.SetFloat("uAlpha", 1.0f);

    // 1. Draw Head
    glm::mat4 mHead = transform.GetModelMatrix();
    shader.SetMat4("uModel", mHead);
    headMesh.Draw(shader);

    // 2. Draw Body Segments
    for (size_t i = 0; i < segments.size(); ++i) {
        if (segments[i].destroyed) continue;

        glm::mat4 mSeg = glm::mat4(1.0f);
        mSeg = glm::translate(mSeg, segments[i].position);
        mSeg = glm::rotate(mSeg, glm::radians(segments[i].rotation.y), glm::vec3(0, 1, 0));
        mSeg = glm::rotate(mSeg, glm::radians(segments[i].rotation.x), glm::vec3(1, 0, 0));
        mSeg = glm::scale(mSeg, glm::vec3(1.0f - i * 0.035f)); // Taper towards tail
        shader.SetMat4("uModel", mSeg);
        segmentMesh.Draw(shader);
    }
}

int BossMechaWorm::FindLockTarget(const glm::vec3& playerPos, const glm::vec3& aimPos, glm::vec3& outLockPos) const {
    if (transform.position.y < -6.0f) return -1; // Cannot lock-on while underground!

    float bestDistSq = 16.0f * 16.0f;
    int target = -1;

    // Check head
    float zDist = playerPos.z - transform.position.z;
    if (zDist >= 10.0f && zDist <= 180.0f) {
        float dx = transform.position.x - aimPos.x;
        float dy = transform.position.y - aimPos.y;
        float dsq = dx*dx + dy*dy;
        if (dsq < bestDistSq) {
            bestDistSq = dsq;
            target = 1;
            outLockPos = transform.position;
        }
    }

    // Check segments
    for (size_t i = 0; i < segments.size(); ++i) {
        if (segments[i].destroyed || segments[i].position.y < -6.0f) continue;
        float sZ = playerPos.z - segments[i].position.z;
        if (sZ < 10.0f || sZ > 180.0f) continue;
        float dx = segments[i].position.x - aimPos.x;
        float dy = segments[i].position.y - aimPos.y;
        float dsq = dx*dx + dy*dy;
        if (dsq < bestDistSq) {
            bestDistSq = dsq;
            target = static_cast<int>(i + 2);
            outLockPos = segments[i].position;
        }
    }

    return target;
}

bool BossMechaWorm::CheckLaserHit(const glm::vec3& laserPos, float laserRadius, float damage,
                                  ParticleSystem& particles, Camera& camera, int& outScoreGained) {
    if (state == WormBossState::Inactive || state == WormBossState::Approaching || state == WormBossState::Defeated) {
        return false;
    }

    // Head Hit
    if (transform.position.y > -7.0f) {
        float dHead = glm::distance(laserPos, transform.position);
        if (dHead <= (3.0f + laserRadius)) {
            headHealth -= damage;
            particles.SpawnExplosion(laserPos, 6, glm::vec3(1.0f, 0.8f, 0.2f));
            outScoreGained += 60;
            if (headHealth <= 0.0f) {
                state = WormBossState::DeathSpiral;
                camera.TriggerShake(1.5f, 0.6f);
                outScoreGained += 4000;
            }
            return true;
        }
    }

    // Segment Node Hits
    for (size_t i = 0; i < segments.size(); ++i) {
        if (segments[i].destroyed || segments[i].position.y < -7.0f) continue;

        float dSeg = glm::distance(laserPos, segments[i].position);
        if (dSeg <= (2.4f + laserRadius)) {
            segments[i].health -= damage;
            particles.SpawnExplosion(laserPos, 6, glm::vec3(0.3f, 0.85f, 1.0f));
            outScoreGained += 40;

            if (segments[i].health <= 0.0f) {
                segments[i].destroyed = true;
                particles.SpawnExplosion(segments[i].position, 28, glm::vec3(1.0f, 0.4f, 0.1f));
                camera.TriggerShake(0.5f, 0.3f);
                outScoreGained += 500;
                headHealth -= 40.0f; // Weakpoint damage directly hurts the worm!
            }
            return true;
        }
    }

    return false;
}

void BossMechaWorm::ApplyShockwaveDamage(const glm::vec3& shockPos, float radius, float damage,
                                        ParticleSystem& particles, Camera& camera, int& outScoreGained) {
    if (state == WormBossState::Inactive || state == WormBossState::Approaching || state == WormBossState::Defeated) {
        return;
    }

    if (transform.position.y > -7.0f && glm::distance(shockPos, transform.position) <= radius) {
        headHealth -= damage;
        particles.SpawnExplosion(transform.position, 16, glm::vec3(0.4f, 0.8f, 1.0f));
        outScoreGained += 150;
    }

    for (size_t i = 0; i < segments.size(); ++i) {
        if (segments[i].destroyed || segments[i].position.y < -7.0f) continue;
        if (glm::distance(shockPos, segments[i].position) <= radius) {
            segments[i].health -= damage;
            particles.SpawnExplosion(segments[i].position, 14, glm::vec3(0.4f, 0.8f, 1.0f));
            outScoreGained += 100;
            if (segments[i].health <= 0.0f) {
                segments[i].destroyed = true;
                headHealth -= 40.0f;
            }
        }
    }
}

float BossMechaWorm::GetHealthRatio() const {
    float cur = std::max(0.0f, headHealth);
    for (const auto& s : segments) {
        if (!s.destroyed) cur += s.health;
    }
    return std::clamp(cur / totalMaxHealth, 0.0f, 1.0f);
}
