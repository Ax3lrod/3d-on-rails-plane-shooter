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
                float wingAlertTimer = 0.0f, const std::string& wingAlertMsg = "") const;
};

#endif
