#ifndef BOSS_MEGA_TANK_H
#define BOSS_MEGA_TANK_H

#include <glm/glm.hpp>
#include <string>
#include "IBoss.h"
#include "Mesh.h"

enum class TankBossState {
    Inactive,
    Approaching,
    CombatPatrol,
    CoreOvercharge,
    DeathSpiral,
    Defeated
};

class BossMegaTank : public IBoss {
public:
    TankBossState state;

    Mesh chassisMesh;
    Mesh turretMesh;
    Mesh coreMesh;

    // Subsystem components
    float leftTreadHp;
    float rightTreadHp;
    float leftTurretHp;
    float rightTurretHp;
    float coreHp;

    bool leftTreadDestroyed;
    bool rightTreadDestroyed;
    bool leftTurretDestroyed;
    bool rightTurretDestroyed;
    bool coreDestroyed;

    float turretYawL;
    float turretYawR;
    float corePulseTime;
    float fireTimerL;
    float fireTimerR;
    float mortarTimer;
    float chargeLaserTimer;

    float stateTime;
    float warningTimer;
    float approachProgress;
    float deathTimer;
    float nextExplosionTimer;

    BossMegaTank();

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
    bool IsActive() const override { return state != TankBossState::Inactive && state != TankBossState::Defeated; }
    bool IsWarning() const override { return state == TankBossState::Approaching; }
    bool IsDefeated() const override { return state == TankBossState::Defeated; }
    std::string GetBossName() const override { return "GOLIATH MEGA-TANK"; }

    bool IsSubsystem1Down() const override { return leftTurretDestroyed; }
    bool IsSubsystem2Down() const override { return rightTurretDestroyed; }
    bool IsShieldDown() const override { return leftTreadDestroyed && rightTreadDestroyed; }
    bool IsCoreExposed() const override { return leftTurretDestroyed || rightTurretDestroyed || coreHp < 400.0f; }
};

#endif
