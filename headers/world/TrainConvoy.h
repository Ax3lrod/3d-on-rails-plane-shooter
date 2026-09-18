#ifndef TRAIN_CONVOY_H
#define TRAIN_CONVOY_H

#include <glm/glm.hpp>
#include <vector>
#include "Transform.h"
#include "Mesh.h"
#include "shaderClass.h"
#include "CombatSystem.h"

class SoundManager;

struct TrainCar {
    glm::vec3 position;
    float health = 80.0f;
    bool destroyed = false;
    bool hasTurret = false;
    float fireTimer = 1.0f;
};

class TrainConvoy {
public:
    bool active;
    glm::vec3 locomotivePos;
    float speed;
    float trackX;

    Mesh locoMesh;
    Mesh cargoMesh;
    Mesh tracksMesh;
    Mesh turretMesh;

    std::vector<TrainCar> cars;
    float smokeTimer;
    float hornTimer;

    TrainConvoy(float xPosition = 24.0f);

    void Spawn(float playerZ, float trackXPos = 24.0f);
    void Update(float dt, float playerZ, const glm::vec3& playerPos,
                ProjectileManager& projectiles, ParticleSystem& particles,
                SoundManager* audio = nullptr);
    void Draw(const Shader& shader, float playerZ) const;
    void Reset();

    bool CheckLaserHit(const glm::vec3& laserPos, float laserRadius, float damage,
                       ParticleSystem& particles, int& outScoreGained);
};

#endif
