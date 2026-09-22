#ifndef BOSS_WALKING_ROBOT_H
#define BOSS_WALKING_ROBOT_H

#include "IBoss.h"
#include "../renderer/Mesh.h"
#include <vector>

class BossWalkingRobot : public IBoss {
public:
    BossWalkingRobot();

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

    bool IsActive() const override { return !isDead && currentPhase != Phase::Death; }
    bool IsWarning() const override { return isWarningActive; }
    bool IsDefeated() const override { return isDead; }
    std::string GetBossName() const override { return "IRON COLOSSUS"; }

    float GetHealthRatio() const override { return hp / maxHp; }
    
    // Arena bounds for all-range mode
    glm::vec3 GetArenaCenter() const { return arenaCenter; }
    float GetArenaRadius() const { return 280.0f; }

private:
    enum class Phase { Walk, Stomp, Berserk, Death };
    Phase currentPhase = Phase::Walk;

    Mesh bodyMesh;
    Mesh shadowMesh;

    float hp;
    float maxHp;
    float stateTimer;
    float walkCycle;
    float yawAngle;
    float targetYaw;
    float cannonCooldown;
    float stompCooldown;
    float beamCooldown;
    float beamBurstTimer;
    int beamBurstCount;
    float deathTimer;
    bool isDead;
    bool isWarningActive;
    glm::vec3 arenaCenter;
};

#endif
