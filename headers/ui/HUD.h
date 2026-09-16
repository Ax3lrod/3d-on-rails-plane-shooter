#ifndef HUD_H
#define HUD_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include "Mesh.h"
#include "shaderClass.h"

class HUD {
public:
    Mesh quadMesh;
    Mesh barBgMesh;
    Mesh bombIconMesh;

    HUD();

    void DrawBar(const Shader& shader, float x, float y, float w, float h, float fillRatio,
                 const glm::vec3& fillColor, const glm::vec3& bgColor) const;

    void Render(const Shader& shader, int screenWidth, int screenHeight,
                float shield, float maxShield,
                float boost, float maxBoost, bool isOverheated,
                bool isDeflecting, int score, int rings,
                int bombCount, float chargeProgress, bool isGameOver) const;
};

#endif
