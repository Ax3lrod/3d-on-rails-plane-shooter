#ifndef PLAYER_STARFIGHTER_H
#define PLAYER_STARFIGHTER_H

#include "Transform.h"
#include "Mesh.h"
#include "Camera.h"

#include <string>
#include <vector>

struct TumblingWing {
    glm::vec3 position;
    glm::vec3 velocity;
    glm::vec3 rotation;
    glm::vec3 rotSpeed;
    bool isLeft;
    float lifetime;
    bool active;
};

class PlayerStarfighter {
public:
    Transform transform;
    Mesh mesh;
    Mesh fuselageMesh;
    Mesh leftWingMesh;
    Mesh rightWingMesh;
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

    // All-Range Mode 360 Dogfight Flight
    bool isAllRangeMode;
    glm::vec3 arenaCenter;
    float arenaRadius;
    bool isOutOfBounds;
    float outOfBoundsTimer;
    float headingYaw;

    // Evasive Acrobatics (Somersault & U-Turn)
    bool isSomersaulting;
    float somersaultTimer;
    float somersaultDuration;
    float somersaultPitch;
    bool isUTurning;
    float uTurnTimer;
    float uTurnDuration;
    float uTurnStartYaw;

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

    // Wing Damage & Breakdown System
    float leftWingHealth;
    float rightWingHealth;
    bool leftWingLost;
    bool rightWingLost;
    std::vector<TumblingWing> tumblingWings;
    float wingAlertTimer;
    std::string wingAlertMessage;

    PlayerStarfighter();

    void Update(float dt, bool allowInput = true);
    void HandleInput(float dt, bool allowInput = true);
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

    bool DamageLeftWing(float amount);
    bool DamageRightWing(float amount);
    void RepairWings();
    glm::vec3 GetLeftWingRootWorldPos() const;
    glm::vec3 GetRightWingRootWorldPos() const;
    glm::vec3 GetLeftWingTipWorldPos() const;
    glm::vec3 GetRightWingTipWorldPos() const;
    bool HasLeftWing() const { return !leftWingLost; }
    bool HasRightWing() const { return !rightWingLost; }

    void SetAllRangeMode(bool enable, const glm::vec3& center = glm::vec3(0.0f, 0.0f, -1000.0f), float radius = 240.0f);
    bool TriggerSomersault();
    bool TriggerUTurn();
    bool IsAcrobatic() const { return isSpinning || isSomersaulting || isUTurning; }
    glm::vec3 GetForwardVector() const;

    glm::vec3 GetLeftMuzzlePos() const;
    glm::vec3 GetRightMuzzlePos() const;
    glm::vec3 GetNosePos() const;
    glm::vec3 GetNearTargetPos() const;
    glm::vec3 GetFarTargetPos() const;
    bool IsDeflecting() const { return isSpinning || isSomersaulting || isUTurning; }
};

#endif
