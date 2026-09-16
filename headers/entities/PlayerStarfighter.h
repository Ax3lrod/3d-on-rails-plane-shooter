#ifndef PLAYER_STARFIGHTER_H
#define PLAYER_STARFIGHTER_H

#include "Transform.h"
#include "Mesh.h"
#include "Camera.h"

class PlayerStarfighter {
public:
    Transform transform;
    Mesh mesh;
    Mesh chargeOrbMesh;

    // Flight parameters
    float baseSpeed;
    float boostSpeed;
    float brakeSpeed;
    float currentSpeed;

    // Corridor flight boundaries relative to camera rail
    float minX, maxX;
    float minY, maxY;

    // Control parameters
    float moveSpeedX;
    float moveSpeedY;
    float maxBankAngle;
    float maxPitchAngle;
    float maxYawAngle;

    // Current smoothed angles
    float currentBank;
    float currentPitch;
    float currentYaw;

    // Tactical Evasion Spin (Barrel Roll)
    bool isSpinning;
    float spinTimer;
    float spinDuration;
    float spinDirection;
    float spinRoll;

    // Boost meter & heat
    float boostMeter;
    float maxBoost;
    bool isBoosting;
    bool isBraking;
    bool isOverheated;
    float overheatTimer;

    // Combat & Shield
    float shield;
    float maxShield;
    float invulnerableTimer;
    float fireTimer;
    float fireRate;

    // Charged Shot & Lock-on
    float chargeTimer;
    float maxChargeTime;
    bool isCharging;
    bool isFullyCharged;
    bool hasLockOn;
    glm::vec3 lockTargetPos;
    float lockRotation;

    // Smart Bomb Ordnance
    int bombCount;
    int maxBombs;

    // Score and statistics
    int score;
    int ringsCollected;

    PlayerStarfighter();

    void Update(float dt);
    void HandleInput(float dt);
    void Draw(const Shader& shader) const;

    void TakeDamage(float amount);
    void AddShield(float amount);
    void TriggerSpin(float direction);
    bool CanFire() const;
    void ResetFireTimer();

    void StartCharging();
    bool ReleaseChargedShot();
    float GetChargeProgress() const;
    bool LaunchBomb();
    void AddBombs(int count);

    glm::vec3 GetLeftMuzzlePos() const;
    glm::vec3 GetRightMuzzlePos() const;
    glm::vec3 GetNosePos() const;
    glm::vec3 GetNearTargetPos() const;
    glm::vec3 GetFarTargetPos() const;
    bool IsDeflecting() const { return isSpinning; }
};

#endif
