#ifndef I_BOSS_H
#define I_BOSS_H

#include <glm/glm.hpp>
#include <string>
#include "Transform.h"
#include "shaderClass.h"
#include "CombatSystem.h"
#include "Camera.h"

class SoundManager;

class IBoss {
public:
    Transform transform;

    virtual ~IBoss() = default;
    virtual void Spawn(float playerZ) = 0;
    virtual void Update(float dt, float playerZ, const glm::vec3& playerPos,
                        ProjectileManager& projectiles, ParticleSystem& particles, Camera& camera,
                        SoundManager* audio = nullptr) = 0;
    virtual void Draw(const Shader& shader) const = 0;
    virtual void Reset() = 0;

    virtual int FindLockTarget(const glm::vec3& playerPos, const glm::vec3& aimPos, glm::vec3& outLockPos) const = 0;
    virtual bool CheckLaserHit(const glm::vec3& laserPos, float laserRadius, float damage,
                               ParticleSystem& particles, Camera& camera, int& outScoreGained) = 0;
    virtual void ApplyShockwaveDamage(const glm::vec3& shockPos, float radius, float damage,
                                      ParticleSystem& particles, Camera& camera, int& outScoreGained) = 0;

    virtual float GetHealthRatio() const = 0;
    virtual bool IsActive() const = 0;
    virtual bool IsWarning() const = 0;
    virtual bool IsDefeated() const = 0;
    virtual std::string GetBossName() const = 0;

    virtual bool IsSubsystem1Down() const { return false; }
    virtual bool IsSubsystem2Down() const { return false; }
    virtual bool IsShieldDown() const { return true; }
    virtual bool IsCoreExposed() const { return true; }
};

#endif
