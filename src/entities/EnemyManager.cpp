#include "EnemyManager.h"
#include "SoundManager.h"
#include <algorithm>
#include <cmath>
#include <cstdlib>

EnemyManager::EnemyManager()
    : enemyMesh(Mesh::CreateEnemyDrone(
          glm::vec3(0.75f, 0.15f, 0.18f), // Menacing crimson-dark hull
          glm::vec3(1.0f, 0.2f, 0.1f)     // Glowing optic eye
      )),
      spawnTimer(0.0f),
      spawnInterval(3.2f),
      lastPlayerZ(0.0f) {}

int EnemyManager::FindLockTarget(const glm::vec3& playerPos, const glm::vec3& aimPos) const {
    int bestIndex = -1;
    float bestDistSq = 14.0f * 14.0f;

    for (size_t i = 0; i < enemies.size(); ++i) {
        const auto& e = enemies[i];
        if (!e.active) continue;

        float zDist = playerPos.z - e.transform.position.z;
        if (zDist < 15.0f || zDist > 160.0f) continue;

        float dx = e.transform.position.x - aimPos.x;
        float dy = e.transform.position.y - aimPos.y;
        float distSq = dx * dx + dy * dy;

        if (distSq < bestDistSq) {
            bestDistSq = distSq;
            bestIndex = static_cast<int>(i);
        }
    }
    return bestIndex;
}

void EnemyManager::SpawnWave(float playerZ) {
    float spawnZ = playerZ - 220.0f; // 220 units ahead
    int formation = rand() % 3;

    if (formation == 0) {
        // V-Formation (3 fighters)
        float centerX = ((float)rand() / RAND_MAX * 2.0f - 1.0f) * 6.0f;
        float centerY = ((float)rand() / RAND_MAX * 2.0f - 1.0f) * 3.0f;

        enemies.push_back({
            Transform(glm::vec3(centerX, centerY, spawnZ)),
            glm::vec3(centerX, centerY, spawnZ),
            30.0f, 30.0f, 1.8f, 0, 0.0f, 0.8f, 1.6f, 150, true
        });
        enemies.push_back({
            Transform(glm::vec3(centerX - 4.5f, centerY + 1.2f, spawnZ - 12.0f)),
            glm::vec3(centerX - 4.5f, centerY + 1.2f, spawnZ - 12.0f),
            30.0f, 30.0f, 1.8f, 0, 0.0f, 1.1f, 1.6f, 150, true
        });
        enemies.push_back({
            Transform(glm::vec3(centerX + 4.5f, centerY + 1.2f, spawnZ - 12.0f)),
            glm::vec3(centerX + 4.5f, centerY + 1.2f, spawnZ - 12.0f),
            30.0f, 30.0f, 1.8f, 0, 0.0f, 1.3f, 1.6f, 150, true
        });
    } else if (formation == 1) {
        // Swooping Sine Squadron
        float startX = ((float)rand() / RAND_MAX * 2.0f - 1.0f) * 8.0f;
        for (int i = 0; i < 2; ++i) {
            enemies.push_back({
                Transform(glm::vec3(startX + (i == 0 ? -3.0f : 3.0f), 5.0f, spawnZ - i * 18.0f)),
                glm::vec3(startX + (i == 0 ? -3.0f : 3.0f), 5.0f, spawnZ - i * 18.0f),
                35.0f, 35.0f, 1.8f, 1, (float)i * 0.5f, 0.6f + (float)i * 0.4f, 1.8f, 200, true
            });
        }
    } else {
        // Corkscrew Heavy Interceptor
        float cx = ((float)rand() / RAND_MAX * 2.0f - 1.0f) * 6.0f;
        float cy = ((float)rand() / RAND_MAX * 2.0f - 1.0f) * 3.5f;
        enemies.push_back({
            Transform(glm::vec3(cx, cy, spawnZ)),
            glm::vec3(cx, cy, spawnZ),
            60.0f, 60.0f, 2.2f, 2, 0.0f, 0.5f, 1.2f, 350, true
        });
    }
}

void EnemyManager::Update(float playerZ, const glm::vec3& playerPos, ProjectileManager& projectiles, float dt, SoundManager* audio) {
    spawnTimer += dt;
    if (spawnTimer >= spawnInterval) {
        spawnTimer = 0.0f;
        SpawnWave(playerZ);
    }

    float forwardFlightSpeed = 22.0f;

    for (auto& e : enemies) {
        if (!e.active) continue;

        e.timeAlive += dt;
        e.fireTimer -= dt;

        // Enemies fly towards positive Z (meeting player)
        e.transform.position.z += forwardFlightSpeed * dt;

        // Flight patterns
        if (e.patternType == 0) {
            // Straight pass with subtle banking
            e.transform.rotation.z = std::sin(e.timeAlive * 3.0f) * 15.0f;
        } else if (e.patternType == 1) {
            // Sine swoop
            e.transform.position.y = e.originPos.y + std::sin(e.timeAlive * 2.5f) * 4.5f;
            e.transform.position.x = e.originPos.x + std::cos(e.timeAlive * 2.0f) * 3.0f;
            e.transform.rotation.z = -std::sin(e.timeAlive * 2.0f) * 35.0f;
        } else if (e.patternType == 2) {
            // Corkscrew
            float radius = 4.2f;
            e.transform.position.x = e.originPos.x + std::cos(e.timeAlive * 3.2f) * radius;
            e.transform.position.y = e.originPos.y + std::sin(e.timeAlive * 3.2f) * radius;
            e.transform.rotation.z += 220.0f * dt;
        }

        // Enemy firing logic
        float distZ = e.transform.position.z - playerPos.z; // negative when ahead of player
        if (distZ < -30.0f && distZ > -150.0f) {
            if (e.fireTimer <= 0.0f) {
                e.fireTimer = e.fireCooldown;
                // Fire red plasma bolt at player
                projectiles.SpawnLaser(e.transform.position, playerPos, false, 85.0f);
                if (audio) {
                    audio->Play(SoundID::EnemyLaser, 0.40f, 0.95f + ((rand() % 10) * 0.01f));
                }
            }
        }

        // Deactivate if far behind player
        if (e.transform.position.z > playerPos.z + 40.0f) {
            e.active = false;
        }
    }

    enemies.erase(
        std::remove_if(enemies.begin(), enemies.end(),
                       [](const Enemy& e) { return !e.active; }),
        enemies.end()
    );
}

void EnemyManager::Draw(const Shader& shader) const {
    shader.SetInt("uUseLighting", 1);
    shader.SetFloat("uAlpha", 1.0f);

    for (const auto& e : enemies) {
        if (!e.active) continue;
        shader.SetMat4("uModel", e.transform.GetModelMatrix());
        enemyMesh.Draw(shader);
    }
}

void EnemyManager::Clear() {
    enemies.clear();
    spawnTimer = 0.0f;
}
