#include "HUD.h"
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <algorithm>

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
                 bool isDeflecting, int, int,
                 int bombCount, float chargeProgress, bool isGameOver) const {
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

    // 1. Shield Bar (Top Left)
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

    // 6. Game Over Banner
    if (isGameOver) {
        float bannerW = 400.0f;
        float bannerH = 100.0f;
        float bx = (screenWidth - bannerW) * 0.5f;
        float by = (screenHeight - bannerH) * 0.5f;

        glm::mat4 modelBanner = glm::mat4(1.0f);
        modelBanner = glm::translate(modelBanner, glm::vec3(bx, by, 0.0f));
        modelBanner = glm::scale(modelBanner, glm::vec3(bannerW, bannerH, 1.0f));
        shader.SetMat4("uModel", modelBanner);
        shader.SetFloat("uAlpha", 0.75f);
        shader.SetVec3("uAmbientColor", glm::vec3(0.7f, 0.1f, 0.1f));
        quadMesh.Draw(shader);
    }

    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
}
