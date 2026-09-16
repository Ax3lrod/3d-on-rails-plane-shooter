#ifndef MESH_H
#define MESH_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>
#include "VAO.h"
#include "VBO.h"
#include "EBO.h"
#include "shaderClass.h"

struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec3 color;
};

class Mesh {
public:
    std::vector<Vertex> vertices;
    std::vector<GLuint> indices;
    VAO vao;
    VBO vbo;
    EBO ebo;

    Mesh();
    Mesh(const std::vector<Vertex>& verts, const std::vector<GLuint>& inds);
    
    // Disable copy semantics to prevent double deletion of GL buffers, allow move
    Mesh(const Mesh&) = delete;
    Mesh& operator=(const Mesh&) = delete;
    Mesh(Mesh&& other) noexcept;
    Mesh& operator=(Mesh&& other) noexcept;
    ~Mesh();

    void SetupMesh();
    void Draw(const Shader& shader, GLenum mode = GL_TRIANGLES) const;
    void Clear();

    // Procedural geometry builders for retro 3D rail-shooter aesthetics
    static Mesh CreateCube(const glm::vec3& size, const glm::vec3& color);
    static Mesh CreatePyramid(float width, float height, float length, const glm::vec3& color);
    static Mesh CreateRing(float innerRadius, float outerRadius, int segments, const glm::vec3& color);
    static Mesh CreateCrosshair(float size, const glm::vec3& color);
    static Mesh CreateStarfighter(const glm::vec3& bodyColor, const glm::vec3& wingColor, const glm::vec3& cockpitColor);
    static Mesh CreateStarfighterFuselage(const glm::vec3& bodyColor, const glm::vec3& cockpitColor);
    static Mesh CreateStarfighterLeftWing(const glm::vec3& wingColor);
    static Mesh CreateStarfighterRightWing(const glm::vec3& wingColor);
    static Mesh CreateEnemyDrone(const glm::vec3& bodyColor, const glm::vec3& eyeColor);
    static Mesh CreateAsteroid(float radius, const glm::vec3& color);
    static Mesh CreateLaserBolt(float width, float length, const glm::vec3& color);
    static Mesh CreateSphere(float radius, int rings, int sectors, const glm::vec3& color);
    static Mesh CreateShockwave(float radius, float thickness, int segments, const glm::vec3& color);
    static Mesh CreateLockOnDiamond(float size, const glm::vec3& color);
    static Mesh CreateCanyonSection(float length, float width, float wallHeight,
                                   const glm::vec3& floorColA, const glm::vec3& floorColB,
                                   const glm::vec3& wallCol);
    static Mesh CreateArcadeHorizon(float radius, float height,
                                   const glm::vec3& skyTopCol, const glm::vec3& horizonCol,
                                   const glm::vec3& mountainCol);
    static Mesh CreateRockArch(float spanWidth, float height, float thickness, const glm::vec3& color);
    static Mesh CreatePillar(float radius, float height, const glm::vec3& color);
    static Mesh CreateBossHull(const glm::vec3& mainColor, const glm::vec3& accentColor);
    static Mesh CreateBossTurret(const glm::vec3& turretColor);
    static Mesh CreateBossCore(const glm::vec3& coreColor);
    static Mesh CreateRadarRelay(float size, const glm::vec3& baseColor, const glm::vec3& dishColor, const glm::vec3& beaconColor);
    static Mesh CreateSpaceDebris(float length, float width, const glm::vec3& color);
    static Mesh CreateShadowDisc(float radius, int segments = 16);
};

#endif
