#include "HUD.h"
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <GLFW/glfw3.h>

static Mesh CreateScreenQuad() {
    std::vector<Vertex> verts;
    std::vector<GLuint> inds;
    glm::vec3 n(0, 0, 1);
    glm::vec3 c(1.0f);

    verts.push_back({{0.0f, 0.0f, 0.0f}, n, c});
    verts.push_back({{1.0f, 0.0f, 0.0f}, n, c});
    verts.push_back({{1.0f, 1.0f, 0.0f}, n, c});
    verts.push_back({{0.0f, 1.0f, 0.0f}, n, c});

    inds.push_back(0); inds.push_back(1); inds.push_back(2);
    inds.push_back(0); inds.push_back(2); inds.push_back(3);

    return Mesh(verts, inds);
}

// 5x7 Font Lookup Table
static const uint8_t* GetGlyph(char c) {
    if (c >= 'a' && c <= 'z') c = static_cast<char>(c - 'a' + 'A');

    static const uint8_t GLYPH_BLANK[7] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    static const uint8_t GLYPH_0[7]     = {0x0E, 0x11, 0x13, 0x15, 0x19, 0x11, 0x0E};
    static const uint8_t GLYPH_1[7]     = {0x04, 0x0C, 0x04, 0x04, 0x04, 0x04, 0x0E};
    static const uint8_t GLYPH_2[7]     = {0x0E, 0x11, 0x01, 0x06, 0x08, 0x10, 0x1F};
    static const uint8_t GLYPH_3[7]     = {0x1E, 0x01, 0x01, 0x0E, 0x01, 0x01, 0x1E};
    static const uint8_t GLYPH_4[7]     = {0x02, 0x06, 0x0A, 0x12, 0x1F, 0x02, 0x02};
    static const uint8_t GLYPH_5[7]     = {0x1F, 0x10, 0x1E, 0x01, 0x01, 0x11, 0x0E};
    static const uint8_t GLYPH_6[7]     = {0x06, 0x08, 0x10, 0x1E, 0x11, 0x11, 0x0E};
    static const uint8_t GLYPH_7[7]     = {0x1F, 0x01, 0x02, 0x04, 0x08, 0x08, 0x08};
    static const uint8_t GLYPH_8[7]     = {0x0E, 0x11, 0x11, 0x0E, 0x11, 0x11, 0x0E};
    static const uint8_t GLYPH_9[7]     = {0x0E, 0x11, 0x11, 0x0F, 0x01, 0x02, 0x0C};

    static const uint8_t GLYPH_A[7]     = {0x0E, 0x11, 0x11, 0x1F, 0x11, 0x11, 0x11};
    static const uint8_t GLYPH_B[7]     = {0x1E, 0x11, 0x11, 0x1E, 0x11, 0x11, 0x1E};
    static const uint8_t GLYPH_C[7]     = {0x0E, 0x11, 0x10, 0x10, 0x10, 0x11, 0x0E};
    static const uint8_t GLYPH_D[7]     = {0x1C, 0x12, 0x11, 0x11, 0x11, 0x12, 0x1C};
    static const uint8_t GLYPH_E[7]     = {0x1F, 0x10, 0x10, 0x1E, 0x10, 0x10, 0x1F};
    static const uint8_t GLYPH_F[7]     = {0x1F, 0x10, 0x10, 0x1E, 0x10, 0x10, 0x10};
    static const uint8_t GLYPH_G[7]     = {0x0E, 0x11, 0x10, 0x17, 0x11, 0x11, 0x0E};
    static const uint8_t GLYPH_H[7]     = {0x11, 0x11, 0x11, 0x1F, 0x11, 0x11, 0x11};
    static const uint8_t GLYPH_I[7]     = {0x0E, 0x04, 0x04, 0x04, 0x04, 0x04, 0x0E};
    static const uint8_t GLYPH_J[7]     = {0x07, 0x02, 0x02, 0x02, 0x02, 0x12, 0x0C};
    static const uint8_t GLYPH_K[7]     = {0x11, 0x12, 0x14, 0x18, 0x14, 0x12, 0x11};
    static const uint8_t GLYPH_L[7]     = {0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x1F};
    static const uint8_t GLYPH_M[7]     = {0x11, 0x1B, 0x15, 0x11, 0x11, 0x11, 0x11};
    static const uint8_t GLYPH_N[7]     = {0x11, 0x19, 0x15, 0x13, 0x11, 0x11, 0x11};
    static const uint8_t GLYPH_O[7]     = {0x0E, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0E};
    static const uint8_t GLYPH_P[7]     = {0x1E, 0x11, 0x11, 0x1E, 0x10, 0x10, 0x10};
    static const uint8_t GLYPH_Q[7]     = {0x0E, 0x11, 0x11, 0x11, 0x15, 0x09, 0x16};
    static const uint8_t GLYPH_R[7]     = {0x1E, 0x11, 0x11, 0x1E, 0x14, 0x12, 0x11};
    static const uint8_t GLYPH_S[7]     = {0x0E, 0x11, 0x10, 0x0E, 0x01, 0x11, 0x0E};
    static const uint8_t GLYPH_T[7]     = {0x1F, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04};
    static const uint8_t GLYPH_U[7]     = {0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0E};
    static const uint8_t GLYPH_V[7]     = {0x11, 0x11, 0x11, 0x11, 0x11, 0x0A, 0x04};
    static const uint8_t GLYPH_W[7]     = {0x11, 0x11, 0x11, 0x15, 0x15, 0x1B, 0x11};
    static const uint8_t GLYPH_X[7]     = {0x11, 0x11, 0x0A, 0x04, 0x0A, 0x11, 0x11};
    static const uint8_t GLYPH_Y[7]     = {0x11, 0x11, 0x0A, 0x04, 0x04, 0x04, 0x04};
    static const uint8_t GLYPH_Z[7]     = {0x1F, 0x01, 0x02, 0x04, 0x08, 0x10, 0x1F};

    static const uint8_t GLYPH_COLON[7] = {0x00, 0x0C, 0x0C, 0x00, 0x0C, 0x0C, 0x00};
    static const uint8_t GLYPH_PCT[7]   = {0x19, 0x19, 0x02, 0x04, 0x08, 0x13, 0x13};
    static const uint8_t GLYPH_SLASH[7] = {0x01, 0x02, 0x04, 0x08, 0x10, 0x00, 0x00};
    static const uint8_t GLYPH_MINUS[7] = {0x00, 0x00, 0x00, 0x1F, 0x00, 0x00, 0x00};
    static const uint8_t GLYPH_EXCL[7]  = {0x04, 0x04, 0x04, 0x04, 0x00, 0x04, 0x00};
    static const uint8_t GLYPH_LBRK[7]  = {0x0E, 0x08, 0x08, 0x08, 0x08, 0x08, 0x0E};
    static const uint8_t GLYPH_RBRK[7]  = {0x0E, 0x02, 0x02, 0x02, 0x02, 0x02, 0x0E};
    static const uint8_t GLYPH_DOT[7]   = {0x00, 0x00, 0x00, 0x00, 0x00, 0x0C, 0x0C};

    switch (c) {
        case '0': return GLYPH_0;
        case '1': return GLYPH_1;
        case '2': return GLYPH_2;
        case '3': return GLYPH_3;
        case '4': return GLYPH_4;
        case '5': return GLYPH_5;
        case '6': return GLYPH_6;
        case '7': return GLYPH_7;
        case '8': return GLYPH_8;
        case '9': return GLYPH_9;
        case 'A': return GLYPH_A;
        case 'B': return GLYPH_B;
        case 'C': return GLYPH_C;
        case 'D': return GLYPH_D;
        case 'E': return GLYPH_E;
        case 'F': return GLYPH_F;
        case 'G': return GLYPH_G;
        case 'H': return GLYPH_H;
        case 'I': return GLYPH_I;
        case 'J': return GLYPH_J;
        case 'K': return GLYPH_K;
        case 'L': return GLYPH_L;
        case 'M': return GLYPH_M;
        case 'N': return GLYPH_N;
        case 'O': return GLYPH_O;
        case 'P': return GLYPH_P;
        case 'Q': return GLYPH_Q;
        case 'R': return GLYPH_R;
        case 'S': return GLYPH_S;
        case 'T': return GLYPH_T;
        case 'U': return GLYPH_U;
        case 'V': return GLYPH_V;
        case 'W': return GLYPH_W;
        case 'X': return GLYPH_X;
        case 'Y': return GLYPH_Y;
        case 'Z': return GLYPH_Z;
        case ':': return GLYPH_COLON;
        case '%': return GLYPH_PCT;
        case '/': return GLYPH_SLASH;
        case '-': return GLYPH_MINUS;
        case '!': return GLYPH_EXCL;
        case '[': return GLYPH_LBRK;
        case ']': return GLYPH_RBRK;
        case '.': return GLYPH_DOT;
        default:  return GLYPH_BLANK;
    }
}

HUD::HUD() : quadMesh(CreateScreenQuad()), hudScale(1.0f) {}

void HUD::SetScale(float scale) {
    hudScale = std::clamp(scale, 0.7f, 2.0f);
}

float HUD::GetVirtualHeight(int, int) const {
    return 720.0f / hudScale;
}

float HUD::GetVirtualWidth(int screenWidth, int screenHeight) const {
    if (screenHeight <= 0) return 1280.0f;
    float aspect = static_cast<float>(screenWidth) / static_cast<float>(screenHeight);
    return GetVirtualHeight(screenWidth, screenHeight) * aspect;
}

void HUD::DrawRect(const Shader& shader, float x, float y, float w, float h,
                   const glm::vec3& color, float alpha) const {
    if (w <= 0.0f || h <= 0.0f) return;

    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(x, y, 0.0f));
    model = glm::scale(model, glm::vec3(w, h, 1.0f));
    shader.SetMat4("uModel", model);
    shader.SetInt("uUseColorOverride", 1);
    shader.SetVec3("uColorOverride", color);
    shader.SetFloat("uAlpha", alpha);
    quadMesh.Draw(shader);
}

void HUD::DrawRectOutline(const Shader& shader, float x, float y, float w, float h, float thickness,
                          const glm::vec3& color, float alpha) const {
    DrawRect(shader, x, y, w, thickness, color, alpha);
    DrawRect(shader, x, y + h - thickness, w, thickness, color, alpha);
    DrawRect(shader, x, y + thickness, thickness, h - 2.0f * thickness, color, alpha);
    DrawRect(shader, x + w - thickness, y + thickness, thickness, h - 2.0f * thickness, color, alpha);
}

void HUD::DrawSlantedRect(const Shader& shader, float x, float y, float w, float h, float slantOffset,
                          const glm::vec3& color, float alpha) const {
    if (w <= 0.0f || h <= 0.0f) return;

    // Slant forward `/`: top edge at x + slantOffset, bottom edge at x
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(x + slantOffset, y, 0.0f));
    glm::mat4 shear = glm::mat4(1.0f);
    shear[1][0] = -slantOffset / h;
    model = model * shear;
    model = glm::scale(model, glm::vec3(w, h, 1.0f));

    shader.SetMat4("uModel", model);
    shader.SetInt("uUseColorOverride", 1);
    shader.SetVec3("uColorOverride", color);
    shader.SetFloat("uAlpha", alpha);
    quadMesh.Draw(shader);
}

void HUD::DrawSlantedRectOutline(const Shader& shader, float x, float y, float w, float h, float slantOffset,
                                float thickness, const glm::vec3& color, float alpha) const {
    // Top horizontal edge (shifted right by slantOffset)
    DrawRect(shader, x + slantOffset, y, w, thickness, color, alpha);
    // Bottom horizontal edge (starts at x)
    DrawRect(shader, x, y + h - thickness, w, thickness, color, alpha);
    // Left slanted edge
    DrawSlantedRect(shader, x, y, thickness, h, slantOffset, color, alpha);
    // Right slanted edge
    DrawSlantedRect(shader, x + w - thickness, y, thickness, h, slantOffset, color, alpha);
}

void HUD::DrawSlantedSegmentedBar(const Shader& shader, float x, float y, float w, float h,
                                 float slantOffset, int totalSegments, float fillRatio,
                                 const glm::vec3& fillColor, const glm::vec3& emptyColor,
                                 const glm::vec3& borderColor) const {
    // Backdrop & outline frame with slant
    DrawSlantedRect(shader, x - 2.0f, y - 2.0f, w + 4.0f, h + 4.0f, slantOffset, glm::vec3(0.04f, 0.06f, 0.10f), 0.9f);
    DrawSlantedRectOutline(shader, x - 2.0f, y - 2.0f, w + 4.0f, h + 4.0f, slantOffset, 1.5f, borderColor, 0.85f);

    float gap = 2.5f;
    float segW = (w - (totalSegments - 1) * gap) / totalSegments;

    for (int i = 0; i < totalSegments; ++i) {
        float segX = x + i * (segW + gap);
        float threshold = static_cast<float>(i + 1) / totalSegments;
        bool isFilled = (fillRatio >= threshold - (1.0f / totalSegments) * 0.5f);

        glm::vec3 col = isFilled ? fillColor : emptyColor;
        float alpha = isFilled ? 0.95f : 0.35f;
        DrawSlantedRect(shader, segX, y, segW, h, slantOffset, col, alpha);

        // Specular highlight along top edge of filled cells
        if (isFilled) {
            float glintY = y + h - 2.0f;
            float glintSlantX = slantOffset * (2.0f / h);
            DrawSlantedRect(shader, segX + slantOffset - glintSlantX, glintY, segW, 2.0f, glintSlantX, glm::mix(fillColor, glm::vec3(1.0f), 0.55f), 0.85f);
        }
    }
}

void HUD::DrawDiamond(const Shader& shader, float cx, float cy, float radius,
                     const glm::vec3& color, float alpha) const {
    if (radius <= 0.0f) return;

    float side = radius * 1.41421356f;
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(cx, cy, 0.0f));
    model = glm::rotate(model, glm::radians(45.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    model = glm::scale(model, glm::vec3(side, side, 1.0f));
    model = glm::translate(model, glm::vec3(-0.5f, -0.5f, 0.0f));

    shader.SetMat4("uModel", model);
    shader.SetInt("uUseColorOverride", 1);
    shader.SetVec3("uColorOverride", color);
    shader.SetFloat("uAlpha", alpha);
    quadMesh.Draw(shader);
}

void HUD::DrawSlantedGradientBar(const Shader& shader, float x, float y, float w, float h,
                                float slantOffset, float fillRatio,
                                const glm::vec3& colLeft, const glm::vec3& colMid, const glm::vec3& colRight,
                                const glm::vec3& emptyColor,
                                const glm::vec3& borderColor,
                                float borderThickness) const {
    if (w <= 0.0f || h <= 0.0f) return;

    float clampedRatio = std::clamp(fillRatio, 0.0f, 1.0f);

    // 1. Draw empty/depleted background parallelogram
    DrawSlantedRect(shader, x, y, w, h, slantOffset, emptyColor, 0.95f);

    // 2. Draw smooth continuous gradient fill across clampedRatio
    if (clampedRatio > 0.005f) {
        float filledW = w * clampedRatio;
        const int numSlices = 28;
        float sliceW = filledW / static_cast<float>(numSlices);

        for (int i = 0; i < numSlices; ++i) {
            float u0 = (static_cast<float>(i) / numSlices) * clampedRatio;
            float u1 = (static_cast<float>(i + 1) / numSlices) * clampedRatio;
            float uMid = (u0 + u1) * 0.5f;

            glm::vec3 sliceCol;
            if (uMid < 0.5f) {
                float t = uMid / 0.5f;
                sliceCol = glm::mix(colLeft, colMid, t);
            } else {
                float t = (uMid - 0.5f) / 0.5f;
                sliceCol = glm::mix(colMid, colRight, t);
            }

            float sx = x + (static_cast<float>(i) * sliceW);
            DrawSlantedRect(shader, sx, y, sliceW + 0.6f, h, slantOffset, sliceCol, 1.0f);
        }
    }

    // 3. Thick solid white border outline
    DrawSlantedRectOutline(shader, x, y, w, h, slantOffset, borderThickness, borderColor, 1.0f);
}

void HUD::DrawRadialGradientDiamond(const Shader& shader, float cx, float cy, float radius,
                                   const glm::vec3& innerColor, const glm::vec3& outerColor,
                                   const glm::vec3& borderColor, float alpha, bool filled) const {
    if (radius <= 0.0f) return;

    // Dark crisp outer border diamond
    DrawDiamond(shader, cx, cy, radius + 2.0f, borderColor, alpha * 0.95f);

    if (filled) {
        // Multi-layer radial glow from outer edge to radiant inner core
        DrawDiamond(shader, cx, cy, radius, outerColor, alpha * 0.95f);
        DrawDiamond(shader, cx, cy, radius * 0.72f, glm::mix(outerColor, innerColor, 0.45f), alpha * 0.98f);
        DrawDiamond(shader, cx, cy, radius * 0.44f, glm::mix(outerColor, innerColor, 0.85f), alpha * 1.0f);
        DrawDiamond(shader, cx, cy, radius * 0.20f, glm::vec3(1.0f, 1.0f, 0.85f), alpha * 1.0f);
    } else {
        // Empty slot: dark charcoal transparent backing
        DrawDiamond(shader, cx, cy, radius, glm::vec3(0.08f, 0.09f, 0.14f), alpha * 0.75f);
        DrawDiamond(shader, cx, cy, radius * 0.70f, glm::vec3(0.04f, 0.05f, 0.08f), alpha * 0.85f);
    }
}

void HUD::DrawStarfighterIcon(const Shader& shader, float cx, float cy, float size) const {
    // 2D vector logo matching player starfighter silhouette (Ex-Zodiac bottom right live icon)
    float s = size;
    // Fuselage / body (royal indigo triangle)
    DrawDiamond(shader, cx, cy, s * 0.55f, glm::vec3(0.32f, 0.18f, 0.65f), 1.0f);
    // Swept wings (magenta-purple chevrons)
    DrawSlantedRect(shader, cx - s * 0.85f, cy - s * 0.15f, s * 0.75f, s * 0.35f, -s * 0.25f, glm::vec3(0.75f, 0.22f, 0.65f), 1.0f);
    DrawSlantedRect(shader, cx + s * 0.10f, cy - s * 0.15f, s * 0.75f, s * 0.35f, s * 0.25f, glm::vec3(0.75f, 0.22f, 0.65f), 1.0f);
    // Cockpit glint (electric azure dot)
    DrawDiamond(shader, cx, cy - s * 0.22f, s * 0.22f, glm::vec3(0.25f, 0.95f, 1.0f), 1.0f);
    // Twin thruster dots
    DrawRect(shader, cx - s * 0.28f, cy + s * 0.35f, s * 0.18f, s * 0.20f, glm::vec3(1.0f, 0.85f, 0.2f), 1.0f);
    DrawRect(shader, cx + s * 0.10f, cy + s * 0.35f, s * 0.18f, s * 0.20f, glm::vec3(1.0f, 0.85f, 0.2f), 1.0f);
}

void HUD::DrawHitCombo(const Shader& shader, float cx, float cy, int comboHits,
                      float comboTimer, float maxTimer, float animScale) const {
    if (comboHits < 2 || comboTimer <= 0.0f) return;

    float timeRatio = std::clamp(comboTimer / maxTimer, 0.0f, 1.0f);

    glm::vec3 primaryColor = glm::vec3(1.0f, 0.88f, 0.2f); // Golden yellow
    std::string comboLabel = "HIT x" + std::to_string(comboHits);

    if (comboHits >= 10) {
        primaryColor = glm::vec3(1.0f, 0.25f, 0.85f); // Neon magenta ULTRA
        comboLabel = "ULTRA x" + std::to_string(comboHits) + "!";
    } else if (comboHits >= 5) {
        primaryColor = glm::vec3(0.2f, 1.0f, 0.88f); // Radiant cyan COMBO
        comboLabel = "COMBO x" + std::to_string(comboHits) + "!";
    }

    float badgeW = 160.0f * animScale;
    float badgeH = 34.0f * animScale;
    float slant = 14.0f;
    float bx = cx - badgeW * 0.5f;
    float by = cy - badgeH * 0.5f;

    // 1. Slanted dark bezel backdrop
    DrawSlantedRect(shader, bx, by, badgeW, badgeH, slant, glm::vec3(0.04f, 0.07f, 0.12f), 0.88f);
    DrawSlantedRectOutline(shader, bx, by, badgeW, badgeH, slant, 2.0f, primaryColor, 0.95f);

    // Accent corner ticks
    DrawSlantedRect(shader, bx, by, 8.0f, badgeH, slant, primaryColor, 0.9f);
    DrawSlantedRect(shader, bx + badgeW - 8.0f, by, 8.0f, badgeH, slant, primaryColor, 0.9f);

    // 2. Combo text with scale bounce
    float textScale = 1.9f * animScale;
    float textW = comboLabel.length() * 6.0f * textScale;
    float tx = cx - textW * 0.5f + slant * 0.4f;
    float ty = by + 6.0f;
    DrawText(shader, comboLabel, tx, ty, textScale, primaryColor, 1.0f);

    // 3. Slanted combo timer bar (depletes as timer runs out)
    float barW = badgeW - 28.0f;
    float barH = 4.0f;
    float barX = bx + 14.0f;
    float barY = by + badgeH + 4.0f;
    DrawSlantedRect(shader, barX, barY, barW, barH, 4.0f, glm::vec3(0.12f, 0.16f, 0.22f), 0.8f);
    DrawSlantedRect(shader, barX, barY, barW * timeRatio, barH, 4.0f, primaryColor, 0.95f);

    // 4. Multiplier badge tag underneath
    int mult = std::clamp(comboHits, 1, 8);
    std::string multStr = "[ x" + std::to_string(mult) + " BONUS ]";
    float multScale = 1.1f;
    float multW = multStr.length() * 6.0f * multScale;
    DrawText(shader, multStr, cx - multW * 0.5f + 4.0f, barY + 8.0f, multScale, glm::vec3(1.0f, 0.95f, 0.7f), 0.9f);
}

void HUD::DrawSegmentedBar(const Shader& shader, float x, float y, float w, float h,
                          int totalSegments, float fillRatio,
                          const glm::vec3& fillColor, const glm::vec3& emptyColor,
                          const glm::vec3& borderColor) const {
    // Backdrop & outline frame
    DrawRect(shader, x - 2.0f, y - 2.0f, w + 4.0f, h + 4.0f, glm::vec3(0.04f, 0.06f, 0.10f), 0.9f);
    DrawRectOutline(shader, x - 2.0f, y - 2.0f, w + 4.0f, h + 4.0f, 1.5f, borderColor, 0.85f);

    float gap = 2.0f;
    float segW = (w - (totalSegments - 1) * gap) / totalSegments;

    for (int i = 0; i < totalSegments; ++i) {
        float segX = x + i * (segW + gap);
        float threshold = static_cast<float>(i + 1) / totalSegments;
        bool isFilled = (fillRatio >= threshold - (1.0f / totalSegments) * 0.5f);

        glm::vec3 col = isFilled ? fillColor : emptyColor;
        float alpha = isFilled ? 0.95f : 0.40f;
        DrawRect(shader, segX, y, segW, h, col, alpha);
    }
}

void HUD::DrawChar(const Shader& shader, char c, float x, float y, float scale,
                   const glm::vec3& color, float alpha) const {
    const uint8_t* glyph = GetGlyph(c);

    for (int row = 0; row < 7; ++row) {
        uint8_t bits = glyph[row];
        int runStart = -1;

        for (int col = 0; col < 5; ++col) {
            bool isPixel = (bits & (1 << (4 - col))) != 0;

            if (isPixel) {
                if (runStart < 0) runStart = col;
            } else {
                if (runStart >= 0) {
                    float px = x + runStart * scale;
                    float py = y + row * scale;
                    float pw = (col - runStart) * scale;
                    DrawRect(shader, px, py, pw, scale, color, alpha);
                    runStart = -1;
                }
            }
        }

        if (runStart >= 0) {
            float px = x + runStart * scale;
            float py = y + row * scale;
            float pw = (5 - runStart) * scale;
            DrawRect(shader, px, py, pw, scale, color, alpha);
        }
    }
}

void HUD::DrawText(const Shader& shader, const std::string& text, float x, float y, float scale,
                   const glm::vec3& color, float alpha) const {
    float shadowOffset = std::max(1.0f, std::round(scale * 0.85f));
    float charAdvance = 6.0f * scale;

    // 1. Retro arcade black drop shadow
    float curX = x + shadowOffset;
    for (char c : text) {
        DrawChar(shader, c, curX, y + shadowOffset, scale, glm::vec3(0.02f, 0.03f, 0.05f), alpha * 0.95f);
        curX += charAdvance;
    }

    // 2. Crisp foreground glyph
    curX = x;
    for (char c : text) {
        DrawChar(shader, c, curX, y, scale, color, alpha);
        curX += charAdvance;
    }
}

void HUD::DrawCockpitCanopyOverlay(const Shader& shader, int screenWidth, int screenHeight,
                                  float pitch, float roll) const {
    float sw = static_cast<float>(screenWidth);
    float sh = static_cast<float>(screenHeight);

    // 1. Lower Dashboard Console Cowl (dark armored composite rim)
    DrawRect(shader, 0.0f, sh - 48.0f, sw, 48.0f, glm::vec3(0.04f, 0.06f, 0.09f), 0.95f);
    DrawRect(shader, 0.0f, sh - 52.0f, sw, 4.0f, glm::vec3(0.12f, 0.22f, 0.35f), 0.90f);
    DrawRect(shader, sw * 0.5f - 180.0f, sh - 62.0f, 360.0f, 12.0f, glm::vec3(0.06f, 0.09f, 0.14f), 0.92f);
    DrawRectOutline(shader, sw * 0.5f - 180.0f, sh - 62.0f, 360.0f, 12.0f, 1.5f, glm::vec3(0.2f, 0.55f, 0.85f), 0.85f);

    // Center Cockpit Callout
    DrawText(shader, "INTERCEPTOR COCKPIT // SYS ONLINE", sw * 0.5f - 130.0f, sh - 32.0f, 1.3f, glm::vec3(0.3f, 0.85f, 1.0f), 0.9f);

    // 2. Left & Right Angled Canopy Struts (A-Pillars)
    float strutW = 32.0f;
    DrawRect(shader, 0.0f, 0.0f, strutW, sh, glm::vec3(0.05f, 0.07f, 0.11f), 0.88f);
    DrawRect(shader, strutW, 0.0f, 3.0f, sh, glm::vec3(0.2f, 0.45f, 0.7f), 0.75f);

    DrawRect(shader, sw - strutW, 0.0f, strutW, sh, glm::vec3(0.05f, 0.07f, 0.11f), 0.88f);
    DrawRect(shader, sw - strutW - 3.0f, 0.0f, 3.0f, sh, glm::vec3(0.2f, 0.45f, 0.7f), 0.75f);

    // Upper Canopy Brow Frame
    DrawRect(shader, 0.0f, 0.0f, sw, 22.0f, glm::vec3(0.05f, 0.07f, 0.11f), 0.88f);
    DrawRect(shader, 0.0f, 22.0f, sw, 2.5f, glm::vec3(0.2f, 0.45f, 0.7f), 0.75f);

    // 3. Canopy Glass Reflection Glare Line (Subtle diagonal cyan shimmer)
    float curTime = static_cast<float>(glfwGetTime());
    float glareX = std::fmod(curTime * 90.0f, sw * 1.5f) - sw * 0.25f;
    DrawRect(shader, glareX, 24.0f, 14.0f, sh - 76.0f, glm::vec3(0.3f, 0.85f, 1.0f), 0.08f);
    DrawRect(shader, glareX + 22.0f, 24.0f, 6.0f, sh - 76.0f, glm::vec3(0.3f, 0.85f, 1.0f), 0.12f);

    // 4. Center Pitch Horizon Ladder (HUD Flight Director)
    float centerX = sw * 0.5f;
    float centerY = sh * 0.5f;
    float pitchOffsetY = -pitch * 3.2f; // Pitch shifts horizon vertically

    // Central Boresight Cross
    DrawRect(shader, centerX - 18.0f, centerY, 12.0f, 2.0f, glm::vec3(0.2f, 0.9f, 0.7f), 0.8f);
    DrawRect(shader, centerX + 6.0f, centerY, 12.0f, 2.0f, glm::vec3(0.2f, 0.9f, 0.7f), 0.8f);
    DrawRect(shader, centerX - 1.0f, centerY - 8.0f, 2.0f, 6.0f, glm::vec3(0.2f, 0.9f, 0.7f), 0.8f);

    // Horizon Line
    float horizY = centerY + pitchOffsetY;
    if (horizY > 60.0f && horizY < sh - 80.0f) {
        DrawRect(shader, centerX - 90.0f, horizY, 60.0f, 2.0f, glm::vec3(0.2f, 0.9f, 0.4f), 0.75f);
        DrawRect(shader, centerX + 30.0f, horizY, 60.0f, 2.0f, glm::vec3(0.2f, 0.9f, 0.4f), 0.75f);
        DrawText(shader, "HORIZON", centerX - 25.0f, horizY - 10.0f, 1.0f, glm::vec3(0.2f, 0.9f, 0.4f), 0.7f);
    }

    // +10 Deg Pitch Bar
    float pitchUpY = centerY + pitchOffsetY - 45.0f;
    if (pitchUpY > 60.0f && pitchUpY < sh - 80.0f) {
        DrawRect(shader, centerX - 55.0f, pitchUpY, 35.0f, 1.5f, glm::vec3(0.2f, 0.8f, 1.0f), 0.65f);
        DrawRect(shader, centerX + 20.0f, pitchUpY, 35.0f, 1.5f, glm::vec3(0.2f, 0.8f, 1.0f), 0.65f);
        DrawText(shader, "+10", centerX + 60.0f, pitchUpY - 4.0f, 0.9f, glm::vec3(0.2f, 0.8f, 1.0f), 0.65f);
    }

    // -10 Deg Pitch Bar
    float pitchDnY = centerY + pitchOffsetY + 45.0f;
    if (pitchDnY > 60.0f && pitchDnY < sh - 80.0f) {
        DrawRect(shader, centerX - 55.0f, pitchDnY, 35.0f, 1.5f, glm::vec3(1.0f, 0.65f, 0.2f), 0.65f);
        DrawRect(shader, centerX + 20.0f, pitchDnY, 35.0f, 1.5f, glm::vec3(1.0f, 0.65f, 0.2f), 0.65f);
        DrawText(shader, "-10", centerX + 60.0f, pitchDnY - 4.0f, 0.9f, glm::vec3(1.0f, 0.65f, 0.2f), 0.65f);
    }
}

void HUD::DrawRadarMinimap(const Shader& shader, float rx, float ry, float radius,
                          const glm::vec3& playerPos, float playerYaw,
                          const glm::vec3& bossPos, bool bossActive,
                          const std::vector<glm::vec3>& enemyPositions) const {
    float curTime = static_cast<float>(glfwGetTime());

    // 1. Radar Backplate
    DrawRect(shader, rx - radius - 6.0f, ry - radius - 6.0f, radius * 2.0f + 12.0f, radius * 2.0f + 12.0f,
             glm::vec3(0.03f, 0.06f, 0.10f), 0.82f);
    DrawRectOutline(shader, rx - radius - 6.0f, ry - radius - 6.0f, radius * 2.0f + 12.0f, radius * 2.0f + 12.0f,
                    1.5f, glm::vec3(0.2f, 0.45f, 0.70f), 0.75f);

    // Range rings
    DrawRectOutline(shader, rx - radius * 0.5f, ry - radius * 0.5f, radius, radius, 1.0f,
                    glm::vec3(0.12f, 0.28f, 0.42f), 0.6f);
    DrawRectOutline(shader, rx - radius, ry - radius, radius * 2.0f, radius * 2.0f, 1.5f,
                    glm::vec3(0.2f, 0.55f, 0.85f), 0.85f);

    // Crosshairs
    DrawRect(shader, rx - radius, ry, radius * 2.0f, 1.0f, glm::vec3(0.15f, 0.35f, 0.5f), 0.5f);
    DrawRect(shader, rx, ry - radius, 1.0f, radius * 2.0f, glm::vec3(0.15f, 0.35f, 0.5f), 0.5f);

    // Labels
    DrawText(shader, "N", rx - 3.0f, ry - radius - 4.0f, 0.9f, glm::vec3(0.3f, 0.8f, 1.0f), 0.8f);
    DrawText(shader, "RADAR 360", rx - radius + 4.0f, ry + radius - 10.0f, 0.9f, glm::vec3(0.3f, 0.8f, 1.0f), 0.8f);

    // 2. Rotating Sweep Line
    float sweepAngle = curTime * 140.0f;
    float sweepRad = glm::radians(sweepAngle);
    float sweepX = rx + std::sin(sweepRad) * (radius - 2.0f);
    float sweepY = ry - std::cos(sweepRad) * (radius - 2.0f);
    DrawRect(shader, (rx + sweepX) * 0.5f - 1.0f, (ry + sweepY) * 0.5f - 1.0f, 3.0f, 3.0f, glm::vec3(0.2f, 1.0f, 0.7f), 0.7f);

    // Range scale: 220.0f world units maps to radar radius
    float worldRange = 220.0f;
    float scale = radius / worldRange;

    // 3. Enemy Drone Blips
    for (const auto& ep : enemyPositions) {
        float dx = ep.x - playerPos.x;
        float dz = ep.z - playerPos.z;

        float mapX = rx + dx * scale;
        float mapY = ry + dz * scale;

        if (glm::distance(glm::vec2(mapX, mapY), glm::vec2(rx, ry)) <= radius) {
            DrawRect(shader, mapX - 2.0f, mapY - 2.0f, 4.0f, 4.0f, glm::vec3(1.0f, 0.2f, 0.2f), 0.95f);
        }
    }

    // 4. Dreadnought Boss Blip
    if (bossActive) {
        float dx = bossPos.x - playerPos.x;
        float dz = bossPos.z - playerPos.z;

        float mapX = rx + dx * scale;
        float mapY = ry + dz * scale;

        float dist = glm::distance(glm::vec2(mapX, mapY), glm::vec2(rx, ry));
        if (dist > radius - 6.0f) {
            glm::vec2 dir = glm::normalize(glm::vec2(mapX - rx, mapY - ry));
            mapX = rx + dir.x * (radius - 6.0f);
            mapY = ry + dir.y * (radius - 6.0f);
        }

        float flash = (std::sin(curTime * 12.0f) > 0.0f) ? 1.0f : 0.4f;
        DrawRect(shader, mapX - 4.0f, mapY - 4.0f, 8.0f, 8.0f, glm::vec3(1.0f, 0.1f, 0.1f), flash);
        DrawRectOutline(shader, mapX - 4.0f, mapY - 4.0f, 8.0f, 8.0f, 1.5f, glm::vec3(1.0f, 0.85f, 0.2f), 0.9f);
        DrawText(shader, "BOSS", mapX - 10.0f, mapY - 12.0f, 0.9f, glm::vec3(1.0f, 0.3f, 0.3f), 0.9f);
    }

    // 5. Player Interceptor Blip in Center
    DrawRect(shader, rx - 3.0f, ry - 3.0f, 6.0f, 6.0f, glm::vec3(0.2f, 0.95f, 0.35f), 1.0f);
    float headRad = glm::radians(playerYaw);
    float hx = rx - std::sin(headRad) * 10.0f;
    float hy = ry - std::cos(headRad) * 10.0f;
    DrawRect(shader, hx - 1.5f, hy - 1.5f, 3.0f, 3.0f, glm::vec3(0.4f, 1.0f, 0.8f), 1.0f);
}

void HUD::DrawCommsBox(const Shader& shader, int screenWidth, int screenHeight,
                      bool hasMessage, int speakerId,
                      const std::string& callsign,
                      const std::string& line1, const std::string& line2,
                      const glm::vec3& themeColor, float /*timer*/) const {
    if (!hasMessage) return;

    float curTime = static_cast<float>(glfwGetTime());

    float boxW = 460.0f;
    float boxH = 90.0f;
    float boxX = (static_cast<float>(screenWidth) - boxW) * 0.5f;
    float boxY = static_cast<float>(screenHeight) - boxH - 24.0f;

    // 1. Semi-transparent dark tactical backdrop (Ex-Zodiac purple gradient style)
    DrawRect(shader, boxX, boxY, boxW, boxH, glm::vec3(0.22f, 0.10f, 0.38f), 0.92f);
    DrawRectOutline(shader, boxX, boxY, boxW, boxH, 2.0f, glm::vec3(1.0f, 1.0f, 1.0f), 0.95f);

    // Cyberpunk corner ticks
    DrawRect(shader, boxX, boxY, 12.0f, 3.0f, themeColor, 1.0f);
    DrawRect(shader, boxX, boxY, 3.0f, 12.0f, themeColor, 1.0f);
    DrawRect(shader, boxX + boxW - 12.0f, boxY, 12.0f, 3.0f, themeColor, 1.0f);
    DrawRect(shader, boxX + boxW - 3.0f, boxY, 3.0f, 12.0f, themeColor, 1.0f);
    DrawRect(shader, boxX, boxY + boxH - 3.0f, 12.0f, 3.0f, themeColor, 1.0f);
    DrawRect(shader, boxX, boxY + boxH - 12.0f, 3.0f, 12.0f, themeColor, 1.0f);
    DrawRect(shader, boxX + boxW - 12.0f, boxY + boxH - 3.0f, 12.0f, 3.0f, themeColor, 1.0f);
    DrawRect(shader, boxX + boxW - 3.0f, boxY + boxH - 12.0f, 3.0f, 12.0f, themeColor, 1.0f);

    // 2. Vector Pilot Portrait Frame (Left side: 66 x 66)
    float portX = boxX + 12.0f;
    float portY = boxY + 12.0f;
    float portSize = 66.0f;

    DrawRect(shader, portX, portY, portSize, portSize, glm::vec3(0.02f, 0.04f, 0.08f), 0.95f);
    DrawRectOutline(shader, portX, portY, portSize, portSize, 1.0f, themeColor, 0.75f);

    float cx = portX + portSize * 0.5f;
    float cy = portY + portSize * 0.5f;

    if (speakerId == 0) {
        // Echo-1: Striker (Avian / Raptor Helmet Visor)
        DrawRect(shader, cx - 18.0f, cy - 20.0f, 36.0f, 14.0f, glm::vec3(0.2f, 0.22f, 0.26f), 0.9f);
        DrawRect(shader, cx - 14.0f, cy - 23.0f, 28.0f, 3.5f, themeColor, 0.95f); // Crest
        DrawRect(shader, cx - 16.0f, cy - 7.0f, 32.0f, 9.0f, glm::vec3(1.0f, 0.85f, 0.2f), 0.95f); // Amber visor
        DrawRect(shader, cx - 5.0f, cy + 3.0f, 10.0f, 7.0f, themeColor, 0.9f); // Beak vent
        DrawRect(shader, cx - 21.0f, cy - 5.0f, 5.0f, 14.0f, glm::vec3(0.5f, 0.55f, 0.6f), 0.9f); // Mic
        DrawRect(shader, cx - 19.0f, cy + 7.0f, 14.0f, 2.0f, glm::vec3(0.5f, 0.55f, 0.6f), 0.9f);
    } else {
        // Echo-2: Aegis (Feline / Lynx Helmet Visor)
        DrawRect(shader, cx - 18.0f, cy - 18.0f, 36.0f, 13.0f, glm::vec3(0.18f, 0.24f, 0.28f), 0.9f);
        DrawRect(shader, cx - 19.0f, cy - 24.0f, 7.0f, 7.0f, themeColor, 0.95f); // Left ear
        DrawRect(shader, cx + 12.0f, cy - 24.0f, 7.0f, 7.0f, themeColor, 0.95f); // Right ear
        DrawRect(shader, cx - 17.0f, cy - 6.0f, 34.0f, 8.0f, glm::vec3(0.2f, 0.95f, 0.9f), 0.95f); // Cyan visor
        DrawRect(shader, cx - 9.0f, cy + 3.0f, 18.0f, 5.0f, glm::vec3(0.25f, 0.3f, 0.35f), 0.9f);
        DrawRect(shader, cx + 17.0f, cy - 7.0f, 4.0f, 13.0f, glm::vec3(0.5f, 0.6f, 0.65f), 0.9f);
    }

    // Animated Speech Equalizer / Waveform Bars
    float barStartX = portX + 9.0f;
    float barY = portY + portSize - 10.0f;
    for (int b = 0; b < 7; ++b) {
        float wave = std::abs(std::sin(curTime * 20.0f + b * 1.3f));
        float barH = 3.0f + wave * 9.0f;
        DrawRect(shader, barStartX + b * 7.0f, barY - barH, 4.0f, barH, themeColor, 0.9f);
    }

    // 3. Header Text: Callsign & Blinking Transmission Tag
    float textX = portX + portSize + 14.0f;
    DrawText(shader, callsign, textX, boxY + 14.0f, 1.4f, themeColor, 1.0f);

    float blink = (std::sin(curTime * 8.0f) > 0.0f) ? 1.0f : 0.25f;
    DrawRect(shader, boxX + boxW - 85.0f, boxY + 16.0f, 6.0f, 6.0f, glm::vec3(0.2f, 1.0f, 0.4f), blink);
    DrawText(shader, "COMMS", boxX + boxW - 74.0f, boxY + 15.0f, 1.1f, glm::vec3(0.6f, 0.9f, 0.7f), 0.9f);

    // 4. Subtitle Feed Lines
    DrawText(shader, line1, textX, boxY + 38.0f, 1.3f, glm::vec3(0.95f, 0.98f, 1.0f), 0.95f);
    DrawText(shader, line2, textX, boxY + 58.0f, 1.2f, glm::vec3(0.75f, 0.85f, 0.95f), 0.90f);
}

void HUD::Render(const Shader& shader, int screenWidth, int screenHeight,
                 float shield, float maxShield,
                 float boost, float maxBoost, bool isOverheated,
                 bool isDeflecting, int score, int rings,
                 int bombCount, float chargeProgress,
                 bool bossActive, bool bossWarning, float bossHealthRatio,
                 bool bossLeftTurretDown, bool bossRightTurretDown,
                 bool bossShieldDown, bool bossCoreExposed,
                 bool isVictory, bool isGameOver,
                 float leftWingHealth, bool leftWingLost,
                 float rightWingHealth, bool rightWingLost,
                 float wingAlertTimer, const std::string& wingAlertMsg,
                 bool isFirstPerson, float playerPitch, float playerRoll,
                 const glm::vec3& playerPos, float playerYaw,
                 const glm::vec3& bossPos,
                 const std::vector<glm::vec3>& enemyPositions,
                 bool hasComms, int commsSpeaker,
                 const std::string& commsCallsign,
                 const std::string& commsLine1,
                 const std::string& commsLine2,
                 const glm::vec3& commsColor,
                 float commsTimer,
                 int destroyedRelays, int totalRelays,
                 int comboHits, float comboTimer,
                 float comboMaxDuration, float comboAnimScale,
                 int lives) const {
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    float vw = GetVirtualWidth(screenWidth, screenHeight);
    float vh = GetVirtualHeight(screenWidth, screenHeight);

    shader.Activate();
    glm::mat4 ortho = glm::ortho(0.0f, vw, vh, 0.0f, -1.0f, 1.0f);
    shader.SetMat4("uProjection", ortho);
    shader.SetMat4("uView", glm::mat4(1.0f));

    // Disable lighting and fog for 2D UI
    shader.SetInt("uUseLighting", 0);
    shader.SetInt("uUseFog", 0);
    shader.SetInt("uUseColorOverride", 1);

    float curTime = static_cast<float>(glfwGetTime());

    // 0. First-Person Cockpit Canopy Overlay
    if (isFirstPerson && !isVictory && !isGameOver) {
        DrawCockpitCanopyOverlay(shader, static_cast<int>(vw), static_cast<int>(vh), playerPitch, playerRoll);
    }

    // =========================================================================
    // 1. TOP-LEFT: EX-ZODIAC MINIMALIST SCORE & COMBO HIT TEXTS (Images 1 & 2)
    // =========================================================================
    if (!isVictory && !isGameOver) {
        char scoreBuf[32];
        std::snprintf(scoreBuf, sizeof(scoreBuf), "SCORE %d", score);
        DrawText(shader, scoreBuf, 36.0f, 28.0f, 2.4f, glm::vec3(1.0f, 1.0f, 1.0f), 1.0f);

        if (comboHits > 1 && comboTimer > 0.0f) {
            char hitBuf[32];
            std::snprintf(hitBuf, sizeof(hitBuf), "%d HIT", comboHits);
            DrawText(shader, hitBuf, 36.0f, 60.0f, 2.2f, glm::vec3(1.0f, 1.0f, 1.0f), 1.0f);

            float mult = 1.0f + static_cast<float>(comboHits) * 0.05f;
            char multBuf[32];
            std::snprintf(multBuf, sizeof(multBuf), "x%.2f", mult);
            DrawText(shader, multBuf, 36.0f, 90.0f, 2.0f, glm::vec3(1.0f, 1.0f, 1.0f), 1.0f);
        }
    }

    // =========================================================================
    // 2. TOP-MIDDLE: EX-ZODIAC MINIMALIST BOSS HEALTH BAR (Image 3)
    // =========================================================================
    if (bossActive && !isVictory && !isGameOver) {
        float bossW = 680.0f;
        float bossH = 20.0f;
        float bossSlant = 24.0f;
        float bossX = (vw - bossW) * 0.5f;
        float bossY = 36.0f;

        // "BOSS" label centered above the bar
        float bossTextW = 4.0f * 6.0f * 2.2f;
        DrawText(shader, "BOSS", (vw - bossTextW) * 0.5f + bossSlant * 0.5f, bossY - 20.0f, 2.2f, glm::vec3(1.0f, 1.0f, 1.0f), 1.0f);

        // Continuous gradient bar: red to yellow to green with thick white border
        DrawSlantedGradientBar(shader, bossX, bossY, bossW, bossH, bossSlant, bossHealthRatio,
                               glm::vec3(0.92f, 0.35f, 0.28f),  // Left: Red
                               glm::vec3(0.98f, 0.85f, 0.25f),  // Mid: Warm yellow
                               glm::vec3(0.65f, 0.92f, 0.42f),  // Right: Pastel lime green
                               glm::vec3(0.06f, 0.07f, 0.12f),  // Depleted dark background
                               glm::vec3(1.0f, 1.0f, 1.0f),     // Solid white border
                               3.0f);
    }

    // =========================================================================
    // 3. BOTTOM-LEFT: EX-ZODIAC GAUGES & BOMBS (Images 1 & 2)
    // =========================================================================
    if (!isVictory && !isGameOver) {
        float blX = 36.0f;
        float barW = 265.0f;
        float barH = 20.0f;
        float barSlant = 24.0f;
        float diamondY = vh - 36.0f;
        float thrustY = diamondY - 20.0f - barH;
        float blY = thrustY - 26.0f - barH;

        // Wing Damage Indicator (Only shown if a wing is damaged or severed)
        if (leftWingLost || rightWingLost || leftWingHealth < 75.0f || rightWingHealth < 75.0f) {
            float warnY = blY - 34.0f;
            std::string warnText = leftWingLost ? "! L-WING LOST !" : (rightWingLost ? "! R-WING LOST !" : "! WING DAMAGE !");
            float flash = (std::sin(curTime * 14.0f) > 0.0f) ? 1.0f : 0.6f;
            DrawSlantedRect(shader, blX, warnY, 195.0f, 22.0f, barSlant, glm::vec3(0.70f, 0.12f, 0.12f) * flash, 0.92f);
            DrawSlantedRectOutline(shader, blX, warnY, 195.0f, 22.0f, barSlant, 2.0f, glm::vec3(1.0f, 1.0f, 1.0f), 1.0f);
            DrawText(shader, warnText, blX + barSlant + 8.0f, warnY + 5.0f, 1.4f, glm::vec3(1.0f, 1.0f, 1.0f), 1.0f);
        }

        // 1. SHIELD BAR: Red to Yellow gradient with thick white border
        DrawText(shader, "SHIELD", blX + barSlant + 4.0f, blY - 18.0f, 1.6f, glm::vec3(1.0f, 1.0f, 1.0f), 1.0f);
        float sRatio = std::clamp(shield / maxShield, 0.0f, 1.0f);
        DrawSlantedGradientBar(shader, blX, blY, barW, barH, barSlant, sRatio,
                               glm::vec3(0.92f, 0.38f, 0.32f),  // Left: Red/coral
                               glm::vec3(0.98f, 0.78f, 0.28f),  // Mid: Warm amber/orange
                               glm::vec3(1.0f, 0.98f, 0.68f),   // Right: Bright pale yellow
                               glm::vec3(0.06f, 0.07f, 0.12f),  // Depleted background
                               glm::vec3(1.0f, 1.0f, 1.0f),     // Thick white border
                               3.0f);

        // 2. THRUST BAR: Dark Blue to Light Blue gradient with thick white border
        DrawText(shader, "THRUST", blX + barSlant + 4.0f, thrustY - 18.0f, 1.6f, glm::vec3(1.0f, 1.0f, 1.0f), 1.0f);
        float bRatio = std::clamp(boost / maxBoost, 0.0f, 1.0f);
        DrawSlantedGradientBar(shader, blX, thrustY, barW, barH, barSlant, bRatio,
                               glm::vec3(0.18f, 0.12f, 0.62f),  // Left: Dark blue / indigo
                               glm::vec3(0.24f, 0.52f, 0.96f),  // Mid: Electric azure
                               glm::vec3(0.65f, 0.92f, 1.0f),   // Right: Light blue / cyan
                               glm::vec3(0.06f, 0.07f, 0.12f),  // Depleted background
                               glm::vec3(1.0f, 1.0f, 1.0f),     // Thick white border
                               3.0f);

        // 3. THREE DIAMOND SMART BOMBS (Under both bars, no text, radial inner-to-outer gradient)
        for (int i = 0; i < 3; ++i) {
            float dx = blX + 16.0f + (static_cast<float>(i) * 28.0f);
            bool hasBomb = (i < bombCount);
            if (hasBomb) {
                DrawRadialGradientDiamond(shader, dx, diamondY, 9.5f,
                                          glm::vec3(1.0f, 0.98f, 0.65f),  // Inner radiant golden yellow
                                          glm::vec3(0.50f, 0.08f, 0.42f),  // Outer rich magenta/purple
                                          glm::vec3(0.06f, 0.04f, 0.14f),  // Dark border
                                          1.0f, true);
            } else {
                DrawRadialGradientDiamond(shader, dx, diamondY, 9.5f,
                                          glm::vec3(0.1f), glm::vec3(0.05f),
                                          glm::vec3(0.10f, 0.10f, 0.16f),
                                          0.45f, false);
            }
        }
    }

    // =========================================================================
    // 4. BOTTOM-RIGHT: PLANE LIVE COUNTER (Logo x Lives, Images 1, 2, 3)
    // =========================================================================
    if (!isVictory && !isGameOver) {
        float brX = vw - 88.0f;
        float brY = vh - 36.0f;
        DrawStarfighterIcon(shader, brX, brY, 22.0f);
        char livesBuf[16];
        std::snprintf(livesBuf, sizeof(livesBuf), "x %d", lives);
        DrawText(shader, livesBuf, brX + 20.0f, brY - 7.0f, 2.2f, glm::vec3(1.0f, 1.0f, 1.0f), 1.0f);
    }

    // =========================================================================
    // 5. BOSS WARNING EMERGENCY DISPLAY
    // =========================================================================
    if (bossWarning) {
        float warnW = 560.0f;
        float warnH = 80.0f;
        float wx = (vw - warnW) * 0.5f;
        float wy = vh * 0.28f;

        float flash = 0.55f + 0.40f * std::sin(curTime * 16.0f);

        DrawRect(shader, wx, wy, warnW, warnH, glm::vec3(0.55f, 0.05f, 0.05f), flash * 0.85f);
        DrawRectOutline(shader, wx, wy, warnW, warnH, 3.0f, glm::vec3(1.0f, 0.85f, 0.1f), 0.95f);
        DrawRect(shader, wx, wy - 6.0f, warnW, 4.0f, glm::vec3(1.0f, 0.85f, 0.1f), 0.95f);
        DrawRect(shader, wx, wy + warnH + 2.0f, warnW, 4.0f, glm::vec3(1.0f, 0.85f, 0.1f), 0.95f);

        DrawText(shader, "! WARNING: ENEMY DREADNOUGHT DETECTED !", wx + 20.0f, wy + 20.0f, 1.8f,
                 glm::vec3(1.0f, 0.95f, 0.9f), 1.0f);
        DrawText(shader, "ALL WEAPONS AUTHORIZED // PREPARE TO ENGAGE", wx + 40.0f, wy + 48.0f, 1.4f,
                 glm::vec3(1.0f, 0.85f, 0.2f), 0.95f);
    }

    // =========================================================================
    // 6. VICTORY SCREEN (MISSION COMPLETE)
    // =========================================================================
    if (isVictory) {
        float vicW = 580.0f;
        float vicH = 150.0f;
        float vx = (vw - vicW) * 0.5f;
        float vy = (vh - vicH) * 0.5f;

        DrawRect(shader, vx, vy, vicW, vicH, glm::vec3(0.04f, 0.14f, 0.26f), 0.92f);
        DrawRectOutline(shader, vx, vy, vicW, vicH, 3.0f, glm::vec3(0.2f, 0.85f, 1.0f), 0.95f);
        DrawRect(shader, vx, vy - 6.0f, vicW, 4.0f, glm::vec3(1.0f, 0.85f, 0.2f), 0.95f);
        DrawRect(shader, vx, vy + vicH + 2.0f, vicW, 4.0f, glm::vec3(1.0f, 0.85f, 0.2f), 0.95f);

        DrawText(shader, "MISSION COMPLETE", vx + 110.0f, vy + 24.0f, 2.8f, glm::vec3(0.3f, 1.0f, 0.8f), 1.0f);
        DrawText(shader, "SECTOR CANYON CLEARED // THREAT ELIMINATED", vx + 70.0f, vy + 68.0f, 1.5f, glm::vec3(0.85f, 0.95f, 1.0f), 0.95f);

        char finalScoreBuf[32];
        std::snprintf(finalScoreBuf, sizeof(finalScoreBuf), "FINAL SCORE: %06d", score);
        DrawText(shader, finalScoreBuf, vx + 160.0f, vy + 94.0f, 1.8f, glm::vec3(1.0f, 0.9f, 0.25f), 0.95f);

        DrawText(shader, "PRESS [R] OR [SPACE] TO PLAY AGAIN", vx + 105.0f, vy + 122.0f, 1.4f, glm::vec3(0.6f, 0.85f, 1.0f), 0.9f);
    }

    // =========================================================================
    // 7. GAME OVER SCREEN
    // =========================================================================
    if (isGameOver) {
        float goW = 540.0f;
        float goH = 140.0f;
        float gx = (vw - goW) * 0.5f;
        float gy = (vh - goH) * 0.5f;

        DrawRect(shader, gx, gy, goW, goH, glm::vec3(0.25f, 0.05f, 0.05f), 0.92f);
        DrawRectOutline(shader, gx, gy, goW, goH, 3.0f, glm::vec3(0.9f, 0.2f, 0.15f), 0.95f);
        DrawRect(shader, gx, gy - 6.0f, goW, 4.0f, glm::vec3(0.8f, 0.1f, 0.1f), 0.95f);
        DrawRect(shader, gx, gy + goH + 2.0f, goW, 4.0f, glm::vec3(0.8f, 0.1f, 0.1f), 0.95f);

        DrawText(shader, "STARFIGHTER DOWN", gx + 100.0f, gy + 24.0f, 2.8f, glm::vec3(1.0f, 0.2f, 0.2f), 1.0f);
        DrawText(shader, "MISSION FAILED // HULL COMPROMISED", gx + 95.0f, gy + 68.0f, 1.5f, glm::vec3(1.0f, 0.7f, 0.7f), 0.95f);

        char finalScoreBuf[32];
        std::snprintf(finalScoreBuf, sizeof(finalScoreBuf), "FINAL SCORE: %06d", score);
        DrawText(shader, finalScoreBuf, gx + 145.0f, gy + 94.0f, 1.8f, glm::vec3(1.0f, 0.85f, 0.3f), 0.95f);

        DrawText(shader, "PRESS [R] OR [SPACE] TO RETRY", gx + 120.0f, gy + 118.0f, 1.4f, glm::vec3(0.85f, 0.85f, 0.9f), 0.9f);
    }

    // 8. Vector Radio Comms Box (Bottom-Center, Image 2)
    if (!isVictory && !isGameOver) {
        DrawCommsBox(shader, static_cast<int>(vw), static_cast<int>(vh), hasComms, commsSpeaker,
                     commsCallsign, commsLine1, commsLine2, commsColor, commsTimer);
    }

    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
}

void HUD::DrawSecretRelaysHUD(const Shader& shader, int screenWidth, int screenHeight,
                             int destroyedCount, int totalCount) const {
    float w = 210.0f;
    float h = 32.0f;
    float x = static_cast<float>(screenWidth) - w - 28.0f;
    float y = 148.0f;

    DrawRect(shader, x, y, w, h, glm::vec3(0.05f, 0.08f, 0.14f), 0.85f);
    glm::vec3 frameCol = (destroyedCount >= totalCount) ? glm::vec3(1.0f, 0.85f, 0.2f) : glm::vec3(0.2f, 0.5f, 0.75f);
    DrawRectOutline(shader, x, y, w, h, 1.5f, frameCol, 0.85f);

    DrawText(shader, "SECRET RELAYS:", x + 10.0f, y + 10.0f, 1.3f, glm::vec3(0.7f, 0.85f, 1.0f), 0.95f);

    // Draw 3 indicator nodes
    float dotStartX = x + 130.0f;
    for (int i = 0; i < totalCount; ++i) {
        bool destroyed = (i < destroyedCount);
        glm::vec3 dotCol = destroyed ? glm::vec3(1.0f, 0.3f, 0.2f) : glm::vec3(0.2f, 0.85f, 1.0f);
        DrawRect(shader, dotStartX + i * 22.0f, y + 8.0f, 16.0f, 16.0f, dotCol, 0.95f);
        DrawRectOutline(shader, dotStartX + i * 22.0f, y + 8.0f, 16.0f, 16.0f, 1.0f, glm::vec3(1.0f), 0.8f);
    }
}

void HUD::DrawWarpHUD(const Shader& shader, int screenWidth, int screenHeight,
                     float warpTimer, bool hardRoute) const {
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    float vw = GetVirtualWidth(screenWidth, screenHeight);
    float vh = GetVirtualHeight(screenWidth, screenHeight);

    shader.Activate();
    glm::mat4 ortho = glm::ortho(0.0f, vw, vh, 0.0f, -1.0f, 1.0f);
    shader.SetMat4("uProjection", ortho);
    shader.SetMat4("uView", glm::mat4(1.0f));
    shader.SetInt("uUseLighting", 0);
    shader.SetInt("uUseFog", 0);
    shader.SetInt("uUseColorOverride", 1);

    float boxW = 680.0f;
    float boxH = 140.0f;
    float bx = (vw - boxW) * 0.5f;
    float by = (vh - boxH) * 0.5f;

    DrawRect(shader, bx, by, boxW, boxH, glm::vec3(0.02f, 0.05f, 0.12f), 0.92f);
    DrawRectOutline(shader, bx, by, boxW, boxH, 3.0f, glm::vec3(1.0f, 0.85f, 0.2f), 0.95f);

    if (hardRoute) {
        DrawText(shader, "MISSION COMPLETE!", bx + 130.0f, by + 25.0f, 2.8f, glm::vec3(1.0f, 0.88f, 0.25f), 1.0f);
        DrawText(shader, "ALL RADAR ARRAYS DESTROYED // HARD ROUTE UNLOCKED", bx + 65.0f, by + 72.0f, 1.5f, glm::vec3(0.3f, 0.95f, 1.0f), 0.95f);
        DrawText(shader, "ENGAGING HYPERSPACE JUMP -> SECTOR 2: DEEP SPACE", bx + 60.0f, by + 98.0f, 1.5f, glm::vec3(1.0f, 0.6f, 1.0f), 0.95f);
    } else {
        DrawText(shader, "MISSION ACCOMPLISHED!", bx + 80.0f, by + 25.0f, 2.8f, glm::vec3(0.2f, 0.95f, 0.4f), 1.0f);
        DrawText(shader, "COLOSSAL DREADNOUGHT NEUTRALIZED", bx + 155.0f, by + 72.0f, 1.5f, glm::vec3(0.85f, 0.9f, 1.0f), 0.95f);
    }

    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
}

void HUD::DrawTitleScreen(const Shader& shader, int screenWidth, int screenHeight,
                         float time, int selectedMenu) const {
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    float vw = GetVirtualWidth(screenWidth, screenHeight);
    float vh = GetVirtualHeight(screenWidth, screenHeight);

    shader.Activate();
    glm::mat4 ortho = glm::ortho(0.0f, vw, vh, 0.0f, -1.0f, 1.0f);
    shader.SetMat4("uProjection", ortho);
    shader.SetMat4("uView", glm::mat4(1.0f));
    shader.SetInt("uUseLighting", 0);
    shader.SetInt("uUseFog", 0);
    shader.SetInt("uUseColorOverride", 1);

    // 1. Title Banner Top Box
    float tW = 720.0f;
    float tH = 110.0f;
    float tx = (vw - tW) * 0.5f;
    float ty = 40.0f;

    DrawRect(shader, tx, ty, tW, tH, glm::vec3(0.03f, 0.06f, 0.12f), 0.88f);
    DrawRectOutline(shader, tx, ty, tW, tH, 2.5f, glm::vec3(0.2f, 0.7f, 1.0f), 0.95f);
    // Neon corners
    DrawRect(shader, tx - 4.0f, ty - 4.0f, 16.0f, 4.0f, glm::vec3(1.0f, 0.85f, 0.2f), 1.0f);
    DrawRect(shader, tx - 4.0f, ty - 4.0f, 4.0f, 16.0f, glm::vec3(1.0f, 0.85f, 0.2f), 1.0f);
    DrawRect(shader, tx + tW - 12.0f, ty - 4.0f, 16.0f, 4.0f, glm::vec3(1.0f, 0.85f, 0.2f), 1.0f);
    DrawRect(shader, tx + tW, ty - 4.0f, 4.0f, 16.0f, glm::vec3(1.0f, 0.85f, 0.2f), 1.0f);

    float pulse = std::sin(time * 3.5f) * 0.15f + 0.85f;
    DrawText(shader, "AEGIS STARFIGHTER", tx + 85.0f, ty + 22.0f, 3.4f, glm::vec3(0.2f, 0.85f, 1.0f) * pulse, 1.0f);
    DrawText(shader, "ADVANCED 3D TACTICAL RAIL SHOOTER", tx + 155.0f, ty + 72.0f, 1.5f, glm::vec3(1.0f, 0.82f, 0.3f), 0.95f);

    // 2. Menu Options Box (Bottom-Center)
    float mW = 500.0f;
    float mH = 190.0f;
    float mx = (vw - mW) * 0.5f;
    float my = vh - mH - 70.0f;

    DrawRect(shader, mx, my, mW, mH, glm::vec3(0.04f, 0.07f, 0.14f), 0.90f);
    DrawRectOutline(shader, mx, my, mW, mH, 2.0f, glm::vec3(0.25f, 0.55f, 0.85f), 0.90f);

    const char* menuItems[] = {
        "1. LAUNCH SORTIE      [ENTER]",
        "2. FLIGHT SETTINGS    [S]",
        "3. HALL OF FAME       [L]",
        "4. CRT MONITOR SHADER [F1]"
    };

    for (int i = 0; i < 4; ++i) {
        float itemY = my + 24.0f + i * 40.0f;
        bool isSel = (i == selectedMenu);
        if (isSel) {
            DrawRect(shader, mx + 16.0f, itemY - 6.0f, mW - 32.0f, 28.0f, glm::vec3(0.15f, 0.35f, 0.6f), 0.65f);
            DrawRectOutline(shader, mx + 16.0f, itemY - 6.0f, mW - 32.0f, 28.0f, 1.5f, glm::vec3(1.0f, 0.85f, 0.2f), 0.95f);
            DrawText(shader, ">", mx + 24.0f, itemY, 1.8f, glm::vec3(1.0f, 0.85f, 0.2f), 1.0f);
        }
        glm::vec3 itemCol = isSel ? glm::vec3(1.0f, 0.95f, 0.5f) : glm::vec3(0.75f, 0.85f, 0.95f);
        DrawText(shader, menuItems[i], mx + 48.0f, itemY, 1.7f, itemCol, 0.95f);
    }

    // 3. Bottom Squadron Status Bar
    float barY = vh - 34.0f;
    DrawRect(shader, 0.0f, barY, vw, 34.0f, glm::vec3(0.02f, 0.04f, 0.08f), 0.95f);
    DrawText(shader, "SQUADRON: ECHO-1 [STRIKER] & ECHO-2 [AEGIS] ONLINE // HANGAR BAY CLEAR",
             40.0f, barY + 8.0f, 1.4f, glm::vec3(0.3f, 0.85f, 0.5f), 0.95f);

    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
}

void HUD::DrawSettingsMenu(const Shader& shader, int screenWidth, int screenHeight,
                          int selectedIndex, float masterVol, float musicVol, float sfxVol,
                          float currentHudScale,
                          bool invertY, bool crtFilter, bool cockpitDefault) const {
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    float vw = GetVirtualWidth(screenWidth, screenHeight);
    float vh = GetVirtualHeight(screenWidth, screenHeight);

    shader.Activate();
    glm::mat4 ortho = glm::ortho(0.0f, vw, vh, 0.0f, -1.0f, 1.0f);
    shader.SetMat4("uProjection", ortho);
    shader.SetMat4("uView", glm::mat4(1.0f));
    shader.SetInt("uUseLighting", 0);
    shader.SetInt("uUseFog", 0);
    shader.SetInt("uUseColorOverride", 1);

    // Window frame
    float boxW = 700.0f;
    float boxH = 500.0f;
    float bx = (vw - boxW) * 0.5f;
    float by = (vh - boxH) * 0.5f;

    DrawRect(shader, bx, by, boxW, boxH, glm::vec3(0.03f, 0.06f, 0.12f), 0.94f);
    DrawRectOutline(shader, bx, by, boxW, boxH, 2.5f, glm::vec3(0.25f, 0.6f, 0.9f), 0.95f);

    DrawText(shader, "FLIGHT CONFIGURATION & AUDIO", bx + 130.0f, by + 24.0f, 2.4f, glm::vec3(0.2f, 0.85f, 1.0f), 1.0f);
    DrawRect(shader, bx + 24.0f, by + 60.0f, boxW - 48.0f, 2.0f, glm::vec3(0.2f, 0.5f, 0.75f), 0.7f);

    struct SettingRow {
        std::string label;
        std::string value;
        float ratio;
        bool isBar;
    };

    char mBuf[16], bgmBuf[16], sfxBuf[16];
    std::snprintf(mBuf, sizeof(mBuf), "%d%%", static_cast<int>(masterVol * 100.0f + 0.5f));
    std::snprintf(bgmBuf, sizeof(bgmBuf), "%d%%", static_cast<int>(musicVol * 100.0f + 0.5f));
    std::snprintf(sfxBuf, sizeof(sfxBuf), "%d%%", static_cast<int>(sfxVol * 100.0f + 0.5f));

    std::string hudScaleStr = "STANDARD [100%]";
    if (std::abs(currentHudScale - 0.85f) < 0.05f) hudScaleStr = "COMPACT [85%]";
    else if (std::abs(currentHudScale - 1.25f) < 0.05f) hudScaleStr = "LARGE [125%]";
    else if (std::abs(currentHudScale - 1.50f) < 0.05f) hudScaleStr = "MAXIMUM [150%]";

    SettingRow rows[7] = {
        {"MASTER VOLUME", mBuf, masterVol, true},
        {"MUSIC VOLUME", bgmBuf, musicVol, true},
        {"SFX VOLUME", sfxBuf, sfxVol, true},
        {"HUD DISPLAY SCALE", hudScaleStr, 0.0f, false},
        {"INVERT PITCH (Y)", invertY ? "INVERTED [PUSH UP]" : "NORMAL [PULL UP]", 0.0f, false},
        {"RETRO CRT SHADER", crtFilter ? "ENABLED [SCANLINES ON]" : "DISABLED [CLEAN HD]", 0.0f, false},
        {"DEFAULT CAMERA", cockpitDefault ? "1ST-PERSON COCKPIT" : "3RD-PERSON CHASE", 0.0f, false}
    };

    for (int i = 0; i < 7; ++i) {
        float rowY = by + 78.0f + i * 48.0f;
        bool isSel = (i == selectedIndex);

        if (isSel) {
            DrawRect(shader, bx + 20.0f, rowY - 5.0f, boxW - 40.0f, 36.0f, glm::vec3(0.12f, 0.30f, 0.50f), 0.65f);
            DrawRectOutline(shader, bx + 20.0f, rowY - 5.0f, boxW - 40.0f, 36.0f, 1.5f, glm::vec3(1.0f, 0.85f, 0.2f), 0.95f);
            DrawText(shader, ">", bx + 28.0f, rowY + 4.0f, 1.8f, glm::vec3(1.0f, 0.85f, 0.2f), 1.0f);
        }

        glm::vec3 lblCol = isSel ? glm::vec3(1.0f, 0.95f, 0.5f) : glm::vec3(0.8f, 0.9f, 1.0f);
        DrawText(shader, rows[i].label, bx + 50.0f, rowY + 4.0f, 1.5f, lblCol, 0.95f);

        if (rows[i].isBar) {
            DrawSegmentedBar(shader, bx + 320.0f, rowY + 4.0f, 180.0f, 14.0f, 10, rows[i].ratio,
                             glm::vec3(0.2f, 0.85f, 1.0f), glm::vec3(0.1f, 0.15f, 0.2f), glm::vec3(0.3f, 0.5f, 0.7f));
            DrawText(shader, rows[i].value, bx + 520.0f, rowY + 4.0f, 1.5f, glm::vec3(1.0f, 0.85f, 0.3f), 0.95f);
        } else {
            glm::vec3 valCol = isSel ? glm::vec3(0.3f, 0.95f, 0.5f) : glm::vec3(0.7f, 0.85f, 0.95f);
            DrawText(shader, rows[i].value, bx + 320.0f, rowY + 4.0f, 1.5f, valCol, 0.95f);
        }
    }

    // Bottom guidance
    DrawText(shader, "[UP/DOWN] SELECT  [LEFT/RIGHT] ADJUST  [ESC/ENTER] BACK",
             bx + 65.0f, by + boxH - 28.0f, 1.5f, glm::vec3(0.6f, 0.8f, 1.0f), 0.90f);

    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
}

void HUD::DrawLeaderboard(const Shader& shader, int screenWidth, int screenHeight,
                         const std::vector<HighScoreEntry>& scores) const {
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    float vw = GetVirtualWidth(screenWidth, screenHeight);
    float vh = GetVirtualHeight(screenWidth, screenHeight);

    shader.Activate();
    glm::mat4 ortho = glm::ortho(0.0f, vw, vh, 0.0f, -1.0f, 1.0f);
    shader.SetMat4("uProjection", ortho);
    shader.SetMat4("uView", glm::mat4(1.0f));
    shader.SetInt("uUseLighting", 0);
    shader.SetInt("uUseFog", 0);
    shader.SetInt("uUseColorOverride", 1);

    float boxW = 720.0f;
    float boxH = 460.0f;
    float bx = (vw - boxW) * 0.5f;
    float by = (vh - boxH) * 0.5f;

    DrawRect(shader, bx, by, boxW, boxH, glm::vec3(0.03f, 0.06f, 0.12f), 0.94f);
    DrawRectOutline(shader, bx, by, boxW, boxH, 2.5f, glm::vec3(1.0f, 0.85f, 0.2f), 0.95f);

    DrawText(shader, "HALL OF FAME - TOP ACE PILOTS", bx + 120.0f, by + 24.0f, 2.4f, glm::vec3(1.0f, 0.85f, 0.2f), 1.0f);
    DrawRect(shader, bx + 24.0f, by + 60.0f, boxW - 48.0f, 2.0f, glm::vec3(0.3f, 0.5f, 0.75f), 0.7f);

    // Table Header
    DrawText(shader, "RANK", bx + 50.0f, by + 80.0f, 1.6f, glm::vec3(0.5f, 0.8f, 1.0f), 0.9f);
    DrawText(shader, "PILOT", bx + 140.0f, by + 80.0f, 1.6f, glm::vec3(0.5f, 0.8f, 1.0f), 0.9f);
    DrawText(shader, "SCORE", bx + 280.0f, by + 80.0f, 1.6f, glm::vec3(0.5f, 0.8f, 1.0f), 0.9f);
    DrawText(shader, "STAGE", bx + 420.0f, by + 80.0f, 1.6f, glm::vec3(0.5f, 0.8f, 1.0f), 0.9f);
    DrawText(shader, "OUTCOME", bx + 530.0f, by + 80.0f, 1.6f, glm::vec3(0.5f, 0.8f, 1.0f), 0.9f);

    DrawRect(shader, bx + 24.0f, by + 104.0f, boxW - 48.0f, 1.0f, glm::vec3(0.25f, 0.45f, 0.65f), 0.6f);

    for (size_t i = 0; i < scores.size() && i < 5; ++i) {
        float rowY = by + 124.0f + i * 50.0f;
        char rankBuf[8];
        std::snprintf(rankBuf, sizeof(rankBuf), "#%d", static_cast<int>(i + 1));
        char scoreBuf[16];
        std::snprintf(scoreBuf, sizeof(scoreBuf), "%06d", scores[i].score);
        std::string stageStr = (scores[i].stageReached >= 2) ? "SECTOR 2" : "SECTOR 1";
        std::string routeStr = scores[i].missionComplete ? "COMPLETE [ACE]" : "ACCOMPLISHED";

        glm::vec3 rowCol = (i == 0) ? glm::vec3(1.0f, 0.85f, 0.2f) : glm::vec3(0.85f, 0.92f, 1.0f);

        DrawText(shader, rankBuf, bx + 50.0f, rowY, 1.6f, rowCol, 0.95f);
        DrawText(shader, scores[i].name, bx + 140.0f, rowY, 1.6f, rowCol, 0.95f);
        DrawText(shader, scoreBuf, bx + 280.0f, rowY, 1.6f, glm::vec3(0.2f, 0.85f, 1.0f), 0.95f);
        DrawText(shader, stageStr, bx + 420.0f, rowY, 1.5f, glm::vec3(0.7f, 0.85f, 0.95f), 0.90f);
        DrawText(shader, routeStr, bx + 530.0f, rowY, 1.4f, scores[i].missionComplete ? glm::vec3(1.0f, 0.85f, 0.2f) : glm::vec3(0.3f, 0.85f, 0.4f), 0.95f);
    }

    DrawText(shader, "PRESS [ESC] OR [SPACE] TO RETURN TO HANGAR",
             bx + 140.0f, by + boxH - 34.0f, 1.5f, glm::vec3(0.7f, 0.85f, 1.0f), 0.90f);

    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
}

void HUD::DrawMissionBriefing(const Shader& shader, int screenWidth, int screenHeight, float time) const {
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    float vw = GetVirtualWidth(screenWidth, screenHeight);
    float vh = GetVirtualHeight(screenWidth, screenHeight);

    shader.Activate();
    glm::mat4 ortho = glm::ortho(0.0f, vw, vh, 0.0f, -1.0f, 1.0f);
    shader.SetMat4("uProjection", ortho);
    shader.SetMat4("uView", glm::mat4(1.0f));
    shader.SetInt("uUseLighting", 0);
    shader.SetInt("uUseFog", 0);
    shader.SetInt("uUseColorOverride", 1);

    float boxW = 740.0f;
    float boxH = 430.0f;
    float bx = (vw - boxW) * 0.5f;
    float by = (vh - boxH) * 0.5f;

    DrawRect(shader, bx, by, boxW, boxH, glm::vec3(0.02f, 0.05f, 0.10f), 0.94f);
    DrawRectOutline(shader, bx, by, boxW, boxH, 2.5f, glm::vec3(0.2f, 0.85f, 0.5f), 0.95f);

    DrawText(shader, "TACTICAL SORTIE BRIEFING // OP CANYON STRIKE", bx + 70.0f, by + 24.0f, 2.2f, glm::vec3(0.2f, 0.95f, 0.5f), 1.0f);
    DrawRect(shader, bx + 24.0f, by + 58.0f, boxW - 48.0f, 2.0f, glm::vec3(0.2f, 0.6f, 0.4f), 0.7f);

    DrawText(shader, "TARGET ZONE: SECTOR 1 - CANYON TRENCH", bx + 40.0f, by + 85.0f, 1.7f, glm::vec3(1.0f, 0.85f, 0.25f), 0.95f);
    DrawText(shader, "The hostile war fleet has deployed a Colossal Dreadnought flagship", bx + 40.0f, by + 115.0f, 1.4f, glm::vec3(0.85f, 0.95f, 1.0f), 0.90f);
    DrawText(shader, "deep inside the planetary canyon corridor.", bx + 40.0f, by + 138.0f, 1.4f, glm::vec3(0.85f, 0.95f, 1.0f), 0.90f);

    DrawText(shader, "PRIMARY OBJECTIVE:", bx + 40.0f, by + 180.0f, 1.6f, glm::vec3(0.2f, 0.85f, 1.0f), 0.95f);
    DrawText(shader, "- Infiltrate trench, eliminate drone squadrons, and destroy Dreadnought.", bx + 60.0f, by + 208.0f, 1.4f, glm::vec3(0.85f, 0.95f, 1.0f), 0.90f);

    DrawText(shader, "SECRET DIRECTIVE [HARD ROUTE]:", bx + 40.0f, by + 248.0f, 1.6f, glm::vec3(1.0f, 0.4f, 0.2f), 0.95f);
    DrawText(shader, "- Locate and destroy all 3 hidden Planetary Radar Relays inside archways.", bx + 60.0f, by + 276.0f, 1.4f, glm::vec3(1.0f, 0.8f, 0.7f), 0.90f);
    DrawText(shader, "- Keep both Echo wingmen intact to unlock Hyperspace Warp to SECTOR 2!", bx + 60.0f, by + 298.0f, 1.4f, glm::vec3(1.0f, 0.85f, 0.2f), 0.95f);

    float pulse = std::sin(time * 4.0f) * 0.25f + 0.75f;
    DrawText(shader, "PRESS [ENTER] OR [SPACE] TO LAUNCH SORTIE", bx + 130.0f, by + boxH - 40.0f, 1.8f, glm::vec3(0.2f, 0.95f, 0.5f) * pulse, 1.0f);

    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
}
