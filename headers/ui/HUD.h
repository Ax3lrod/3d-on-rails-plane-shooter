#ifndef HUD_H
#define HUD_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <string>
#include "Mesh.h"
#include "shaderClass.h"

struct HighScoreEntry {
    char name[16];
    int score;
    int stageReached;
    bool missionComplete;
};

class HUD {
public:
    Mesh quadMesh;

    float hudScale;

    HUD();

    void SetScale(float scale);
    float GetScale() const { return hudScale; }
    float GetVirtualWidth(int screenWidth, int screenHeight) const;
    float GetVirtualHeight(int screenWidth, int screenHeight) const;

    void DrawRect(const Shader& shader, float x, float y, float w, float h,
                  const glm::vec3& color, float alpha = 1.0f) const;
    void DrawRectOutline(const Shader& shader, float x, float y, float w, float h, float thickness,
                         const glm::vec3& color, float alpha = 1.0f) const;
    void DrawSlantedRect(const Shader& shader, float x, float y, float w, float h, float slantOffset,
                         const glm::vec3& color, float alpha = 1.0f) const;
    void DrawSlantedRectOutline(const Shader& shader, float x, float y, float w, float h, float slantOffset,
                                float thickness, const glm::vec3& color, float alpha = 1.0f) const;
    void DrawSegmentedBar(const Shader& shader, float x, float y, float w, float h,
                          int totalSegments, float fillRatio,
                          const glm::vec3& fillColor, const glm::vec3& emptyColor,
                          const glm::vec3& borderColor) const;
    void DrawSlantedSegmentedBar(const Shader& shader, float x, float y, float w, float h,
                                 float slantOffset, int totalSegments, float fillRatio,
                                 const glm::vec3& fillColor, const glm::vec3& emptyColor,
                                 const glm::vec3& borderColor) const;
    void DrawDiamond(const Shader& shader, float cx, float cy, float radius,
                     const glm::vec3& color, float alpha = 1.0f) const;
    void DrawSlantedGradientBar(const Shader& shader, float x, float y, float w, float h,
                                float slantOffset, float fillRatio,
                                const glm::vec3& colLeft, const glm::vec3& colMid, const glm::vec3& colRight,
                                const glm::vec3& emptyColor = glm::vec3(0.08f, 0.09f, 0.13f),
                                const glm::vec3& borderColor = glm::vec3(1.0f, 1.0f, 1.0f),
                                float borderThickness = 2.5f) const;
    void DrawRadialGradientDiamond(const Shader& shader, float cx, float cy, float radius,
                                   const glm::vec3& innerColor, const glm::vec3& outerColor,
                                   const glm::vec3& borderColor = glm::vec3(0.08f, 0.06f, 0.16f),
                                   float alpha = 1.0f, bool filled = true) const;
    void DrawStarfighterIcon(const Shader& shader, float cx, float cy, float size) const;
    void DrawHitCombo(const Shader& shader, float cx, float cy, int comboHits,
                      float comboTimer, float maxTimer, float animScale) const;
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

    void DrawTitleScreen(const Shader& shader, int screenWidth, int screenHeight,
                         float time, int selectedMenu) const;
    void DrawSettingsMenu(const Shader& shader, int screenWidth, int screenHeight,
                          int selectedIndex, float masterVol, float musicVol, float sfxVol,
                          float currentHudScale,
                          bool invertY, bool crtFilter, bool cockpitDefault) const;
    void DrawLeaderboard(const Shader& shader, int screenWidth, int screenHeight,
                         const std::vector<HighScoreEntry>& scores) const;
    void DrawMissionBriefing(const Shader& shader, int screenWidth, int screenHeight, float time) const;
    void DrawSecretRelaysHUD(const Shader& shader, int screenWidth, int screenHeight,
                            int destroyedCount, int totalCount) const;
    void DrawWarpHUD(const Shader& shader, int screenWidth, int screenHeight,
                     float warpTimer, bool hardRoute) const;

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
                float commsTimer = 0.0f,
                int destroyedRelays = 0, int totalRelays = 3,
                int comboHits = 0, float comboTimer = 0.0f,
                float comboMaxDuration = 2.6f, float comboAnimScale = 1.0f,
                int lives = 3) const;
};

#endif
