#include "HUD.h"
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <algorithm>
#include <cmath>
#include <GLFW/glfw3.h>

static Mesh CreateScreenQuad(const glm::vec3& color) {
    std::vector<Vertex> verts;
    std::vector<GLuint> inds;
    glm::vec3 n(0, 0, 1);

    verts.push_back({{0.0f, 0.0f, 0.0f}, n, color});
    verts.push_back({{1.0f, 0.0f, 0.0f}, n, color});
    verts.push_back({{1.0f, 1.0f, 0.0f}, n, color});
    verts.push_back({{0.0f, 1.0f, 0.0f}, n, color});

    inds.push_back(0); inds.push_back(1); inds.push_back(2);
    inds.push_back(0); inds.push_back(2); inds.push_back(3);

    return Mesh(verts, inds);
}

HUD::HUD()
    : quadMesh(CreateScreenQuad(glm::vec3(1.0f))),
      barBgMesh(CreateScreenQuad(glm::vec3(0.08f, 0.1f, 0.14f))),
      bombIconMesh(CreateScreenQuad(glm::vec3(0.2f, 0.85f, 1.0f))) {}

void HUD::DrawBar(const Shader& shader, float x, float y, float w, float h, float fillRatio,
                  const glm::vec3& fillColor, const glm::vec3& bgColor) const {
    // Background box
    glm::mat4 modelBg = glm::mat4(1.0f);
    modelBg = glm::translate(modelBg, glm::vec3(x - 2.0f, y - 2.0f, 0.0f));
    modelBg = glm::scale(modelBg, glm::vec3(w + 4.0f, h + 4.0f, 1.0f));
    shader.SetMat4("uModel", modelBg);
    shader.SetVec3("uAmbientColor", bgColor);
    barBgMesh.Draw(shader);

    // Foreground filled bar
    if (fillRatio > 0.001f) {
        glm::mat4 modelFill = glm::mat4(1.0f);
        modelFill = glm::translate(modelFill, glm::vec3(x, y, 0.0f));
        modelFill = glm::scale(modelFill, glm::vec3(w * fillRatio, h, 1.0f));
        shader.SetMat4("uModel", modelFill);
        shader.SetVec3("uAmbientColor", fillColor);
        quadMesh.Draw(shader);
    }
}

void HUD::Render(const Shader& shader, int screenWidth, int screenHeight,
                 float shield, float maxShield,
                 float boost, float maxBoost, bool isOverheated,
                 bool isDeflecting, int score, int rings,
                 int bombCount, float chargeProgress,
                 bool bossActive, bool bossWarning, float bossHealthRatio,
                 bool bossLeftTurretDown, bool bossRightTurretDown,
                 bool bossShieldDown, bool bossCoreExposed,
                 bool isVictory, bool isGameOver) const {
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    shader.Activate();
    glm::mat4 ortho = glm::ortho(0.0f, static_cast<float>(screenWidth),
                                 static_cast<float>(screenHeight), 0.0f, -1.0f, 1.0f);
    shader.SetMat4("uProjection", ortho);
    shader.SetMat4("uView", glm::mat4(1.0f));
    shader.SetInt("uUseLighting", 0);
    shader.SetFloat("uAlpha", 0.9f);

    float curTime = static_cast<float>(glfwGetTime());

    // 1. Player Shield Bar (Top Left)
    float shieldRatio = std::clamp(shield / maxShield, 0.0f, 1.0f);
    glm::vec3 shieldColor(0.2f, 0.9f, 0.3f);
    if (shieldRatio < 0.3f) shieldColor = glm::vec3(1.0f, 0.2f, 0.2f);
    else if (shieldRatio < 0.6f) shieldColor = glm::vec3(1.0f, 0.85f, 0.15f);

    DrawBar(shader, 40.0f, 40.0f, 220.0f, 18.0f, shieldRatio,
            shieldColor, glm::vec3(0.12f, 0.12f, 0.15f));

    // 2. Boost / Thruster Meter (Below Shield)
    float boostRatio = std::clamp(boost / maxBoost, 0.0f, 1.0f);
    glm::vec3 boostColor = isOverheated ? glm::vec3(1.0f, 0.35f, 0.1f) : glm::vec3(0.2f, 0.8f, 1.0f);

    DrawBar(shader, 40.0f, 66.0f, 160.0f, 12.0f, boostRatio,
            boostColor, glm::vec3(0.08f, 0.12f, 0.18f));

    // 3. Charge Shot Meter (Appears when charging)
    if (chargeProgress > 0.05f) {
        glm::vec3 chargeColor = (chargeProgress >= 0.75f) ? glm::vec3(0.2f, 1.0f, 0.75f) : glm::vec3(0.3f, 0.8f, 0.4f);
        DrawBar(shader, 40.0f, 84.0f, 160.0f, 8.0f, chargeProgress,
                chargeColor, glm::vec3(0.08f, 0.12f, 0.15f));
    }

    // 4. Smart Bomb Ammo Icons (Bottom Left)
    float bombStartX = 40.0f;
    float bombY = 100.0f;
    for (int i = 0; i < bombCount; ++i) {
        glm::mat4 modelBomb = glm::mat4(1.0f);
        modelBomb = glm::translate(modelBomb, glm::vec3(bombStartX + i * 22.0f, bombY, 0.0f));
        modelBomb = glm::scale(modelBomb, glm::vec3(14.0f, 14.0f, 1.0f));
        shader.SetMat4("uModel", modelBomb);
        shader.SetVec3("uAmbientColor", glm::vec3(1.0f, 0.85f, 0.2f));
        bombIconMesh.Draw(shader);
    }

    // 5. Barrel Roll / Deflect Status Icon
    if (isDeflecting) {
        glm::mat4 modelDeflect = glm::mat4(1.0f);
        modelDeflect = glm::translate(modelDeflect, glm::vec3(40.0f, 122.0f, 0.0f));
        modelDeflect = glm::scale(modelDeflect, glm::vec3(130.0f, 14.0f, 1.0f));
        shader.SetMat4("uModel", modelDeflect);
        shader.SetVec3("uAmbientColor", glm::vec3(0.2f, 1.0f, 0.8f));
        quadMesh.Draw(shader);
    }

    // 6. BOSS WARNING BANNER (Center Screen Flashing)
    if (bossWarning) {
        float warnW = 540.0f;
        float warnH = 70.0f;
        float wx = (screenWidth - warnW) * 0.5f;
        float wy = screenHeight * 0.32f;

        float flashAlpha = 0.55f + 0.35f * std::sin(curTime * 14.0f);

        // Backdrop
        glm::mat4 mWarnBg = glm::mat4(1.0f);
        mWarnBg = glm::translate(mWarnBg, glm::vec3(wx, wy, 0.0f));
        mWarnBg = glm::scale(mWarnBg, glm::vec3(warnW, warnH, 1.0f));
        shader.SetMat4("uModel", mWarnBg);
        shader.SetFloat("uAlpha", flashAlpha);
        shader.SetVec3("uAmbientColor", glm::vec3(0.75f, 0.08f, 0.08f));
        quadMesh.Draw(shader);

        // Warning Border Strips
        glm::mat4 mStripTop = glm::mat4(1.0f);
        mStripTop = glm::translate(mStripTop, glm::vec3(wx, wy - 6.0f, 0.0f));
        mStripTop = glm::scale(mStripTop, glm::vec3(warnW, 4.0f, 1.0f));
        shader.SetMat4("uModel", mStripTop);
        shader.SetVec3("uAmbientColor", glm::vec3(1.0f, 0.85f, 0.1f));
        quadMesh.Draw(shader);

        glm::mat4 mStripBot = glm::mat4(1.0f);
        mStripBot = glm::translate(mStripBot, glm::vec3(wx, wy + warnH + 2.0f, 0.0f));
        mStripBot = glm::scale(mStripBot, glm::vec3(warnW, 4.0f, 1.0f));
        shader.SetMat4("uModel", mStripBot);
        shader.SetVec3("uAmbientColor", glm::vec3(1.0f, 0.85f, 0.1f));
        quadMesh.Draw(shader);
    }

    // 7. BOSS HEALTH BAR & SUBSYSTEM STATUS (Top Center)
    if (bossActive) {
        float bBarW = 440.0f;
        float bBarH = 16.0f;
        float bx = (screenWidth - bBarW) * 0.5f;
        float by = 40.0f;

        // Title Plate Frame (GOLIATH DREADNOUGHT)
        glm::mat4 mPlate = glm::mat4(1.0f);
        mPlate = glm::translate(mPlate, glm::vec3(bx - 12.0f, by - 16.0f, 0.0f));
        mPlate = glm::scale(mPlate, glm::vec3(bBarW + 24.0f, 10.0f, 1.0f));
        shader.SetMat4("uModel", mPlate);
        shader.SetFloat("uAlpha", 0.9f);
        shader.SetVec3("uAmbientColor", glm::vec3(0.15f, 0.18f, 0.24f));
        quadMesh.Draw(shader);

        // Dreadnought Health Bar
        glm::vec3 bossColor(1.0f, 0.35f, 0.15f);
        if (bossCoreExposed) {
            bossColor = glm::vec3(1.0f, 0.1f, 0.15f); // Blazing red during core overdrive
        } else if (bossHealthRatio > 0.5f) {
            bossColor = glm::vec3(0.95f, 0.7f, 0.2f);
        }

        DrawBar(shader, bx, by, bBarW, bBarH, bossHealthRatio,
                bossColor, glm::vec3(0.08f, 0.09f, 0.12f));

        // Subsystem Status Pips: [L.TURRET] [SHIELD] [R.TURRET] [CORE]
        float pipW = 75.0f;
        float pipH = 8.0f;
        float pipGap = 12.0f;
        float totalPipsW = (pipW * 4.0f) + (pipGap * 3.0f);
        float pipsStartX = (screenWidth - totalPipsW) * 0.5f;
        float pipsY = by + bBarH + 8.0f;

        // Pip 1: Port Turret
        glm::vec3 lTurretCol = bossLeftTurretDown ? glm::vec3(0.35f, 0.1f, 0.1f) : glm::vec3(0.2f, 0.9f, 0.4f);
        DrawBar(shader, pipsStartX, pipsY, pipW, pipH, 1.0f, lTurretCol, glm::vec3(0.05f, 0.05f, 0.07f));

        // Pip 2: Shield Generator
        glm::vec3 shieldCol = bossShieldDown ? glm::vec3(0.35f, 0.1f, 0.1f) : glm::vec3(0.2f, 0.75f, 1.0f);
        DrawBar(shader, pipsStartX + (pipW + pipGap), pipsY, pipW, pipH, 1.0f, shieldCol, glm::vec3(0.05f, 0.05f, 0.07f));

        // Pip 3: Starboard Turret
        glm::vec3 rTurretCol = bossRightTurretDown ? glm::vec3(0.35f, 0.1f, 0.1f) : glm::vec3(0.2f, 0.9f, 0.4f);
        DrawBar(shader, pipsStartX + (pipW + pipGap) * 2.0f, pipsY, pipW, pipH, 1.0f, rTurretCol, glm::vec3(0.05f, 0.05f, 0.07f));

        // Pip 4: Quantum Core
        glm::vec3 coreCol = bossCoreExposed ? glm::vec3(1.0f, 0.4f, 0.1f) : glm::vec3(0.25f, 0.28f, 0.35f);
        DrawBar(shader, pipsStartX + (pipW + pipGap) * 3.0f, pipsY, pipW, pipH, 1.0f, coreCol, glm::vec3(0.05f, 0.05f, 0.07f));
    }

    // 8. VICTORY SCREEN (MISSION COMPLETE)
    if (isVictory) {
        float vicW = 560.0f;
        float vicH = 130.0f;
        float vx = (screenWidth - vicW) * 0.5f;
        float vy = (screenHeight - vicH) * 0.5f;

        // Radiant backdrop
        glm::mat4 mVic = glm::mat4(1.0f);
        mVic = glm::translate(mVic, glm::vec3(vx, vy, 0.0f));
        mVic = glm::scale(mVic, glm::vec3(vicW, vicH, 1.0f));
        shader.SetMat4("uModel", mVic);
        shader.SetFloat("uAlpha", 0.85f);
        shader.SetVec3("uAmbientColor", glm::vec3(0.08f, 0.22f, 0.38f));
        quadMesh.Draw(shader);

        // Gold Trim Bars
        glm::mat4 mTrimTop = glm::mat4(1.0f);
        mTrimTop = glm::translate(mTrimTop, glm::vec3(vx, vy - 4.0f, 0.0f));
        mTrimTop = glm::scale(mTrimTop, glm::vec3(vicW, 6.0f, 1.0f));
        shader.SetMat4("uModel", mTrimTop);
        shader.SetVec3("uAmbientColor", glm::vec3(1.0f, 0.85f, 0.2f));
        quadMesh.Draw(shader);

        glm::mat4 mTrimBot = glm::mat4(1.0f);
        mTrimBot = glm::translate(mTrimBot, glm::vec3(vx, vy + vicH, 0.0f));
        mTrimBot = glm::scale(mTrimBot, glm::vec3(vicW, 6.0f, 1.0f));
        shader.SetMat4("uModel", mTrimBot);
        shader.SetVec3("uAmbientColor", glm::vec3(1.0f, 0.85f, 0.2f));
        quadMesh.Draw(shader);
    }

    // 9. GAME OVER BANNER
    if (isGameOver) {
        float bannerW = 420.0f;
        float bannerH = 110.0f;
        float bx = (screenWidth - bannerW) * 0.5f;
        float by = (screenHeight - bannerH) * 0.5f;

        glm::mat4 modelBanner = glm::mat4(1.0f);
        modelBanner = glm::translate(modelBanner, glm::vec3(bx, by, 0.0f));
        modelBanner = glm::scale(modelBanner, glm::vec3(bannerW, bannerH, 1.0f));
        shader.SetMat4("uModel", modelBanner);
        shader.SetFloat("uAlpha", 0.82f);
        shader.SetVec3("uAmbientColor", glm::vec3(0.72f, 0.12f, 0.12f));
        quadMesh.Draw(shader);
    }

    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
}
