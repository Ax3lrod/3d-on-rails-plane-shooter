#ifndef BOSS_TWIN_HELICOPTERS_H
#define BOSS_TWIN_HELICOPTERS_H

#include <glm/glm.hpp>
#include <string>
#include "IBoss.h"
#include "Mesh.h"

enum class HeliBossState {
    Inactive,
    Approaching,
    PincerPatrol,
    EnrageSolo,
    DeathSpiral,
    Defeated
};

struct HelicopterUnit {
    Transform transform;
    float health = 350.0f;
    float maxHealth = 350.0f;
    float radius = 3.2f;
    bool destroyed = false;
    float fireTimer = 0.5f;
    float fireInterval = 0.22f;
    float missileTimer = 2.5f;
    float rotorAngle = 0.0f;
    float orbitAngle = 0.0f;
    float orbitRadius = 24.0f;
    float targetHeight = 4.0f;
};

class BossTwinHelicopters : public IBoss {
public:
    HeliBossState state;

    Mesh fuselageMesh;
    Mesh mainRotorMesh;
    Mesh tailRotorMesh;

    HelicopterUnit heliA; // Viper-1 (Alpha)
    HelicopterUnit heliB; // Viper-2 (Beta)

    float stateTime;
    float warningTimer;
    float approachProgress;
    float deathTimer;
    float nextExplosionTimer;

    BossTwinHelicopters();

    void Spawn(float playerZ) override;
    void Update(float dt, float playerZ, const glm::vec3& playerPos,
                ProjectileManager& projectiles, ParticleSystem& particles, Camera& camera,
                SoundManager* audio = nullptr) override;
    void Draw(const Shader& shader) const override;
    void Reset() override;

    int FindLockTarget(const glm::vec3& playerPos, const glm::vec3& aimPos, glm::vec3& outLockPos) const override;
    bool CheckLaserHit(const glm::vec3& laserPos, float laserRadius, float damage,
                       ParticleSystem& particles, Camera& camera, int& outScoreGained) override;
    void ApplyShockwaveDamage(const glm::vec3& shockPos, float radius, float damage,
                              ParticleSystem& particles, Camera& camera, int& outScoreGained) override;

    float GetHealthRatio() const override;
    bool IsActive() const override { return state != HeliBossState::Inactive && state != HeliBossState::Defeated; }
    bool IsWarning() const override { return state == HeliBossState::Approaching; }
    bool IsDefeated() const override { return state == HeliBossState::Defeated; }
    std::string GetBossName() const override { return "TWIN VIPER GUNSHIPS"; }

    bool IsSubsystem1Down() const override { return heliA.destroyed; }
    bool IsSubsystem2Down() const override { return heliB.destroyed; }
    bool IsShieldDown() const override { return true; }
    bool IsCoreExposed() const override { return state == HeliBossState::EnrageSolo; }
};

#endif
