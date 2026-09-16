#ifndef CAMERA_H
#define CAMERA_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Camera {
public:
    glm::vec3 position;
    glm::vec3 target;
    glm::vec3 up;

    float fov;
    float baseFov;
    float targetFov;
    float aspectRatio;
    float nearPlane;
    float farPlane;

    // Camera offset behind and above player starfighter
    glm::vec3 followOffset;
    float followDamping;
    float rollTiltDamping;
    float currentRollTilt;

    // Screen shake
    float shakeTimer;
    float shakeDuration;
    float shakeIntensity;
    glm::vec3 shakeOffset;

    Camera(float fovDeg = 60.0f, float aspect = 1.0f, float nearP = 0.1f, float farP = 800.0f);

    void SetAspectRatio(float aspect);
    void SetTargetFOV(float newFov);
    void TriggerShake(float intensity, float duration);

    void Follow(const glm::vec3& playerPos, float playerRoll, float dt);
    void Update(float dt);

    glm::mat4 GetViewMatrix() const;
    glm::mat4 GetProjectionMatrix() const;
};

#endif
