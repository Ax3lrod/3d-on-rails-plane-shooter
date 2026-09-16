#include "PlayerStarfighter.h"
#include "Input.h"
#include <algorithm>
#include <cmath>

PlayerStarfighter::PlayerStarfighter()
    : transform(glm::vec3(0.0f, 0.0f, 0.0f)),
      mesh(Mesh::CreateStarfighter(
          glm::vec3(0.85f, 0.88f, 0.92f), // Pearlescent sleek hull
          glm::vec3(0.12f, 0.45f, 0.85f), // High-tech cobalt blue wings
          glm::vec3(0.1f, 0.85f, 0.95f)   // Luminous cyan canopy
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
      maxBankAngle(52.0f),
      maxPitchAngle(24.0f),
      maxYawAngle(18.0f),
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
      ringsCollected(0) {}

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

void PlayerStarfighter::HandleInput(float dt) {
    float inputX = 0.0f;
    float inputY = 0.0f;

    if (Input::IsKeyDown(GLFW_KEY_A) || Input::IsKeyDown(GLFW_KEY_LEFT)) inputX -= 1.0f;
    if (Input::IsKeyDown(GLFW_KEY_D) || Input::IsKeyDown(GLFW_KEY_RIGHT)) inputX += 1.0f;
    if (Input::IsKeyDown(GLFW_KEY_W) || Input::IsKeyDown(GLFW_KEY_UP)) inputY += 1.0f;
    if (Input::IsKeyDown(GLFW_KEY_S) || Input::IsKeyDown(GLFW_KEY_DOWN)) inputY -= 1.0f;

    // Double tap barrel roll or dedicated buttons (Q/E or Z/C)
    if (Input::IsDoubleTap(GLFW_KEY_A) || Input::IsDoubleTap(GLFW_KEY_LEFT) ||
        Input::IsKeyPressed(GLFW_KEY_Q) || Input::IsKeyPressed(GLFW_KEY_Z)) {
        TriggerSpin(-1.0f);
    }
    if (Input::IsDoubleTap(GLFW_KEY_D) || Input::IsDoubleTap(GLFW_KEY_RIGHT) ||
        Input::IsKeyPressed(GLFW_KEY_E) || Input::IsKeyPressed(GLFW_KEY_C)) {
        TriggerSpin(1.0f);
    }

    // Boost & Brake logic (Shift to Boost, Ctrl/Alt to Brake)
    bool wantsBoost = Input::IsKeyDown(GLFW_KEY_LEFT_SHIFT) || Input::IsKeyDown(GLFW_KEY_RIGHT_SHIFT);
    bool wantsBrake = Input::IsKeyDown(GLFW_KEY_LEFT_CONTROL) || Input::IsKeyDown(GLFW_KEY_RIGHT_CONTROL) ||
                      Input::IsKeyDown(GLFW_KEY_LEFT_ALT);

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

    // Move in local corridor (X, Y)
    transform.position.x += inputX * moveSpeedX * dt;
    transform.position.y += inputY * moveSpeedY * dt;

    // Clamp inside corridor bounds
    transform.position.x = std::clamp(transform.position.x, minX, maxX);
    transform.position.y = std::clamp(transform.position.y, minY, maxY);

    // Dynamic rotation coupling (banking when turning)
    float targetBank = -inputX * maxBankAngle;
    float targetPitch = inputY * maxPitchAngle;
    float targetYaw = -inputX * maxYawAngle;

    float bankSpeed = 12.0f;
    float pitchSpeed = 10.0f;
    currentBank = glm::mix(currentBank, targetBank, 1.0f - std::exp(-bankSpeed * dt));
    currentPitch = glm::mix(currentPitch, targetPitch, 1.0f - std::exp(-pitchSpeed * dt));
    currentYaw = glm::mix(currentYaw, targetYaw, 1.0f - std::exp(-pitchSpeed * dt));
}

void PlayerStarfighter::Update(float dt) {
    HandleInput(dt);

    // Forward motion along Z axis (into the screen, -Z)
    transform.position.z -= currentSpeed * dt;

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
            transform.position.x += spinDirection * 22.0f * dt;
            transform.position.x = std::clamp(transform.position.x, minX, maxX);
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

    // Apply rotation to transform:
    transform.rotation.x = currentPitch;
    transform.rotation.y = currentYaw;
    transform.rotation.z = currentBank + spinRoll;

    // Update timers
    if (invulnerableTimer > 0.0f) invulnerableTimer -= dt;
    if (fireTimer > 0.0f) fireTimer -= dt;
}

void PlayerStarfighter::Draw(const Shader& shader) const {
    if (invulnerableTimer > 0.0f && !isSpinning) {
        int flash = static_cast<int>(invulnerableTimer * 20.0f);
        if (flash % 2 == 0) return;
    }

    shader.SetMat4("uModel", transform.GetModelMatrix());
    shader.SetInt("uUseLighting", 1);
    shader.SetFloat("uAlpha", 1.0f);
    mesh.Draw(shader);

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
    return transform.position + glm::vec3(0.0f, 0.0f, -20.0f);
}

glm::vec3 PlayerStarfighter::GetFarTargetPos() const {
    return transform.position + glm::vec3(0.0f, 0.0f, -60.0f);
}
