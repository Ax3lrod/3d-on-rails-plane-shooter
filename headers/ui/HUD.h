#ifndef HUD_H
#define HUD_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <string>
#include "Mesh.h"
#include "shaderClass.h"

class HUD {
public:
    Mesh quadMesh;

    HUD();

    void DrawRect(const Shader& shader, float x, float y, float w, float h,
                  const glm::vec3& color, float alpha = 1.0f) const;
    void DrawRectOutline(const Shader& shader, float x, float y, float w, float h, float thickness,
                         const glm::vec3& color, float alpha = 1.0f) const;
    void DrawSegmentedBar(const Shader& shader, float x, float y, float w, float h,
                          int totalSegments, float fillRatio,
                          const glm::vec3& fillColor, const glm::vec3& emptyColor,
                          const glm::vec3& borderColor) const;
    void DrawChar(const Shader& shader, char c, float x, float y, float scale,
                  const glm::vec3& color, float alpha = 1.0f) const;
    void DrawText(const Shader& shader, const std::string& text, float x, float y, float scale,
                  const glm::vec3& color, float alpha = 1.0f) const;

    void DrawCockpitCanopyOverlay(const Shader& shader, int screenWidth, int screenHeight,
                                 float pitch, float roll) const;

    void DrawRadarMinimap(const Shader& shader, float rx, float ry, float radius,
                          const glm::vec3& playerPos, float playerYaw,
                          const glm::vec3& bossPos, bool bossActive,
                          const std::vector<glm::vec3>& enemyPositions) const;

    void DrawCommsBox(const Shader& shader, int screenWidth, int screenHeight,
                      bool hasMessage, int speakerId,
                      const std::string& callsign,
                      const std::string& line1, const std::string& line2,
                      const glm::vec3& themeColor, float timer) const;

    void Render(const Shader& shader, int screenWidth, int screenHeight,
                float shield, float maxShield,
                float boost, float maxBoost, bool isOverheated,
                bool isDeflecting, int score, int rings,
                int bombCount, float chargeProgress,
                bool bossActive, bool bossWarning, float bossHealthRatio,
                bool bossLeftTurretDown, bool bossRightTurretDown,
                bool bossShieldDown, bool bossCoreExposed,
                bool isVictory, bool isGameOver,
                float leftWingHealth = 100.0f, bool leftWingLost = false,
                float rightWingHealth = 100.0f, bool rightWingLost = false,
                float wingAlertTimer = 0.0f, const std::string& wingAlertMsg = "",
                bool isFirstPerson = false, float playerPitch = 0.0f, float playerRoll = 0.0f,
                const glm::vec3& playerPos = glm::vec3(0.0f), float playerYaw = 0.0f,
                const glm::vec3& bossPos = glm::vec3(0.0f),
                const std::vector<glm::vec3>& enemyPositions = {},
                bool hasComms = false, int commsSpeaker = 0,
                const std::string& commsCallsign = "",
                const std::string& commsLine1 = "",
                const std::string& commsLine2 = "",
                const glm::vec3& commsColor = glm::vec3(1.0f),
                float commsTimer = 0.0f) const;
};

#endif
