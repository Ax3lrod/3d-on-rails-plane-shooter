#ifndef COMBAT_SYSTEM_H
#define COMBAT_SYSTEM_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>
#include "Mesh.h"
#include "shaderClass.h"

struct Projectile {
    glm::vec3 position;
    glm::vec3 velocity;
    glm::vec3 color;
    float radius;
    float lifetime;
    bool isPlayer;
    bool active;
};

struct ChargedShot {
    glm::vec3 position;
    glm::vec3 velocity;
    float radius;
    float damage;
    float aoeRadius;
    bool hasHomingTarget;
    glm::vec3 homingTarget;
    float homingTurnSpeed;
    float lifetime;
    bool active;
};

struct SmartBomb {
    glm::vec3 position;
    glm::vec3 velocity;
    float radius;
    float lifetime;
    bool active;
};

struct Shockwave {
    glm::vec3 position;
    float currentRadius;
    float maxRadius;
    float expansionSpeed;
    float damage;
    float alpha;
    bool active;
};

struct Particle {
    glm::vec3 position;
    glm::vec3 velocity;
    glm::vec3 color;
    float lifetime;
    float maxLifetime;
    float size;
    bool active;
};

class ProjectileManager {
public:
    std::vector<Projectile> projectiles;
    Mesh laserMesh;
    Mesh enemyLaserMesh;

    ProjectileManager();

    void SpawnLaser(const glm::vec3& origin, const glm::vec3& target, bool isPlayer, float speed = 220.0f);
    void SpawnLaserWithDir(const glm::vec3& origin, const glm::vec3& dir, bool isPlayer, float speed = 80.0f,
                           const glm::vec3& color = glm::vec3(1.0f, 0.25f, 0.15f), float radius = 0.85f);
    void Update(float dt);
    void Draw(const Shader& shader) const;
    void Clear();
};

class OrdnanceManager {
public:
    std::vector<ChargedShot> chargedShots;
    std::vector<SmartBomb> smartBombs;
    std::vector<Shockwave> shockwaves;

    Mesh chargeBallMesh;
    Mesh bombMesh;
    Mesh shockwaveMesh;

    OrdnanceManager();

    void SpawnChargedShot(const glm::vec3& origin, const glm::vec3& forwardDir,
                          bool hasLock, const glm::vec3& lockTarget);
    void SpawnSmartBomb(const glm::vec3& origin, const glm::vec3& forwardDir);
    void DetonateBomb(size_t index);
    void TriggerShockwave(const glm::vec3& pos, float maxR = 36.0f, float dmg = 120.0f);

    void Update(float dt);
    void Draw(const Shader& shader) const;
    void Clear();
};

class ParticleSystem {
public:
    std::vector<Particle> particles;
    Mesh particleMesh;

    ParticleSystem();

    void SpawnExplosion(const glm::vec3& pos, int count, const glm::vec3& primaryColor);
    void SpawnThrusterSparks(const glm::vec3& pos, const glm::vec3& shipVel);
    void SpawnChargeInwardSparks(const glm::vec3& centerPos, float chargeProgress);
    void Update(float dt);
    void Draw(const Shader& shader) const;
    void Clear();
};

class TargetingReticle {
public:
    Mesh crosshairMesh;
    Mesh farPointMesh;
    Mesh lockOnMesh;

    TargetingReticle();

    void Draw(const Shader& shader, const glm::vec3& nearPos, const glm::vec3& farPos,
              bool hasLockOn, const glm::vec3& lockTargetPos, float lockAngle) const;
};

#endif
