#ifndef BOSS_DREADNOUGHT_H
#define BOSS_DREADNOUGHT_H

#include <glm/glm.hpp>
#include <string>
#include <vector>
#include "Transform.h"
#include "Mesh.h"
#include "shaderClass.h"
#include "CombatSystem.h"
#include "Camera.h"
#include "IBoss.h"

enum class BossState {
    Inactive,
    Approaching,
    Phase1_Subsystems,
    Phase2_ExposedCore,
    Phase3_DeathSpiral,
    Defeated
};

struct BossWeakpoint {
    std::string name;
    glm::vec3 localOffset;
    glm::vec3 worldPos;
    float health;
    float maxHealth;
    float radius;
    bool destroyed;
    bool invulnerable;
    float fireTimer;
    float fireInterval;
};

class SoundManager;

class BossDreadnought : public IBoss {
public:
    BossState state;

    // Procedural Meshes
    Mesh hullMesh;
    Mesh turretMesh;
    Mesh shieldMesh;
    Mesh coreMesh;
    Mesh maceMesh;
    Mesh plasmaRingMesh;

    // Subsystem Weakpoints
    BossWeakpoint leftTurret;
    BossWeakpoint rightTurret;
    BossWeakpoint shieldGen;
    BossWeakpoint quantumCore;

    // Timers & Combat State
    float stateTime;
    float warningTimer;
    float approachProgress;
    float attackTimer;
    float spiralAttackTimer;
    float spiralAngle;
    float deathTimer;
    float nextExplosionTimer;

    // Movement & flight animation
    float swayTime;
    float currentTurretYaw;
    float corePulseTime;
    float maceRotAngle;

    BossDreadnought();

    void Spawn(float playerZ) override;
    void Update(float dt, float playerZ, const glm::vec3& playerPos,
                ProjectileManager& projectiles, ParticleSystem& particles, Camera& camera,
                SoundManager* audio = nullptr) override;
    void Draw(const Shader& shader) const override;
    void Reset() override;

    // Combat queries and interactions
    int FindLockTarget(const glm::vec3& playerPos, const glm::vec3& aimPos, glm::vec3& outLockPos) const override;
    bool CheckLaserHit(const glm::vec3& laserPos, float laserRadius, float damage,
                       ParticleSystem& particles, Camera& camera, int& outScoreGained) override;
    void ApplyShockwaveDamage(const glm::vec3& shockPos, float radius, float damage,
                              ParticleSystem& particles, Camera& camera, int& outScoreGained) override;

    // Health and state helpers
    float GetTotalHealth() const;
    float GetMaxHealth() const;
    float GetHealthRatio() const override;
    bool IsActive() const override { return state != BossState::Inactive && state != BossState::Defeated; }
    bool IsWarning() const override { return state == BossState::Approaching; }
    bool IsDefeated() const override { return state == BossState::Defeated; }
    std::string GetBossName() const override { return "COLOSSAL DREADNOUGHT"; }

    bool IsSubsystem1Down() const override { return leftTurret.destroyed; }
    bool IsSubsystem2Down() const override { return rightTurret.destroyed; }
    bool IsShieldDown() const override { return shieldGen.destroyed; }
    bool IsCoreExposed() const override { return state == BossState::Phase2_ExposedCore; }

private:
    void UpdateSubsystemsWorldPos();
    void FirePhase1Attacks(const glm::vec3& playerPos, ProjectileManager& projectiles, float dt, SoundManager* audio = nullptr);
    void FirePhase2Attacks(const glm::vec3& playerPos, ProjectileManager& projectiles, float dt, SoundManager* audio = nullptr);
    void UpdateDeathSpiral(float dt, ParticleSystem& particles, Camera& camera);
};

#endif
