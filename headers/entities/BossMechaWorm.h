#ifndef BOSS_MECHA_WORM_H
#define BOSS_MECHA_WORM_H

#include <glm/glm.hpp>
#include <string>
#include <vector>
#include "IBoss.h"
#include "Mesh.h"

enum class WormBossState {
    Inactive,
    Approaching,
    SubterraneanBurrow,
    BreachAscent,
    BreachDescent,
    CoilStrike,
    DeathSpiral,
    Defeated
};

struct WormSegmentState {
    glm::vec3 position;
    glm::vec3 rotation;
    float health = 100.0f;
    bool destroyed = false;
};

class BossMechaWorm : public IBoss {
public:
    WormBossState state;

    Mesh headMesh;
    Mesh segmentMesh;

    std::vector<WormSegmentState> segments;
    float headHealth;
    float headMaxHealth;
    float totalMaxHealth;

    float breachTimer;
    float breachDuration;
    glm::vec3 breachStartPos;
    glm::vec3 breachApexPos;
    glm::vec3 breachEndPos;

    float fireTimer;
    float mandibleAngle;
    float dustSpawnTimer;

    float stateTime;
    float warningTimer;
    float approachProgress;
    float deathTimer;
    float nextExplosionTimer;

    BossMechaWorm();

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
    bool IsActive() const override { return state != WormBossState::Inactive && state != WormBossState::Defeated; }
    bool IsWarning() const override { return state == WormBossState::Approaching; }
    bool IsDefeated() const override { return state == WormBossState::Defeated; }
    std::string GetBossName() const override { return "CYBERNETIC SANDWORM"; }

    bool IsSubsystem1Down() const override { return segments.empty() ? false : segments[0].destroyed; }
    bool IsSubsystem2Down() const override { return segments.empty() ? false : segments.back().destroyed; }
    bool IsShieldDown() const override { return true; }
    bool IsCoreExposed() const override { return transform.position.y > -5.0f; } // Exposed while airborne!
};

#endif
