#ifndef ENEMY_MANAGER_H
#define ENEMY_MANAGER_H

#include <glm/glm.hpp>
#include <vector>
#include <string>
#include "Transform.h"
#include "Mesh.h"
#include "CombatSystem.h"

enum class EnemyType {
    Drone,             // Aerial crimson scout
    GroundTank,        // Mobile armored tread vehicle on the plains
    FlakTurret,        // Stationary anti-air pillbox bunker
    EliteInterceptor   // High-speed aggressive rival starfighter
};

struct Enemy {
    Transform transform;
    glm::vec3 originPos;
    EnemyType type;
    float health;
    float maxHealth;
    float radius;
    int patternType;
    float timeAlive;
    float fireTimer;
    float fireCooldown;
    int scoreValue;
    bool active;
    float aimAngleYaw;
    float aimAnglePitch;
};

class SoundManager;

class EnemyManager {
public:
    std::vector<Enemy> enemies;
    Mesh droneMesh;
    Mesh tankMesh;
    Mesh turretMesh;
    Mesh interceptorMesh;

    float spawnTimer;
    float spawnInterval;
    float lastPlayerZ;

    EnemyManager();

    int FindLockTarget(const glm::vec3& playerPos, const glm::vec3& aimPos) const;
    void SpawnWave(float playerZ);
    void SpawnCustomWave(EnemyType type, const std::string& formation, int count,
                         float spawnX, float spawnY, float spawnZ, float spacingX = 4.5f, float spacingZ = 12.0f);
    void Update(float playerZ, const glm::vec3& playerPos, ProjectileManager& projectiles, float dt, SoundManager* audio = nullptr);
    void Draw(const Shader& shader) const;
    void Clear();
};

#endif
