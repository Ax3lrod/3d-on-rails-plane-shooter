#include "PlayerStarfighter.h"
#include "Input.h"
#include <algorithm>
#include <cmath>

PlayerStarfighter::PlayerStarfighter()
    : transform(glm::vec3(0.0f, 0.0f, 0.0f)),
      mesh(Mesh::CreateStarfighter(
          glm::vec3(0.55f, 0.16f, 0.58f), // Ex-Zodiac Signature Royal Violet / Magenta Fuselage
          glm::vec3(0.18f, 0.82f, 0.98f), // Electric Cyan Wingtips & Accent
          glm::vec3(0.98f, 0.86f, 0.25f)  // Luminous Amber/Golden Canopy Glass
      )),
      fuselageMesh(Mesh::CreateStarfighterFuselage(
          glm::vec3(0.55f, 0.16f, 0.58f),
          glm::vec3(0.98f, 0.86f, 0.25f)
      )),
      leftWingMesh(Mesh::CreateStarfighterLeftWing(
          glm::vec3(0.18f, 0.82f, 0.98f)
      )),
      rightWingMesh(Mesh::CreateStarfighterRightWing(
          glm::vec3(0.18f, 0.82f, 0.98f)
      )),
      chargeOrbMesh(Mesh::CreateSphere(0.7f, 10, 12, glm::vec3(0.2f, 1.0f, 0.6f))),
      baseSpeed(48.0f),
      boostSpeed(84.0f),
      brakeSpeed(24.0f),
      currentSpeed(48.0f),
      minX(-13.5f), maxX(13.5f),
      minY(-6.5f), maxY(6.8f),
      moveSpeedX(23.0f),
      moveSpeedY(18.0f),
      maxBankAngle(55.0f),
      maxPitchAngle(24.0f),
      maxYawAngle(18.0f),
      lateralVelocityX(0.0f),
      lateralVelocityY(0.0f),
      currentBank(0.0f),
      currentPitch(0.0f),
      currentYaw(0.0f),
      isSpinning(false),
      spinTimer(0.0f),
      spinDuration(0.38f),
      spinDirection(1.0f),
      spinRoll(0.0f),
      boostMeter(100.0f),
      maxBoost(100.0f),
      isBoosting(false),
      isBraking(false),
      isOverheated(false),
      overheatTimer(0.0f),
      invertPitch(false),
      shield(100.0f),
      maxShield(100.0f),
      invulnerableTimer(0.0f),
      fireTimer(0.0f),
      fireRate(0.11f),
      chargeTimer(0.0f),
      maxChargeTime(0.85f),
      isCharging(false),
      isFullyCharged(false),
      hasLockOn(false),
      lockTargetPos(0.0f),
      lockRotation(0.0f),
      bombCount(3),
      maxBombs(5),
      score(0),
      ringsCollected(0),
      lives(3),
      maxLives(5),
      leftWingHealth(100.0f),
      rightWingHealth(100.0f),
      leftWingLost(false),
      rightWingLost(false),
      wingAlertTimer(0.0f),
      wingAlertMessage(""),
      isAllRangeMode(false),
      arenaCenter(0.0f, 0.0f, -1000.0f),
      arenaRadius(240.0f),
      isOutOfBounds(false),
      outOfBoundsTimer(0.0f),
      headingYaw(0.0f),
      isSomersaulting(false),
      somersaultTimer(0.0f),
      somersaultDuration(1.05f),
      somersaultPitch(0.0f),
      somersaultStartY(0.0f),
      isUTurning(false),
      uTurnTimer(0.0f),
      uTurnDuration(0.95f),
      uTurnStartYaw(0.0f) {}

void PlayerStarfighter::TriggerSpin(float direction) {
    if (!isSpinning) {
        isSpinning = true;
        spinTimer = spinDuration;
        spinDirection = direction;
        spinRoll = 0.0f;
    }
}

void PlayerStarfighter::TakeDamage(float amount) {
    if (invulnerableTimer > 0.0f || isSpinning) return;

    shield = std::max(0.0f, shield - amount);
    invulnerableTimer = 0.8f;
    Input::SetRumble(0.65f, 0.75f, 0.28f);
}

void PlayerStarfighter::AddShield(float amount) {
    shield = std::min(maxShield, shield + amount);
}

void PlayerStarfighter::AddBombs(int count) {
    bombCount = std::min(maxBombs, bombCount + count);
}

bool PlayerStarfighter::CanFire() const {
    return fireTimer <= 0.0f;
}

void PlayerStarfighter::ResetFireTimer() {
    fireTimer = fireRate;
}

void PlayerStarfighter::StartCharging() {
    isCharging = true;
}

float PlayerStarfighter::GetChargeProgress() const {
    return std::clamp(chargeTimer / maxChargeTime, 0.0f, 1.0f);
}

bool PlayerStarfighter::ReleaseChargedShot() {
    bool wasFullyCharged = (chargeTimer >= maxChargeTime * 0.75f);
    chargeTimer = 0.0f;
    isCharging = false;
    isFullyCharged = false;
    return wasFullyCharged;
}

bool PlayerStarfighter::LaunchBomb() {
    if (bombCount > 0) {
        bombCount--;
        return true;
    }
    return false;
}

void PlayerStarfighter::SetAllRangeMode(bool enable, const glm::vec3& center, float radius) {
    isAllRangeMode = enable;
    arenaCenter = center;
    arenaRadius = radius;
    if (enable) {
        minX = -radius * 1.1f;
        maxX = radius * 1.1f;
        minY = -25.0f;
        maxY = 45.0f;
    } else {
        minX = -13.5f;
        maxX = 13.5f;
        minY = -6.5f;
        maxY = 6.8f;
        headingYaw = 0.0f;
    }
}

bool PlayerStarfighter::TriggerSomersault() {
    if (isSomersaulting || isUTurning || isSpinning) return false;

    isSomersaulting = true;
    somersaultTimer = 0.0f;
    somersaultDuration = 1.05f;
    somersaultPitch = 0.0f;
    somersaultStartY = transform.position.y;
    boostMeter = std::max(0.0f, boostMeter - 15.0f);
    invulnerableTimer = std::max(invulnerableTimer, 1.2f);
    return true;
}

bool PlayerStarfighter::TriggerUTurn() {
    if (!isAllRangeMode) return false; // Only in open arenas
    if (isSomersaulting || isUTurning || isSpinning) return false;

    isUTurning = true;
    uTurnTimer = 0.0f;
    uTurnDuration = 0.90f;
    uTurnStartYaw = headingYaw;
    somersaultPitch = 0.0f;
    boostMeter = std::max(0.0f, boostMeter - 10.0f);
    invulnerableTimer = std::max(invulnerableTimer, 1.2f);
    return true;
}

glm::vec3 PlayerStarfighter::GetForwardVector() const {
    return transform.GetForward();
}

bool PlayerStarfighter::DamageLeftWing(float amount) {
    if (leftWingLost) return false;

    leftWingHealth = std::max(0.0f, leftWingHealth - amount);
    if (leftWingHealth <= 0.0f) {
        leftWingLost = true;
        leftWingHealth = 0.0f;
        wingAlertTimer = 3.0f;
        wingAlertMessage = "WARNING: LEFT WING DESTROYED";
        Input::SetRumble(0.9f, 1.0f, 0.45f);

        // Spawn tumbling wing debris
        TumblingWing debris;
        debris.position = GetLeftWingRootWorldPos();
        debris.velocity = glm::vec3(-12.0f, 7.0f, -currentSpeed * 0.35f);
        debris.rotation = glm::vec3(currentPitch, currentYaw, currentBank);
        debris.rotSpeed = glm::vec3(400.0f, -220.0f, 520.0f);
        debris.isLeft = true;
        debris.lifetime = 3.5f;
        debris.active = true;
        tumblingWings.push_back(debris);

        return true; // Wing severed!
    }
    return false;
}

bool PlayerStarfighter::DamageRightWing(float amount) {
    if (rightWingLost) return false;

    rightWingHealth = std::max(0.0f, rightWingHealth - amount);
    if (rightWingHealth <= 0.0f) {
        rightWingLost = true;
        rightWingHealth = 0.0f;
        wingAlertTimer = 3.0f;
        wingAlertMessage = "WARNING: RIGHT WING DESTROYED";
        Input::SetRumble(0.9f, 1.0f, 0.45f);

        // Spawn tumbling wing debris
        TumblingWing debris;
        debris.position = GetRightWingRootWorldPos();
        debris.velocity = glm::vec3(12.0f, 7.0f, -currentSpeed * 0.35f);
        debris.rotation = glm::vec3(currentPitch, currentYaw, currentBank);
        debris.rotSpeed = glm::vec3(400.0f, 220.0f, -520.0f);
        debris.isLeft = false;
        debris.lifetime = 3.5f;
        debris.active = true;
        tumblingWings.push_back(debris);

        return true; // Wing severed!
    }
    return false;
}

void PlayerStarfighter::RepairWings() {
    leftWingLost = false;
    rightWingLost = false;
    leftWingHealth = 100.0f;
    rightWingHealth = 100.0f;
    wingAlertTimer = 2.4f;
    wingAlertMessage = "WINGS REPAIRED";
}

glm::vec3 PlayerStarfighter::GetLeftWingRootWorldPos() const {
    glm::mat4 model = transform.GetModelMatrix();
    return glm::vec3(model * glm::vec4(-0.45f, 0.05f, 0.7f, 1.0f));
}

glm::vec3 PlayerStarfighter::GetRightWingRootWorldPos() const {
    glm::mat4 model = transform.GetModelMatrix();
    return glm::vec3(model * glm::vec4(0.45f, 0.05f, 0.7f, 1.0f));
}

glm::vec3 PlayerStarfighter::GetLeftWingTipWorldPos() const {
    glm::mat4 model = transform.GetModelMatrix();
    return glm::vec3(model * glm::vec4(-2.6f, -0.05f, 0.9f, 1.0f));
}

glm::vec3 PlayerStarfighter::GetRightWingTipWorldPos() const {
    glm::mat4 model = transform.GetModelMatrix();
    return glm::vec3(model * glm::vec4(2.6f, -0.05f, 0.9f, 1.0f));
}

void PlayerStarfighter::HandleInput(float dt, bool allowInput) {
    float inputX = 0.0f;
    float inputY = 0.0f;
    bool wantsBoost = false;
    bool justBoost = false;
    bool wantsBrake = false;
    bool justBrake = false;

    if (allowInput) {
        float pitchMult = invertPitch ? -1.0f : 1.0f;
        inputX = Input::GetAxisHorizontal();
        inputY = Input::GetAxisVertical() * pitchMult;

        // Double tap barrel roll, dedicated keys (Q/E, Z/C), or Gamepad Bumpers (LB/RB)
        if (Input::IsRollLeftPressed()) {
            TriggerSpin(-1.0f);
            Input::SetRumble(0.15f, 0.35f, 0.12f);
        }
        if (Input::IsRollRightPressed()) {
            TriggerSpin(1.0f);
            Input::SetRumble(0.15f, 0.35f, 0.12f);
        }

        // Boost & Brake logic (Shift / RT to Boost, Ctrl / LT to Brake)
        wantsBoost = Input::IsBoostDown();
        justBoost = Input::IsBoostPressed();
        wantsBrake = Input::IsBrakeDown();
        justBrake = Input::IsBrakePressed();

        // Somersault: Dedicated button (X/F or Gamepad Y) OR chord (S/Down + Boost)
        bool keySomersault = Input::IsSomersaultPressed();
        bool pressDown = (inputY * pitchMult < -0.45f);
        bool justDown = pressDown && (justBoost || Input::IsMenuDownPressed());
        bool chordSomersault = (pressDown && justBoost) || (justDown && wantsBoost);

        if (keySomersault || chordSomersault) {
            if (isAllRangeMode) {
                TriggerUTurn();
            } else {
                TriggerSomersault();
            }
            Input::SetRumble(0.25f, 0.50f, 0.20f);
        } else if ((pressDown && justBrake) || (justDown && wantsBrake)) {
            TriggerUTurn();
            Input::SetRumble(0.30f, 0.40f, 0.18f);
        }
    } else {
        // Cruise smoothly towards corridor center when input is disabled (e.g. cinematic intro)
        transform.position.x = glm::mix(transform.position.x, 0.0f, 1.0f - std::exp(-5.0f * dt));
        transform.position.y = glm::mix(transform.position.y, 0.0f, 1.0f - std::exp(-5.0f * dt));
    }

    if (isOverheated) {
        overheatTimer -= dt;
        if (overheatTimer <= 0.0f) {
            isOverheated = false;
        }
        wantsBoost = false;
    }

    if (wantsBoost && boostMeter > 5.0f && !isOverheated) {
        isBoosting = true;
        isBraking = false;
        boostMeter -= 42.0f * dt;
        if (boostMeter <= 0.0f) {
            boostMeter = 0.0f;
            isOverheated = true;
            overheatTimer = 2.0f;
        }
    } else {
        isBoosting = false;
        // Recharge boost meter
        if (!wantsBoost) {
            boostMeter = std::min(maxBoost, boostMeter + 22.0f * dt);
        }
    }

    if (wantsBrake && !isBoosting) {
        isBraking = true;
    } else {
        isBraking = false;
    }

    // Smooth speed interpolation
    float targetSpeed = baseSpeed;
    if (isBoosting) targetSpeed = boostSpeed;
    else if (isBraking) targetSpeed = brakeSpeed;

    currentSpeed = glm::mix(currentSpeed, targetSpeed, 1.0f - std::exp(-8.0f * dt));

    // Ex-Zodiac lateral velocity smoothing with aerodynamic inertia
    float targetVelX = inputX * moveSpeedX;
    float targetVelY = inputY * moveSpeedY;

    // Responsive acceleration on input (20.0f), silky aerodynamic inertia on release (14.0f)
    float accelRateX = (std::abs(inputX) > 0.01f) ? 20.0f : 14.0f;
    float accelRateY = (std::abs(inputY) > 0.01f) ? 20.0f : 14.0f;
    lateralVelocityX = glm::mix(lateralVelocityX, targetVelX, 1.0f - std::exp(-accelRateX * dt));
    lateralVelocityY = glm::mix(lateralVelocityY, targetVelY, 1.0f - std::exp(-accelRateY * dt));

    if (isAllRangeMode) {
        // Free 360-degree heading yaw steering
        headingYaw -= inputX * 75.0f * dt;
        if (headingYaw > 180.0f) headingYaw -= 360.0f;
        if (headingYaw < -180.0f) headingYaw += 360.0f;

        // Pitch / Altitude with inertia
        transform.position.y += lateralVelocityY * dt;
        transform.position.y = std::clamp(transform.position.y, minY, maxY);
    } else {
        // Corridor rail movement with lateral velocity smoothing (X, Y)
        transform.position.x += lateralVelocityX * dt;
        transform.position.y += lateralVelocityY * dt;

        // Asymmetric aerodynamic drag / drift when wings are lost
        if (leftWingLost && !rightWingLost) {
            transform.position.x -= 3.2f * dt; // Drifts left
        } else if (rightWingLost && !leftWingLost) {
            transform.position.x += 3.2f * dt; // Drifts right
        }

        // Clamp inside corridor bounds
        transform.position.x = std::clamp(transform.position.x, minX, maxX);
        transform.position.y = std::clamp(transform.position.y, minY, maxY);
    }

    // Dynamic rotation coupling (Ex-Zodiac snappy bank entry & centering spring)
    float targetBank = -inputX * maxBankAngle;
    float targetPitch = inputY * maxPitchAngle;
    float targetYaw = -inputX * maxYawAngle;

    // Aerodynamic list bias when wings are severed
    if (leftWingLost && !rightWingLost) {
        targetBank += 8.5f; // Lists left
    } else if (rightWingLost && !leftWingLost) {
        targetBank -= 8.5f; // Lists right
    } else if (leftWingLost && rightWingLost) {
        targetPitch -= 4.0f; // Sinks slightly without wing surface
    }

    // Snappy roll-in when entering turns (17.5f), smooth centering spring when releasing (12.5f)
    float bankSpeed = (std::abs(inputX) > 0.05f) ? 17.5f : 12.5f;
    float pitchSpeed = (std::abs(inputY) > 0.05f) ? 14.0f : 10.5f;
    currentBank = glm::mix(currentBank, targetBank, 1.0f - std::exp(-bankSpeed * dt));
    currentPitch = glm::mix(currentPitch, targetPitch, 1.0f - std::exp(-pitchSpeed * dt));
    currentYaw = glm::mix(currentYaw, targetYaw, 1.0f - std::exp(-bankSpeed * dt));
}

void PlayerStarfighter::Update(float dt, bool allowInput) {
    if (isSomersaulting || isUTurning) {
        // While performing acrobatics, freeze steering inputs so they do not fight the loop maneuver
        boostMeter = std::min(maxBoost, boostMeter + 22.0f * dt);
        isBoosting = false;
        lateralVelocityX = 0.0f;
        lateralVelocityY = 0.0f;
    } else {
        HandleInput(dt, allowInput);
    }

    // Evasive Somersault Loop-de-loop (Star Fox 64 & Ex-Zodiac vertical loop)
    if (isSomersaulting) {
        somersaultTimer += dt;
        float t = std::clamp(somersaultTimer / somersaultDuration, 0.0f, 1.0f);
        if (t >= 1.0f) {
            isSomersaulting = false;
            somersaultPitch = 0.0f;
            currentPitch = 0.0f;
        } else {
            // Full 360 degree pitch loop
            somersaultPitch = t * 360.0f;
            currentPitch = 0.0f;

            // Vertical climbing arc: climbs smoothly up to +8.5m above starting altitude at apex
            float arc = std::sin(t * 3.14159265f);
            transform.position.y = somersaultStartY + arc * 8.5f;

            // Longitudinal braking at apex for dramatic Star Fox loop feel
            float speedMod = 1.0f - arc * 0.55f;
            if (!isAllRangeMode) {
                transform.position.z -= (currentSpeed * speedMod) * dt;
            } else {
                glm::vec3 fwd = glm::vec3(-std::sin(glm::radians(headingYaw)), 0.0f, -std::cos(glm::radians(headingYaw)));
                transform.position += fwd * (currentSpeed * speedMod * dt);
            }
        }
    }

    // Evasive U-Turn Maneuver (All-Range Mode 180-degree reversal)
    if (isUTurning) {
        uTurnTimer += dt;
        float t = uTurnTimer / uTurnDuration;
        if (t >= 1.0f) {
            isUTurning = false;
            headingYaw = uTurnStartYaw + 180.0f;
            if (headingYaw > 180.0f) headingYaw -= 360.0f;
            if (headingYaw < -180.0f) headingYaw += 360.0f;
            somersaultPitch = 0.0f;
            currentPitch = 0.0f;
        } else {
            somersaultPitch = std::sin(t * 3.14159f) * 85.0f;
            float smoothT = t * t * (3.0f - 2.0f * t);
            headingYaw = uTurnStartYaw + smoothT * 180.0f;

            // Climb altitude during reversal arc
            float arc = std::sin(t * 3.14159f);
            transform.position.y += arc * 10.0f * dt;

            // Move along reversing heading vector
            glm::vec3 fwd = glm::vec3(-std::sin(glm::radians(headingYaw)), 0.0f, -std::cos(glm::radians(headingYaw)));
            transform.position += fwd * (currentSpeed * 0.85f * dt);
        }
    }

    // Forward motion: 360-degree vector in All-Range mode, -Z in Rail mode
    if (isAllRangeMode) {
        if (!isSomersaulting && !isUTurning) {
            glm::vec3 fwd = transform.GetForward();
            transform.position += fwd * (currentSpeed * dt);
        }

        // Arena boundary check
        float dx = transform.position.x - arenaCenter.x;
        float dz = transform.position.z - arenaCenter.z;
        float distFromCenter = std::sqrt(dx * dx + dz * dz);

        if (distFromCenter > arenaRadius * 0.85f) {
            isOutOfBounds = true;
            wingAlertTimer = 0.5f;
            wingAlertMessage = "WARNING: COMBAT ZONE PERIMETER";
        } else {
            isOutOfBounds = false;
        }

        if (distFromCenter > arenaRadius) {
            if (!isUTurning) {
                TriggerUTurn();
                wingAlertTimer = 2.0f;
                wingAlertMessage = "U-TURN: RETURNING TO ARENA";
            }
        }
    } else {
        if (!isSomersaulting) {
            transform.position.z -= currentSpeed * dt;
        }
    }

    // Handle tactical barrel roll spin
    if (isSpinning) {
        spinTimer -= dt;
        float progress = 1.0f - (spinTimer / spinDuration);
        if (progress >= 1.0f) {
            isSpinning = false;
            spinRoll = 0.0f;
        } else {
            // Full 360 degree spin
            spinRoll = spinDirection * 360.0f * progress;
            // Lateral evasive dash
            float dashSpeed = (leftWingLost || rightWingLost) ? 14.0f : 22.0f;
            if (!isAllRangeMode) {
                transform.position.x += spinDirection * dashSpeed * dt;
                transform.position.x = std::clamp(transform.position.x, minX, maxX);
            }
        }
    }

    // Charging logic
    if (isCharging) {
        chargeTimer += dt;
        if (chargeTimer >= maxChargeTime) {
            isFullyCharged = true;
        }
    }

    // Lock-on bracket animation
    if (hasLockOn) {
        lockRotation += 160.0f * dt;
        if (lockRotation > 360.0f) lockRotation -= 360.0f;
    }

    // Apply combined rotations to transform:
    transform.rotation.x = currentPitch + somersaultPitch;
    transform.rotation.y = headingYaw + currentYaw;
    transform.rotation.z = currentBank + spinRoll;

    // Update timers
    if (invulnerableTimer > 0.0f) invulnerableTimer -= dt;
    if (fireTimer > 0.0f) fireTimer -= dt;
    if (wingAlertTimer > 0.0f) wingAlertTimer -= dt;

    // Update tumbling wing debris pieces
    for (auto& debris : tumblingWings) {
        if (!debris.active) continue;
        debris.position += debris.velocity * dt;
        debris.velocity.y -= 22.0f * dt; // Gravity pull into canyon
        debris.velocity.x *= (1.0f - 0.35f * dt); // Air resistance
        debris.rotation += debris.rotSpeed * dt;
        debris.lifetime -= dt;
        if (debris.lifetime <= 0.0f) {
            debris.active = false;
        }
    }
    tumblingWings.erase(
        std::remove_if(tumblingWings.begin(), tumblingWings.end(),
                       [](const TumblingWing& w) { return !w.active; }),
        tumblingWings.end()
    );
}

void PlayerStarfighter::Draw(const Shader& shader) const {
    if (invulnerableTimer > 0.0f && !isSpinning) {
        int flash = static_cast<int>(invulnerableTimer * 20.0f);
        if (flash % 2 == 0) return;
    }

    glm::mat4 shipModel = transform.GetModelMatrix();
    shader.SetMat4("uModel", shipModel);
    shader.SetInt("uUseLighting", 1);
    shader.SetFloat("uAlpha", 1.0f);

    // 1. Draw central fuselage (always present)
    fuselageMesh.Draw(shader);

    // 2. Draw left wing if intact
    if (!leftWingLost) {
        leftWingMesh.Draw(shader);
    }

    // 3. Draw right wing if intact
    if (!rightWingLost) {
        rightWingMesh.Draw(shader);
    }

    // 4. Draw tumbling wing debris pieces
    for (const auto& debris : tumblingWings) {
        if (!debris.active) continue;

        glm::mat4 debrisModel = glm::mat4(1.0f);
        debrisModel = glm::translate(debrisModel, debris.position);
        debrisModel = glm::rotate(debrisModel, glm::radians(debris.rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
        debrisModel = glm::rotate(debrisModel, glm::radians(debris.rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
        debrisModel = glm::rotate(debrisModel, glm::radians(debris.rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

        shader.SetMat4("uModel", debrisModel);
        if (debris.isLeft) {
            leftWingMesh.Draw(shader);
        } else {
            rightWingMesh.Draw(shader);
        }
    }

    // Reset model matrix back to ship
    shader.SetMat4("uModel", shipModel);

    // Draw glowing charge orb at nose when charging
    if (isCharging && chargeTimer > 0.12f) {
        shader.SetInt("uUseLighting", 0);
        float progress = GetChargeProgress();
        float scale = progress * 1.3f;
        if (isFullyCharged) {
            scale *= (1.0f + 0.12f * std::sin(chargeTimer * 22.0f));
        }

        glm::mat4 orbModel = glm::mat4(1.0f);
        orbModel = glm::translate(orbModel, GetNosePos());
        orbModel = glm::scale(orbModel, glm::vec3(scale));

        shader.SetMat4("uModel", orbModel);
        shader.SetFloat("uAlpha", 0.9f);
        chargeOrbMesh.Draw(shader);
    }
}

glm::vec3 PlayerStarfighter::GetLeftMuzzlePos() const {
    glm::mat4 model = transform.GetModelMatrix();
    return glm::vec3(model * glm::vec4(-1.2f, -0.1f, -0.7f, 1.0f));
}

glm::vec3 PlayerStarfighter::GetRightMuzzlePos() const {
    glm::mat4 model = transform.GetModelMatrix();
    return glm::vec3(model * glm::vec4(1.2f, -0.1f, -0.7f, 1.0f));
}

glm::vec3 PlayerStarfighter::GetNosePos() const {
    glm::mat4 model = transform.GetModelMatrix();
    return glm::vec3(model * glm::vec4(0.0f, 0.05f, -2.6f, 1.0f));
}

glm::vec3 PlayerStarfighter::GetNearTargetPos() const {
    return transform.position + GetForwardVector() * 20.0f;
}

glm::vec3 PlayerStarfighter::GetFarTargetPos() const {
    return transform.position + GetForwardVector() * 60.0f;
}

