#include "EnemyManager.h"
#include "SoundManager.h"
#include <algorithm>
#include <cmath>
#include <cstdlib>

EnemyManager::EnemyManager()
    : droneMesh(Mesh::CreateEnemyDrone(
          glm::vec3(0.75f, 0.15f, 0.18f), // Menacing crimson-dark hull
          glm::vec3(1.0f, 0.2f, 0.1f)     // Glowing optic eye
      )),
      tankMesh(Mesh::CreateGroundTank(
          glm::vec3(0.22f, 0.23f, 0.26f), // Tread metal
          glm::vec3(0.36f, 0.44f, 0.32f), // Olive drab armor chassis
          glm::vec3(0.28f, 0.35f, 0.26f)  // Turret cupola
      )),
      turretMesh(Mesh::CreateFlakTurret(
          glm::vec3(0.42f, 0.44f, 0.48f), // Fortified concrete pillbox
          glm::vec3(0.20f, 0.22f, 0.25f)  // Steel flak barrels
      )),
      interceptorMesh(Mesh::CreateEliteInterceptor(
          glm::vec3(0.16f, 0.18f, 0.24f), // Dark navy stealth hull
          glm::vec3(0.88f, 0.15f, 0.22f), // Crimson predator wing accents
          glm::vec3(1.0f, 0.78f, 0.12f)   // Luminous gold visor
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

void EnemyManager::SpawnCustomWave(EnemyType type, const std::string& formation, int count,
                                   float spawnX, float spawnY, float spawnZ, float spacingX, float spacingZ) {
    if (count <= 0) return;

    for (int i = 0; i < count; ++i) {
        float posX = spawnX;
        float posY = spawnY;
        float posZ = spawnZ;

        if (formation == "line") {
            posX = spawnX + (i - (count - 1) * 0.5f) * spacingX;
            posZ = spawnZ;
        } else if (formation == "column") {
            posX = spawnX;
            posZ = spawnZ - i * spacingZ;
        } else if (formation == "v_formation") {
            if (i == 0) {
                posX = spawnX;
                posZ = spawnZ;
            } else {
                int wing = (i % 2 == 1) ? -1 : 1;
                int rank = (i + 1) / 2;
                posX = spawnX + wing * rank * spacingX;
                posZ = spawnZ - rank * spacingZ;
            }
        } else if (formation == "staggered") {
            float side = (i % 2 == 0) ? -1.0f : 1.0f;
            posX = spawnX + side * spacingX * 0.5f;
            posZ = spawnZ - i * spacingZ;
        } else { // "single" or default
            posX = spawnX;
            posZ = spawnZ;
        }

        Enemy e;
        e.type = type;
        e.transform = Transform(glm::vec3(posX, posY, posZ));
        e.originPos = glm::vec3(posX, posY, posZ);
        e.timeAlive = 0.0f;
        e.active = true;
        e.aimAngleYaw = 0.0f;
        e.aimAnglePitch = 0.0f;

        switch (type) {
            case EnemyType::Drone:
                e.health = 30.0f;
                e.maxHealth = 30.0f;
                e.radius = 1.8f;
                e.patternType = (formation == "v_formation") ? 0 : 1;
                e.fireTimer = 0.6f + (i * 0.25f);
                e.fireCooldown = 1.6f;
                e.scoreValue = 150;
                break;

            case EnemyType::GroundTank:
                // Ground units locked to ground plane (Y = -7.5)
                e.transform.position.y = -7.5f;
                e.originPos.y = -7.5f;
                e.health = 80.0f;
                e.maxHealth = 80.0f;
                e.radius = 2.4f;
                e.patternType = 0;
                e.fireTimer = 0.8f + (i * 0.35f);
                e.fireCooldown = 2.2f;
                e.scoreValue = 300;
                break;

            case EnemyType::FlakTurret:
                // Stationary anti-air pillbox bunker on the ground
                e.transform.position.y = -7.5f;
                e.originPos.y = -7.5f;
                e.health = 120.0f;
                e.maxHealth = 120.0f;
                e.radius = 2.6f;
                e.patternType = 0;
                e.fireTimer = 0.5f + (i * 0.4f);
                e.fireCooldown = 1.8f;
                e.scoreValue = 450;
                break;

            case EnemyType::EliteInterceptor:
                e.health = 60.0f;
                e.maxHealth = 60.0f;
                e.radius = 2.2f;
                e.patternType = 2;
                e.fireTimer = 0.4f + (i * 0.2f);
                e.fireCooldown = 1.1f;
                e.scoreValue = 500;
                break;
        }

        enemies.push_back(e);
    }
}

void EnemyManager::SpawnWave(float playerZ) {
    float spawnZ = playerZ - 220.0f; // 220 units ahead
    int waveType = rand() % 4;

    if (waveType == 0) {
        // V-Formation Drones
        float cx = ((float)rand() / RAND_MAX * 2.0f - 1.0f) * 6.0f;
        float cy = ((float)rand() / RAND_MAX * 2.0f - 1.0f) * 3.0f;
        SpawnCustomWave(EnemyType::Drone, "v_formation", 3, cx, cy, spawnZ, 4.5f, 12.0f);
    } else if (waveType == 1) {
        // Ground Tank Patrol on the plains
        float cx = ((float)rand() / RAND_MAX * 2.0f - 1.0f) * 12.0f;
        SpawnCustomWave(EnemyType::GroundTank, "column", 2, cx, -7.5f, spawnZ, 0.0f, 16.0f);
    } else if (waveType == 2) {
        // Fortified Flak Turret Bunker
        float cx = ((float)rand() / RAND_MAX * 2.0f - 1.0f) * 14.0f;
        SpawnCustomWave(EnemyType::FlakTurret, "single", 1, cx, -7.5f, spawnZ);
    } else {
        // Elite Interceptor Dogfight Pair
        float cx = ((float)rand() / RAND_MAX * 2.0f - 1.0f) * 8.0f;
        SpawnCustomWave(EnemyType::EliteInterceptor, "staggered", 2, cx, 4.0f, spawnZ, 8.0f, 15.0f);
    }
}

void EnemyManager::Update(float playerZ, const glm::vec3& playerPos, ProjectileManager& projectiles, float dt, SoundManager* audio) {
    spawnTimer += dt;
    if (spawnTimer >= spawnInterval) {
        spawnTimer = 0.0f;
        SpawnWave(playerZ);
    }

    for (auto& e : enemies) {
        if (!e.active) continue;

        e.timeAlive += dt;
        e.fireTimer -= dt;

        if (e.type == EnemyType::Drone) {
            float forwardFlightSpeed = 22.0f;
            e.transform.position.z += forwardFlightSpeed * dt;

            // Flight patterns
            if (e.patternType == 0) {
                e.transform.rotation.z = std::sin(e.timeAlive * 3.0f) * 15.0f;
            } else if (e.patternType == 1) {
                e.transform.position.y = e.originPos.y + std::sin(e.timeAlive * 2.5f) * 4.5f;
                e.transform.position.x = e.originPos.x + std::cos(e.timeAlive * 2.0f) * 3.0f;
                e.transform.rotation.z = -std::sin(e.timeAlive * 2.0f) * 35.0f;
            } else if (e.patternType == 2) {
                float radius = 4.2f;
                e.transform.position.x = e.originPos.x + std::cos(e.timeAlive * 3.2f) * radius;
                e.transform.position.y = e.originPos.y + std::sin(e.timeAlive * 3.2f) * radius;
                e.transform.rotation.z += 220.0f * dt;
            }

            // Enemy firing logic
            float distZ = e.transform.position.z - playerPos.z;
            if (distZ < -25.0f && distZ > -150.0f && e.fireTimer <= 0.0f) {
                e.fireTimer = e.fireCooldown;
                projectiles.SpawnLaser(e.transform.position, playerPos, false, 85.0f);
                if (audio) {
                    audio->Play(SoundID::EnemyLaser, 0.40f, 0.95f + ((rand() % 10) * 0.01f));
                }
            }
        }
        else if (e.type == EnemyType::GroundTank) {
            // Locked to ground level
            e.transform.position.y = -7.5f;
            // Slow advance along +Z
            e.transform.position.z += 10.0f * dt;

            // Subtle lateral strafe
            e.transform.position.x = e.originPos.x + std::sin(e.timeAlive * 0.8f) * 2.0f;

            // Swivel towards player
            float dx = playerPos.x - e.transform.position.x;
            float dz = playerPos.z - e.transform.position.z;
            float targetYaw = glm::degrees(std::atan2(dx, dz));
            e.transform.rotation.y = glm::mix(e.transform.rotation.y, targetYaw, dt * 3.0f);

            // Heavy twin cannon bursts
            float distZ = e.transform.position.z - playerPos.z;
            if (distZ < -20.0f && distZ > -160.0f && e.fireTimer <= 0.0f) {
                e.fireTimer = e.fireCooldown;
                glm::vec3 muzzleL = e.transform.position + glm::vec3(-0.5f, 1.4f, 1.2f);
                glm::vec3 muzzleR = e.transform.position + glm::vec3( 0.5f, 1.4f, 1.2f);
                projectiles.SpawnLaser(muzzleL, playerPos + glm::vec3(-0.8f, 0.0f, 0.0f), false, 75.0f);
                projectiles.SpawnLaser(muzzleR, playerPos + glm::vec3( 0.8f, 0.0f, 0.0f), false, 75.0f);
                if (audio) {
                    audio->Play(SoundID::EnemyLaser, 0.50f, 0.80f);
                }
            }
        }
        else if (e.type == EnemyType::FlakTurret) {
            // Stationary bunker locked to ground
            e.transform.position.y = -7.5f;

            // Swivel dome cupola to track oncoming starfighter
            float dx = playerPos.x - e.transform.position.x;
            float dz = playerPos.z - e.transform.position.z;
            float targetYaw = glm::degrees(std::atan2(dx, dz));
            e.transform.rotation.y = glm::mix(e.transform.rotation.y, targetYaw, dt * 4.5f);

            // Anti-air flak fire
            float distZ = e.transform.position.z - playerPos.z;
            if (distZ < -25.0f && distZ > -175.0f && e.fireTimer <= 0.0f) {
                e.fireTimer = e.fireCooldown;
                glm::vec3 flakPos = e.transform.position + glm::vec3(0.0f, 2.0f, 0.5f);
                projectiles.SpawnLaser(flakPos, playerPos, false, 95.0f);
                if (audio) {
                    audio->Play(SoundID::EnemyLaser, 0.48f, 1.15f);
                }
            }
        }
        else if (e.type == EnemyType::EliteInterceptor) {
            // High-speed aggressive attack runs
            float interceptorSpeed = 30.0f;
            e.transform.position.z += interceptorSpeed * dt;

            // Pursue player X/Y with smooth spring damping
            float targetX = e.originPos.x + std::sin(e.timeAlive * 2.2f) * 5.0f;
            float targetY = e.originPos.y + std::cos(e.timeAlive * 1.8f) * 2.5f;
            e.transform.position.x = glm::mix(e.transform.position.x, targetX, dt * 3.5f);
            e.transform.position.y = glm::mix(e.transform.position.y, targetY, dt * 3.5f);

            // Hard banking roll into maneuvers
            float rollBank = -(targetX - e.transform.position.x) * 14.0f;
            e.transform.rotation.z = std::clamp(rollBank, -45.0f, 45.0f);

            // Rapid dual plasma fire
            float distZ = e.transform.position.z - playerPos.z;
            if (distZ < -20.0f && distZ > -140.0f && e.fireTimer <= 0.0f) {
                e.fireTimer = e.fireCooldown;
                glm::vec3 tipL = e.transform.position + glm::vec3(-2.2f, 0.0f, 0.8f);
                glm::vec3 tipR = e.transform.position + glm::vec3( 2.2f, 0.0f, 0.8f);
                projectiles.SpawnLaser(tipL, playerPos, false, 110.0f);
                projectiles.SpawnLaser(tipR, playerPos, false, 110.0f);
                if (audio) {
                    audio->Play(SoundID::EnemyLaser, 0.45f, 1.25f);
                }
            }
        }

        // Deactivate if far behind player
        if (e.transform.position.z > playerPos.z + 45.0f) {
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
        switch (e.type) {
            case EnemyType::Drone:
                droneMesh.Draw(shader);
                break;
            case EnemyType::GroundTank:
                tankMesh.Draw(shader);
                break;
            case EnemyType::FlakTurret:
                turretMesh.Draw(shader);
                break;
            case EnemyType::EliteInterceptor:
                interceptorMesh.Draw(shader);
                break;
        }
    }
}

void EnemyManager::Clear() {
    enemies.clear();
    spawnTimer = 0.0f;
}
