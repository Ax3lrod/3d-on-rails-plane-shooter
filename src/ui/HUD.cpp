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

HUD::HUD() : quadMesh(CreateScreenQuad()) {}

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
    float curX = x;
    float charAdvance = 6.0f * scale;

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
    float boxX = 28.0f;
    float boxY = static_cast<float>(screenHeight) - boxH - 24.0f;

    // 1. Semi-transparent dark tactical backdrop
    DrawRect(shader, boxX, boxY, boxW, boxH, glm::vec3(0.04f, 0.07f, 0.12f), 0.88f);
    DrawRectOutline(shader, boxX, boxY, boxW, boxH, 1.5f, themeColor, 0.90f);

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
                 float commsTimer) const {
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    shader.Activate();
    glm::mat4 ortho = glm::ortho(0.0f, static_cast<float>(screenWidth),
                                 static_cast<float>(screenHeight), 0.0f, -1.0f, 1.0f);
    shader.SetMat4("uProjection", ortho);
    shader.SetMat4("uView", glm::mat4(1.0f));

    // CRITICAL: Disable lighting and fog for 2D UI elements
    shader.SetInt("uUseLighting", 0);
    shader.SetInt("uUseFog", 0);
    shader.SetInt("uUseColorOverride", 1);

    float curTime = static_cast<float>(glfwGetTime());

    // 0. Draw First-Person Cockpit Canopy Overlay
    if (isFirstPerson && !isVictory && !isGameOver) {
        DrawCockpitCanopyOverlay(shader, screenWidth, screenHeight, playerPitch, playerRoll);
    }

    // =========================================================================
    // 1. TOP-LEFT CLUSTER: PILOT FLIGHT SYSTEMS
    // =========================================================================
    float pX = 28.0f;
    float pY = 24.0f;
    float pW = 280.0f;
    float pH = 142.0f;

    // Tactical HUD Panel Backplate with cybernetic corner accents
    DrawRect(shader, pX, pY, pW, pH, glm::vec3(0.05f, 0.08f, 0.14f), 0.75f);
    DrawRectOutline(shader, pX, pY, pW, pH, 1.5f, glm::vec3(0.2f, 0.45f, 0.7f), 0.65f);
    // Tech corner ticks
    DrawRect(shader, pX, pY, 14.0f, 3.0f, glm::vec3(0.3f, 0.8f, 1.0f), 0.9f);
    DrawRect(shader, pX, pY, 3.0f, 14.0f, glm::vec3(0.3f, 0.8f, 1.0f), 0.9f);
    DrawRect(shader, pX + pW - 14.0f, pY, 14.0f, 3.0f, glm::vec3(0.3f, 0.8f, 1.0f), 0.9f);
    DrawRect(shader, pX + pW - 3.0f, pY, 3.0f, 14.0f, glm::vec3(0.3f, 0.8f, 1.0f), 0.9f);

    // --- Shield Meter ---
    float shieldRatio = std::clamp(shield / maxShield, 0.0f, 1.0f);
    glm::vec3 shieldColor(0.2f, 0.95f, 0.35f);
    std::string shieldStatus = "100%";
    int shieldPct = static_cast<int>(shieldRatio * 100.0f + 0.5f);
    char shieldBuf[16];
    std::snprintf(shieldBuf, sizeof(shieldBuf), "%d%%", shieldPct);
    shieldStatus = shieldBuf;

    if (shieldRatio < 0.28f) {
        shieldColor = (std::sin(curTime * 14.0f) > 0.0f) ? glm::vec3(1.0f, 0.15f, 0.15f) : glm::vec3(0.4f, 0.05f, 0.05f);
        shieldStatus = "DANGER";
    } else if (shieldRatio < 0.55f) {
        shieldColor = glm::vec3(1.0f, 0.85f, 0.15f);
    }

    DrawText(shader, "SHIELD", pX + 12.0f, pY + 12.0f, 1.6f, glm::vec3(0.8f, 0.95f, 1.0f), 0.95f);
    DrawText(shader, shieldStatus, pX + pW - 75.0f, pY + 12.0f, 1.6f, shieldColor, 0.95f);

    DrawSegmentedBar(shader, pX + 12.0f, pY + 28.0f, pW - 24.0f, 12.0f, 12, shieldRatio,
                     shieldColor, glm::vec3(0.12f, 0.16f, 0.22f), glm::vec3(0.3f, 0.5f, 0.7f));

    // --- Boost / Engine Meter ---
    float boostRatio = std::clamp(boost / maxBoost, 0.0f, 1.0f);
    glm::vec3 boostColor = glm::vec3(0.2f, 0.82f, 1.0f);
    std::string boostStatus = "READY";

    if (isOverheated) {
        boostColor = (std::sin(curTime * 14.0f) > 0.0f) ? glm::vec3(1.0f, 0.3f, 0.1f) : glm::vec3(0.4f, 0.15f, 0.05f);
        boostStatus = "OVERHEAT";
    }

    DrawText(shader, "BOOST", pX + 12.0f, pY + 48.0f, 1.5f, glm::vec3(0.7f, 0.9f, 1.0f), 0.95f);
    DrawText(shader, boostStatus, pX + pW - 85.0f, pY + 48.0f, 1.5f, boostColor, 0.95f);

    DrawSegmentedBar(shader, pX + 12.0f, pY + 62.0f, pW - 24.0f, 9.0f, 12, boostRatio,
                     boostColor, glm::vec3(0.10f, 0.14f, 0.20f), glm::vec3(0.25f, 0.4f, 0.65f));

    // --- Ordnance status / Smart Bomb count ---
    DrawText(shader, "BOMBS:", pX + 12.0f, pY + 80.0f, 1.4f, glm::vec3(0.9f, 0.85f, 0.3f), 0.95f);
    float bombStartX = pX + 75.0f;
    for (int i = 0; i < 5; ++i) {
        bool hasBomb = (i < bombCount);
        glm::vec3 bCol = hasBomb ? glm::vec3(0.2f, 0.85f, 1.0f) : glm::vec3(0.15f, 0.18f, 0.25f);
        float bAlpha = hasBomb ? 0.95f : 0.35f;

        // Torpedo Icon
        DrawRect(shader, bombStartX + i * 16.0f, pY + 80.0f, 11.0f, 11.0f, bCol, bAlpha);
        DrawRectOutline(shader, bombStartX + i * 16.0f, pY + 80.0f, 11.0f, 11.0f, 1.0f, glm::vec3(1.0f), hasBomb ? 0.8f : 0.2f);
    }

    // --- Wing Integrity Readout ---
    float lWingRatio = std::clamp(leftWingHealth / 100.0f, 0.0f, 1.0f);
    float rWingRatio = std::clamp(rightWingHealth / 100.0f, 0.0f, 1.0f);

    glm::vec3 lWingColor = leftWingLost ? ((std::sin(curTime * 12.0f) > 0.0f) ? glm::vec3(1.0f, 0.2f, 0.2f) : glm::vec3(0.4f, 0.05f, 0.05f))
                                        : ((lWingRatio < 0.6f) ? glm::vec3(1.0f, 0.85f, 0.2f) : glm::vec3(0.2f, 0.95f, 0.35f));
    glm::vec3 rWingColor = rightWingLost ? ((std::sin(curTime * 12.0f) > 0.0f) ? glm::vec3(1.0f, 0.2f, 0.2f) : glm::vec3(0.4f, 0.05f, 0.05f))
                                         : ((rWingRatio < 0.6f) ? glm::vec3(1.0f, 0.85f, 0.2f) : glm::vec3(0.2f, 0.95f, 0.35f));

    std::string lWingText = leftWingLost ? "L-WING LOST" : (lWingRatio < 0.6f ? "L-WING DMG" : "L-WING OK");
    std::string rWingText = rightWingLost ? "R-WING LOST" : (rWingRatio < 0.6f ? "R-WING DMG" : "R-WING OK");

    DrawText(shader, lWingText, pX + 12.0f, pY + 102.0f, 1.3f, lWingColor, 0.95f);
    DrawText(shader, rWingText, pX + 148.0f, pY + 102.0f, 1.3f, rWingColor, 0.95f);

    DrawSegmentedBar(shader, pX + 12.0f, pY + 118.0f, 116.0f, 7.0f, 6, leftWingLost ? 0.0f : lWingRatio,
                     lWingColor, glm::vec3(0.12f, 0.15f, 0.22f), glm::vec3(0.25f, 0.45f, 0.65f));
    DrawSegmentedBar(shader, pX + 148.0f, pY + 118.0f, 116.0f, 7.0f, 6, rightWingLost ? 0.0f : rWingRatio,
                     rWingColor, glm::vec3(0.12f, 0.15f, 0.22f), glm::vec3(0.25f, 0.45f, 0.65f));

    // --- Charge Shot Meter (Appears dynamically below panel) ---
    if (chargeProgress > 0.05f) {
        float cY = pY + pH + 8.0f;
        DrawRect(shader, pX, cY, pW, 28.0f, glm::vec3(0.05f, 0.10f, 0.16f), 0.85f);
        DrawRectOutline(shader, pX, cY, pW, 28.0f, 1.2f, glm::vec3(0.2f, 0.75f, 0.9f), 0.8f);

        bool isFull = (chargeProgress >= 0.75f);
        glm::vec3 chargeCol = isFull ? glm::vec3(0.2f, 1.0f, 0.8f) : glm::vec3(0.3f, 0.8f, 0.4f);
        std::string cLabel = isFull ? "LOCK-ON READY" : "CHARGING PLASMA";

        DrawText(shader, cLabel, pX + 10.0f, cY + 5.0f, 1.3f, chargeCol, 0.95f);
        DrawSegmentedBar(shader, pX + 10.0f, cY + 16.0f, pW - 20.0f, 6.0f, 10, chargeProgress,
                         chargeCol, glm::vec3(0.1f, 0.15f, 0.2f), glm::vec3(0.2f, 0.6f, 0.8f));
    }

    // --- Barrel Roll Deflect Badge ---
    if (isDeflecting) {
        float defY = (chargeProgress > 0.05f) ? (pY + pH + 42.0f) : (pY + pH + 8.0f);
        DrawRect(shader, pX, defY, 190.0f, 24.0f, glm::vec3(0.08f, 0.35f, 0.25f), 0.85f);
        DrawRectOutline(shader, pX, defY, 190.0f, 24.0f, 1.5f, glm::vec3(0.2f, 1.0f, 0.7f), 0.95f);
        DrawText(shader, "[ DEFLECT ACTIVE ]", pX + 8.0f, defY + 7.0f, 1.4f, glm::vec3(0.3f, 1.0f, 0.8f), 1.0f);
    }

    // =========================================================================
    // 2. TOP-RIGHT CLUSTER: TACTICAL MISSION STATUS
    // =========================================================================
    float rW = 240.0f;
    float rH = 82.0f;
    float rX = screenWidth - rW - 28.0f;
    float rY = 24.0f;

    DrawRect(shader, rX, rY, rW, rH, glm::vec3(0.05f, 0.08f, 0.14f), 0.75f);
    DrawRectOutline(shader, rX, rY, rW, rH, 1.5f, glm::vec3(0.2f, 0.45f, 0.7f), 0.65f);
    DrawRect(shader, rX + rW - 14.0f, rY, 14.0f, 3.0f, glm::vec3(0.3f, 0.8f, 1.0f), 0.9f);
    DrawRect(shader, rX + rW - 3.0f, rY, 3.0f, 14.0f, glm::vec3(0.3f, 0.8f, 1.0f), 0.9f);

    // Score Readout
    char scoreBuf[32];
    std::snprintf(scoreBuf, sizeof(scoreBuf), "SCORE: %06d", score);
    DrawText(shader, scoreBuf, rX + 14.0f, rY + 14.0f, 1.8f, glm::vec3(1.0f, 0.9f, 0.25f), 0.95f);

    // Rings Readout
    char ringBuf[32];
    std::snprintf(ringBuf, sizeof(ringBuf), "RINGS: %02d", rings);
    DrawText(shader, ringBuf, rX + 14.0f, rY + 38.0f, 1.6f, glm::vec3(0.3f, 0.85f, 1.0f), 0.95f);

    // Stage Zone
    DrawText(shader, "SECTOR: CANYON 01", rX + 14.0f, rY + 58.0f, 1.3f, glm::vec3(0.5f, 0.7f, 0.9f), 0.8f);

    // =========================================================================
    // 3. TOP-CENTER CLUSTER: BOSS DREADNOUGHT STATUS
    // =========================================================================
    if (bossActive) {
        float bW = 460.0f;
        float bH = 54.0f;
        float bX = (screenWidth - bW) * 0.5f;
        float bY = 20.0f;

        // Boss plate
        DrawRect(shader, bX, bY, bW, bH, glm::vec3(0.12f, 0.04f, 0.05f), 0.85f);
        DrawRectOutline(shader, bX, bY, bW, bH, 1.5f, glm::vec3(0.85f, 0.2f, 0.15f), 0.9f);

        // Header Title
        std::string bossTitle = bossCoreExposed ? "GOLIATH // CORE OVERHEATED" : "GOLIATH // TITAN DREADNOUGHT";
        glm::vec3 bTitleCol = bossCoreExposed ? glm::vec3(1.0f, 0.2f, 0.1f) : glm::vec3(1.0f, 0.8f, 0.2f);
        DrawText(shader, bossTitle, bX + 16.0f, bY + 8.0f, 1.5f, bTitleCol, 0.95f);

        // Boss Segmented Health Bar
        glm::vec3 bossFillCol = bossCoreExposed ? glm::vec3(1.0f, 0.15f, 0.15f) : glm::vec3(0.95f, 0.55f, 0.15f);
        DrawSegmentedBar(shader, bX + 16.0f, bY + 22.0f, bW - 32.0f, 12.0f, 20, bossHealthRatio,
                         bossFillCol, glm::vec3(0.2f, 0.08f, 0.08f), glm::vec3(0.8f, 0.3f, 0.2f));

        // Subsystem indicator badges
        float pipW = 98.0f;
        float pipGap = 8.0f;
        float pipStartX = bX + 16.0f;
        float pipY = bY + 38.0f;

        // Pip 1: L-Turret
        glm::vec3 lCol = bossLeftTurretDown ? glm::vec3(0.5f, 0.15f, 0.15f) : glm::vec3(0.2f, 0.9f, 0.4f);
        DrawText(shader, bossLeftTurretDown ? "[L-TUR DOWN]" : "[L-TURRET OK]", pipStartX, pipY, 1.1f, lCol, 0.95f);

        // Pip 2: Shield
        glm::vec3 sCol = bossShieldDown ? glm::vec3(0.5f, 0.15f, 0.15f) : glm::vec3(0.2f, 0.85f, 1.0f);
        DrawText(shader, bossShieldDown ? "[SHIELD DOWN]" : "[SHIELD ON]", pipStartX + pipW + pipGap, pipY, 1.1f, sCol, 0.95f);

        // Pip 3: R-Turret
        glm::vec3 rCol = bossRightTurretDown ? glm::vec3(0.5f, 0.15f, 0.15f) : glm::vec3(0.2f, 0.9f, 0.4f);
        DrawText(shader, bossRightTurretDown ? "[R-TUR DOWN]" : "[R-TURRET OK]", pipStartX + (pipW + pipGap) * 2.0f, pipY, 1.1f, rCol, 0.95f);

        // Pip 4: Core
        glm::vec3 cCol = bossCoreExposed ? glm::vec3(1.0f, 0.35f, 0.1f) : glm::vec3(0.4f, 0.45f, 0.5f);
        DrawText(shader, bossCoreExposed ? "[CORE VULN]" : "[CORE SHIELDED]", pipStartX + (pipW + pipGap) * 3.0f, pipY, 1.1f, cCol, 0.95f);
    }

    // =========================================================================
    // 3.5. WING DAMAGE / REPAIR ALERT BANNER
    // =========================================================================
    if (wingAlertTimer > 0.0f && !wingAlertMsg.empty()) {
        float bW = 460.0f;
        float bH = 46.0f;
        float bx = (screenWidth - bW) * 0.5f;
        float by = screenHeight * 0.20f;

        bool isLost = (wingAlertMsg.find("DESTROYED") != std::string::npos);
        float flash = 0.6f + 0.4f * std::sin(curTime * 14.0f);

        glm::vec3 bgCol = isLost ? glm::vec3(0.5f, 0.08f, 0.08f) : glm::vec3(0.06f, 0.28f, 0.22f);
        glm::vec3 borderCol = isLost ? glm::vec3(1.0f, 0.3f, 0.2f) : glm::vec3(0.2f, 1.0f, 0.7f);

        DrawRect(shader, bx, by, bW, bH, bgCol, flash * 0.9f);
        DrawRectOutline(shader, bx, by, bW, bH, 2.0f, borderCol, 0.95f);
        DrawRect(shader, bx, by - 3.0f, bW, 2.0f, borderCol, 0.9f);
        DrawRect(shader, bx, by + bH + 1.0f, bW, 2.0f, borderCol, 0.9f);

        DrawText(shader, wingAlertMsg, bx + 28.0f, by + 14.0f, 1.6f, glm::vec3(1.0f, 0.95f, 0.95f), 1.0f);
    }

    // =========================================================================
    // 4. BOSS WARNING EMERGENCY DISPLAY
    // =========================================================================
    if (bossWarning) {
        float warnW = 560.0f;
        float warnH = 80.0f;
        float wx = (screenWidth - warnW) * 0.5f;
        float wy = screenHeight * 0.28f;

        float flash = 0.55f + 0.40f * std::sin(curTime * 16.0f);

        // Backdrop
        DrawRect(shader, wx, wy, warnW, warnH, glm::vec3(0.55f, 0.05f, 0.05f), flash * 0.85f);
        DrawRectOutline(shader, wx, wy, warnW, warnH, 3.0f, glm::vec3(1.0f, 0.85f, 0.1f), 0.95f);

        // Hazard borders
        DrawRect(shader, wx, wy - 6.0f, warnW, 4.0f, glm::vec3(1.0f, 0.85f, 0.1f), 0.95f);
        DrawRect(shader, wx, wy + warnH + 2.0f, warnW, 4.0f, glm::vec3(1.0f, 0.85f, 0.1f), 0.95f);

        // Bold warning text
        DrawText(shader, "! WARNING: ENEMY DREADNOUGHT DETECTED !", wx + 20.0f, wy + 20.0f, 1.8f,
                 glm::vec3(1.0f, 0.95f, 0.9f), 1.0f);
        DrawText(shader, "ALL WEAPONS AUTHORIZED // PREPARE TO ENGAGE", wx + 40.0f, wy + 48.0f, 1.4f,
                 glm::vec3(1.0f, 0.85f, 0.2f), 0.95f);
    }

    // =========================================================================
    // 5. VICTORY SCREEN (MISSION COMPLETE)
    // =========================================================================
    if (isVictory) {
        float vicW = 580.0f;
        float vicH = 150.0f;
        float vx = (screenWidth - vicW) * 0.5f;
        float vy = (screenHeight - vicH) * 0.5f;

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
    // 6. GAME OVER SCREEN
    // =========================================================================
    if (isGameOver) {
        float goW = 540.0f;
        float goH = 140.0f;
        float gx = (screenWidth - goW) * 0.5f;
        float gy = (screenHeight - goH) * 0.5f;

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

    // 7. Tactical 360-Degree Minimap (Bottom-Right)
    if (!isVictory && !isGameOver) {
        float rRadius = 65.0f;
        float rx = static_cast<float>(screenWidth) - rRadius - 28.0f;
        float ry = static_cast<float>(screenHeight) - rRadius - 28.0f;
        DrawRadarMinimap(shader, rx, ry, rRadius, playerPos, playerYaw, bossPos, bossActive, enemyPositions);
    }

    // 8. Vector Radio Comms Box (Bottom-Left)
    if (!isVictory && !isGameOver) {
        DrawCommsBox(shader, screenWidth, screenHeight, hasComms, commsSpeaker,
                     commsCallsign, commsLine1, commsLine2, commsColor, commsTimer);
    }

    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
}
