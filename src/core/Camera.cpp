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
      followOffset(0.0f, 3.2f, 9.5f),
      followDamping(12.0f),
      rollTiltDamping(8.0f),
      currentRollTilt(0.0f),
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

void Camera::Follow(const glm::vec3& playerPos, float playerRoll, float dt) {
    // Desired camera position follows behind and slightly above the player
    glm::vec3 desiredPos = playerPos + followOffset;

    // Smooth interpolation (spring follow)
    float t = 1.0f - std::exp(-followDamping * dt);
    position = glm::mix(position, desiredPos, t);

    // Target is slightly ahead of the player in the flight direction (-Z)
    glm::vec3 desiredTarget = playerPos + glm::vec3(0.0f, 0.5f, -15.0f);
    target = glm::mix(target, desiredTarget, t);

    // Dynamic camera banking (subtle tilt with player banking, clamped to max 4 degrees)
    float targetTiltDeg = std::clamp(-playerRoll * 0.06f, -4.0f, 4.0f);
    float targetTiltRad = glm::radians(targetTiltDeg);
    float rollT = 1.0f - std::exp(-rollTiltDamping * dt);
    currentRollTilt = glm::mix(currentRollTilt, targetTiltRad, rollT);

    up = glm::vec3(std::sin(currentRollTilt), std::cos(currentRollTilt), 0.0f);
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
