#ifndef CAMERA_H
#define CAMERA_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

enum class CameraViewMode {
    ThirdPerson,
    CockpitFirstPerson
};

enum class CinematicMode {
    None,
    BossIntro,
    BossDeathSlowMo
};

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

    // View Mode & Cinematic Director
    CameraViewMode viewMode;
    CinematicMode cinematicMode;
    float cinematicTimer;
    float cinematicDuration;
    glm::vec3 cinematicFocusPos;
    float cinematicOrbitAngle;

    // Camera offset behind and above player starfighter
    glm::vec3 followOffset;
    glm::vec3 cockpitOffset;
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

    void SetViewMode(CameraViewMode mode);
    void ToggleViewMode();
    void StartBossIntro(const glm::vec3& bossPos, const glm::vec3& playerPos);
    void StartBossDeathSlowMo(const glm::vec3& bossPos);
    void StopCinematic();
    bool IsInCinematic() const { return cinematicMode != CinematicMode::None; }
    bool IsFirstPerson() const { return viewMode == CameraViewMode::CockpitFirstPerson && cinematicMode == CinematicMode::None; }

    void Follow(const glm::vec3& playerPos, float playerPitch, float playerYaw, float playerRoll, float dt);
    void Update(float dt);

    glm::mat4 GetViewMatrix() const;
    glm::mat4 GetProjectionMatrix() const;
};

#endif
