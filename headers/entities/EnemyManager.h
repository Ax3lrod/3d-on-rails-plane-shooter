#ifndef ENEMY_MANAGER_H
#define ENEMY_MANAGER_H

#include <glm/glm.hpp>
#include <vector>
#include "Transform.h"
#include "Mesh.h"
#include "CombatSystem.h"

struct Enemy {
    Transform transform;
    glm::vec3 originPos;
    float health;
    float maxHealth;
    float radius;
    int patternType;
    float timeAlive;
    float fireTimer;
    float fireCooldown;
    int scoreValue;
    bool active;
};

class SoundManager;

class EnemyManager {
public:
    std::vector<Enemy> enemies;
    Mesh enemyMesh;

    float spawnTimer;
    float spawnInterval;
    float lastPlayerZ;

    EnemyManager();

    int FindLockTarget(const glm::vec3& playerPos, const glm::vec3& aimPos) const;
    void SpawnWave(float playerZ);
    void Update(float playerZ, const glm::vec3& playerPos, ProjectileManager& projectiles, float dt, SoundManager* audio = nullptr);
    void Draw(const Shader& shader) const;
    void Clear();
};

#endif
