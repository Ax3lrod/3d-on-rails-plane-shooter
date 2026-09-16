#include "Camera.h"
#include <algorithm>
#include <cmath>
#include <cstdlib>

Camera::Camera(float fovDeg, float aspect, float nearP, float farP)
    : position(0.0f, 4.0f, 12.0f),
      target(0.0f, 0.0f, 0.0f),
      up(0.0f, 1.0f, 0.0f),
      fov(fovDeg),
      baseFov(fovDeg),
      targetFov(fovDeg),
      aspectRatio(aspect),
      nearPlane(nearP),
      farPlane(farP),
      viewMode(CameraViewMode::ThirdPerson),
      cinematicMode(CinematicMode::None),
      cinematicTimer(0.0f),
      cinematicDuration(0.0f),
      cinematicFocusPos(0.0f),
      cinematicOrbitAngle(0.0f),
      followOffset(0.0f, 2.7f, 9.2f),
      cockpitOffset(0.0f, 0.35f, -0.2f),
      followDamping(9.5f),
      rollTiltDamping(7.5f),
      currentRollTilt(0.0f),
      lateralTrackingRatio(0.45f),
      verticalTrackingRatio(0.38f),
      maxRollTiltDeg(11.5f),
      shakeTimer(0.0f),
      shakeDuration(0.0f),
      shakeIntensity(0.0f),
      shakeOffset(0.0f) {}

void Camera::SetAspectRatio(float aspect) {
    aspectRatio = aspect;
}

void Camera::SetTargetFOV(float newFov) {
    targetFov = newFov;
}

void Camera::TriggerShake(float intensity, float duration) {
    shakeIntensity = intensity;
    shakeDuration = duration;
    shakeTimer = duration;
}

void Camera::SetViewMode(CameraViewMode mode) {
    viewMode = mode;
    if (viewMode == CameraViewMode::CockpitFirstPerson) {
        followDamping = 24.0f; // Snappy inside cockpit
    } else {
        followDamping = 14.0f;
    }
}

void Camera::ToggleViewMode() {
    if (viewMode == CameraViewMode::ThirdPerson) {
        SetViewMode(CameraViewMode::CockpitFirstPerson);
    } else {
        SetViewMode(CameraViewMode::ThirdPerson);
    }
}

void Camera::StartBossIntro(const glm::vec3& bossPos, const glm::vec3& /*playerPos*/) {
    cinematicMode = CinematicMode::BossIntro;
    cinematicTimer = 0.0f;
    cinematicDuration = 4.2f;
    cinematicFocusPos = bossPos;
    cinematicOrbitAngle = 0.0f;
    SetTargetFOV(72.0f);
}

void Camera::StartBossDeathSlowMo(const glm::vec3& bossPos) {
    cinematicMode = CinematicMode::BossDeathSlowMo;
    cinematicTimer = 0.0f;
    cinematicDuration = 3.6f;
    cinematicFocusPos = bossPos;
    cinematicOrbitAngle = 20.0f;
    SetTargetFOV(68.0f);
}

void Camera::StopCinematic() {
    cinematicMode = CinematicMode::None;
    SetTargetFOV(baseFov);
}

void Camera::Follow(const glm::vec3& playerPos, float playerPitch, float playerYaw, float playerRoll,
                    float dt, bool isAllRange, bool isBoost, bool isBrake) {
    if (cinematicMode == CinematicMode::BossIntro) {
        // Dramatic Boss Intro Camera Sweep
        cinematicTimer += dt;
        float progress = std::clamp(cinematicTimer / cinematicDuration, 0.0f, 1.0f);

        if (progress < 0.65f) {
            // Stage 1: Sweep across dreadnought nose & cannons
            float sweepProg = progress / 0.65f;
            float camX = glm::mix(35.0f, -30.0f, sweepProg);
            float camY = glm::mix(18.0f, -6.0f, sweepProg);
            float camZ = cinematicFocusPos.z + glm::mix(45.0f, 25.0f, sweepProg);

            position = glm::vec3(camX, camY, camZ);
            target = cinematicFocusPos + glm::vec3(0.0f, 2.0f, -5.0f);
            up = glm::vec3(0.0f, 1.0f, 0.0f);
        } else {
            // Stage 2: Zoom behind player starfighter into battle formation
            float returnProg = (progress - 0.65f) / 0.35f;
            float smoothT = returnProg * returnProg * (3.0f - 2.0f * returnProg);

            glm::vec3 introEndPos = glm::vec3(-30.0f, -6.0f, cinematicFocusPos.z + 25.0f);
            glm::vec3 desiredPos = playerPos + followOffset;

            position = glm::mix(introEndPos, desiredPos, smoothT);
            target = glm::mix(cinematicFocusPos, playerPos + glm::vec3(0.0f, 0.5f, -20.0f), smoothT);
            up = glm::vec3(0.0f, 1.0f, 0.0f);
        }

        if (cinematicTimer >= cinematicDuration) {
            StopCinematic();
        }
        return;
    }

    if (cinematicMode == CinematicMode::BossDeathSlowMo) {
        // Orbiting Kill-Cam around collapsing dreadnought
        cinematicTimer += dt;
        cinematicOrbitAngle += 42.0f * dt;

        float rad = glm::radians(cinematicOrbitAngle);
        float radius = 55.0f;
        position = cinematicFocusPos + glm::vec3(std::sin(rad) * radius, 16.0f + std::sin(rad * 0.5f) * 6.0f, std::cos(rad) * radius);
        target = cinematicFocusPos;
        up = glm::vec3(0.0f, 1.0f, 0.0f);

        if (cinematicTimer >= cinematicDuration) {
            StopCinematic();
        }
        return;
    }

    // Normal Gameplay Follow
    if (viewMode == CameraViewMode::CockpitFirstPerson) {
        // Cockpit First-Person View
        glm::mat4 playerRot = glm::mat4(1.0f);
        playerRot = glm::rotate(playerRot, glm::radians(playerYaw), glm::vec3(0.0f, 1.0f, 0.0f));
        playerRot = glm::rotate(playerRot, glm::radians(playerPitch), glm::vec3(1.0f, 0.0f, 0.0f));
        playerRot = glm::rotate(playerRot, glm::radians(playerRoll), glm::vec3(0.0f, 0.0f, 1.0f));

        glm::vec3 rotatedCockpitOffset = glm::vec3(playerRot * glm::vec4(cockpitOffset, 1.0f));
        glm::vec3 desiredPos = playerPos + rotatedCockpitOffset;

        float t = 1.0f - std::exp(-followDamping * dt);
        position = glm::mix(position, desiredPos, t);

        // Forward look direction coupled with ship rotation
        glm::vec3 forwardDir = glm::vec3(playerRot * glm::vec4(0.0f, 0.0f, -30.0f, 0.0f));
        target = position + forwardDir;

        // Cockpit up vector rolls with the starfighter
        glm::vec3 localUp = glm::vec3(playerRot * glm::vec4(0.0f, 1.0f, 0.0f, 0.0f));
        up = localUp;
    } else {
        // Third-Person Chase Cam (Ex-Zodiac Decoupled Frustum & Camera Spring Lag)
        float dynamicDist = followOffset.z;
        if (isBoost) dynamicDist += 1.4f;
        else if (isBrake) dynamicDist -= 1.4f;

        glm::vec3 desiredPos;
        glm::vec3 desiredTarget;

        glm::mat4 yawMat = glm::rotate(glm::mat4(1.0f), glm::radians(playerYaw), glm::vec3(0.0f, 1.0f, 0.0f));

        if (isAllRange) {
            glm::vec3 back = glm::vec3(yawMat * glm::vec4(0.0f, 0.0f, 1.0f, 0.0f));
            desiredPos = playerPos + back * dynamicDist + glm::vec3(0.0f, followOffset.y, 0.0f);
            desiredTarget = playerPos + glm::vec3(0.0f, 0.6f + playerPitch * 0.12f, 0.0f) - back * 18.0f;
        } else {
            // Rail Corridor mode:
            // Camera tracks ~45% of lateral displacement and ~38% of vertical displacement
            float camX = playerPos.x * lateralTrackingRatio;
            float camY = followOffset.y + (playerPos.y * verticalTrackingRatio);
            desiredPos = glm::vec3(camX, camY, playerPos.z + dynamicDist);

            // Camera looks ahead toward targeting reticle with pitch horizon look-ahead
            float lookX = playerPos.x * 0.72f;
            float lookY = playerPos.y * 0.62f + 0.35f + (playerPitch * 0.14f);
            desiredTarget = glm::vec3(lookX, lookY, playerPos.z - 30.0f);
        }

        float t = 1.0f - std::exp(-followDamping * dt);
        position = glm::mix(position, desiredPos, t);
        target = glm::mix(target, desiredTarget, t);

        // Dynamic camera banking: Ex-Zodiac 11.5 degree max roll with spring lag
        float targetTiltDeg = std::clamp(-playerRoll * 0.18f, -maxRollTiltDeg, maxRollTiltDeg);
        float targetTiltRad = glm::radians(targetTiltDeg);
        float rollT = 1.0f - std::exp(-rollTiltDamping * dt);
        currentRollTilt = glm::mix(currentRollTilt, targetTiltRad, rollT);

        glm::mat4 rollMat = glm::rotate(glm::mat4(1.0f), currentRollTilt, glm::vec3(0.0f, 0.0f, 1.0f));
        up = glm::vec3(yawMat * rollMat * glm::vec4(0.0f, 1.0f, 0.0f, 0.0f));
    }
}

void Camera::Update(float dt) {
    // Smoothly interpolate FOV (e.g. boost effect)
    fov = glm::mix(fov, targetFov, 1.0f - std::exp(-8.0f * dt));

    // Update screen shake
    if (shakeTimer > 0.0f) {
        shakeTimer -= dt;
        float progress = shakeTimer / shakeDuration; // 1.0 -> 0.0
        float currentMag = shakeIntensity * progress;
        float rx = ((float)rand() / RAND_MAX * 2.0f - 1.0f) * currentMag;
        float ry = ((float)rand() / RAND_MAX * 2.0f - 1.0f) * currentMag;
        float rz = ((float)rand() / RAND_MAX * 2.0f - 1.0f) * currentMag;
        shakeOffset = glm::vec3(rx, ry, rz);
    } else {
        shakeOffset = glm::vec3(0.0f);
    }
}

glm::mat4 Camera::GetViewMatrix() const {
    return glm::lookAt(position + shakeOffset, target + shakeOffset, up);
}

glm::mat4 Camera::GetProjectionMatrix() const {
    return glm::perspective(glm::radians(fov), aspectRatio, nearPlane, farPlane);
}
