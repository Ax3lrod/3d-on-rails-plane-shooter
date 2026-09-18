#include "Mesh.h"
#include <cmath>
#include <glm/gtc/constants.hpp>

Mesh::Mesh() : vao(), vbo(), ebo() {}

Mesh::Mesh(const std::vector<Vertex>& verts, const std::vector<GLuint>& inds)
    : vertices(verts), indices(inds) {
    SetupMesh();
}

Mesh::Mesh(Mesh&& other) noexcept
    : vertices(std::move(other.vertices)),
      indices(std::move(other.indices)),
      vao(std::move(other.vao)),
      vbo(std::move(other.vbo)),
      ebo(std::move(other.ebo)) {
    other.vao.ID = 0;
    other.vbo.ID = 0;
    other.ebo.ID = 0;
}

Mesh& Mesh::operator=(Mesh&& other) noexcept {
    if (this != &other) {
        Clear();
        vertices = std::move(other.vertices);
        indices = std::move(other.indices);
        vao = std::move(other.vao);
        vbo = std::move(other.vbo);
        ebo = std::move(other.ebo);
        other.vao.ID = 0;
        other.vbo.ID = 0;
        other.ebo.ID = 0;
    }
    return *this;
}

Mesh::~Mesh() {
    Clear();
}

void Mesh::Clear() {
    vao.Delete();
    vbo.Delete();
    ebo.Delete();
}

void Mesh::SetupMesh() {
    if (vertices.empty()) return;

    vao.Bind();
    vbo.BufferData(vertices.data(), vertices.size() * sizeof(Vertex), GL_STATIC_DRAW);

    if (!indices.empty()) {
        ebo.BufferData(indices.data(), indices.size() * sizeof(GLuint), GL_STATIC_DRAW);
    }

    GLsizei stride = sizeof(Vertex);

    // Attribute 0: Position (vec3)
    vao.LinkAttrib(vbo, 0, 3, GL_FLOAT, stride, (void*)offsetof(Vertex, position));

    // Attribute 1: Normal (vec3)
    vao.LinkAttrib(vbo, 1, 3, GL_FLOAT, stride, (void*)offsetof(Vertex, normal));

    // Attribute 2: Color (vec3)
    vao.LinkAttrib(vbo, 2, 3, GL_FLOAT, stride, (void*)offsetof(Vertex, color));

    vao.Unbind();
    vbo.Unbind();
    if (!indices.empty()) {
        ebo.Unbind();
    }
}

void Mesh::Draw(const Shader& shader, GLenum mode) const {
    shader.Activate();
    const_cast<VAO&>(vao).Bind();

    if (!indices.empty()) {
        glDrawElements(mode, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_INT, 0);
    } else {
        glDrawArrays(mode, 0, static_cast<GLsizei>(vertices.size()));
    }

    const_cast<VAO&>(vao).Unbind();
}

// Helper to append a flat shaded triangle with computed face normal
static void AddTriangle(std::vector<Vertex>& verts, std::vector<GLuint>& inds,
                        const glm::vec3& p0, const glm::vec3& p1, const glm::vec3& p2,
                        const glm::vec3& color) {
    glm::vec3 normal = glm::normalize(glm::cross(p1 - p0, p2 - p0));
    GLuint startIndex = static_cast<GLuint>(verts.size());
    verts.push_back({p0, normal, color});
    verts.push_back({p1, normal, color});
    verts.push_back({p2, normal, color});
    inds.push_back(startIndex);
    inds.push_back(startIndex + 1);
    inds.push_back(startIndex + 2);
}

// Helper to append a flat quad as two triangles
static void AddQuad(std::vector<Vertex>& verts, std::vector<GLuint>& inds,
                    const glm::vec3& p0, const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3,
                    const glm::vec3& color) {
    AddTriangle(verts, inds, p0, p1, p2, color);
    AddTriangle(verts, inds, p0, p2, p3, color);
}

Mesh Mesh::CreateCube(const glm::vec3& size, const glm::vec3& color) {
    std::vector<Vertex> verts;
    std::vector<GLuint> inds;
    glm::vec3 h = size * 0.5f;

    // Front
    AddQuad(verts, inds, {-h.x, -h.y,  h.z}, { h.x, -h.y,  h.z}, { h.x,  h.y,  h.z}, {-h.x,  h.y,  h.z}, color);
    // Back
    AddQuad(verts, inds, { h.x, -h.y, -h.z}, {-h.x, -h.y, -h.z}, {-h.x,  h.y, -h.z}, { h.x,  h.y, -h.z}, color * 0.8f);
    // Top
    AddQuad(verts, inds, {-h.x,  h.y,  h.z}, { h.x,  h.y,  h.z}, { h.x,  h.y, -h.z}, {-h.x,  h.y, -h.z}, color * 1.1f);
    // Bottom
    AddQuad(verts, inds, {-h.x, -h.y, -h.z}, { h.x, -h.y, -h.z}, { h.x, -h.y,  h.z}, {-h.x, -h.y,  h.z}, color * 0.6f);
    // Left
    AddQuad(verts, inds, {-h.x, -h.y, -h.z}, {-h.x, -h.y,  h.z}, {-h.x,  h.y,  h.z}, {-h.x,  h.y, -h.z}, color * 0.9f);
    // Right
    AddQuad(verts, inds, { h.x, -h.y,  h.z}, { h.x, -h.y, -h.z}, { h.x,  h.y, -h.z}, { h.x,  h.y,  h.z}, color * 0.95f);

    return Mesh(verts, inds);
}

Mesh Mesh::CreateStarfighter(const glm::vec3& bodyColor, const glm::vec3& wingColor, const glm::vec3& cockpitColor) {
    std::vector<Vertex> verts;
    std::vector<GLuint> inds;

    // Starfighter Key Landmarks:
    // Forward is -Z, Up is +Y, Right is +X
    glm::vec3 noseTip(0.0f, 0.0f, -2.4f);
    glm::vec3 noseTop(0.0f, 0.22f, -1.0f);
    glm::vec3 noseBottom(0.0f, -0.2f, -1.0f);
    glm::vec3 noseLeft(-0.35f, 0.0f, -1.0f);
    glm::vec3 noseRight(0.35f, 0.0f, -1.0f);

    glm::vec3 cockpitTop(0.0f, 0.45f, -0.1f);
    glm::vec3 cabinRear(0.0f, 0.35f, 0.9f);
    glm::vec3 cabinLeft(-0.5f, 0.1f, 0.8f);
    glm::vec3 cabinRight(0.5f, 0.1f, 0.8f);
    glm::vec3 keelBottom(0.0f, -0.32f, 0.8f);

    // 1. Sleek Nose Cone
    AddTriangle(verts, inds, noseTip, noseRight, noseTop, bodyColor);
    AddTriangle(verts, inds, noseTip, noseTop, noseLeft, bodyColor);
    AddTriangle(verts, inds, noseTip, noseLeft, noseBottom, bodyColor * 0.8f);
    AddTriangle(verts, inds, noseTip, noseBottom, noseRight, bodyColor * 0.8f);

    // 2. Cockpit Canopy (Translucent cyan/amber glass look)
    AddTriangle(verts, inds, noseTop, cabinRight, cockpitTop, cockpitColor);
    AddTriangle(verts, inds, noseTop, cockpitTop, cabinLeft, cockpitColor);
    AddTriangle(verts, inds, cockpitTop, cabinRight, cabinRear, cockpitColor * 0.9f);
    AddTriangle(verts, inds, cockpitTop, cabinRear, cabinLeft, cockpitColor * 0.9f);

    // 3. Main Fuselage flanks
    AddQuad(verts, inds, noseLeft, cabinLeft, keelBottom, noseBottom, bodyColor * 0.85f);
    AddQuad(verts, inds, noseRight, noseBottom, keelBottom, cabinRight, bodyColor * 0.85f);
    AddQuad(verts, inds, cabinLeft, cabinRight, keelBottom, keelBottom, bodyColor * 0.75f);

    // 4. Swept Forward-Slash Wings
    glm::vec3 leftWingRoot(-0.45f, 0.05f, 0.1f);
    glm::vec3 leftWingTip(-2.6f, -0.05f, 0.9f);
    glm::vec3 leftWingRear(-0.45f, 0.05f, 1.4f);
    glm::vec3 leftFinTip(-2.6f, 0.65f, 1.1f);

    AddTriangle(verts, inds, leftWingRoot, leftWingTip, leftWingRear, wingColor);
    AddTriangle(verts, inds, leftWingRear, leftWingTip, leftWingRoot, wingColor * 0.7f); // underside
    // Winglet / vertical stabilizer
    AddTriangle(verts, inds, leftWingTip, leftFinTip, glm::vec3(-2.4f, 0.0f, 1.3f), wingColor * 1.15f);

    glm::vec3 rightWingRoot(0.45f, 0.05f, 0.1f);
    glm::vec3 rightWingTip(2.6f, -0.05f, 0.9f);
    glm::vec3 rightWingRear(0.45f, 0.05f, 1.4f);
    glm::vec3 rightFinTip(2.6f, 0.65f, 1.1f);

    AddTriangle(verts, inds, rightWingRoot, rightWingRear, rightWingTip, wingColor);
    AddTriangle(verts, inds, rightWingTip, rightWingRear, rightWingRoot, wingColor * 0.7f); // underside
    // Right winglet
    AddTriangle(verts, inds, rightWingTip, glm::vec3(2.4f, 0.0f, 1.3f), rightFinTip, wingColor * 1.15f);

    // 5. Dual Blaster Cannons on wings
    glm::vec3 leftCannonPos(-1.2f, -0.1f, 0.3f);
    AddQuad(verts, inds, 
            leftCannonPos + glm::vec3(-0.06f, -0.06f, -0.7f),
            leftCannonPos + glm::vec3( 0.06f, -0.06f, -0.7f),
            leftCannonPos + glm::vec3( 0.06f,  0.06f,  0.3f),
            leftCannonPos + glm::vec3(-0.06f,  0.06f,  0.3f),
            glm::vec3(0.2f, 0.2f, 0.25f));

    glm::vec3 rightCannonPos(1.2f, -0.1f, 0.3f);
    AddQuad(verts, inds,
            rightCannonPos + glm::vec3(-0.06f, -0.06f, -0.7f),
            rightCannonPos + glm::vec3( 0.06f, -0.06f, -0.7f),
            rightCannonPos + glm::vec3( 0.06f,  0.06f,  0.3f),
            rightCannonPos + glm::vec3(-0.06f,  0.06f,  0.3f),
            glm::vec3(0.2f, 0.2f, 0.25f));

    // 6. Glowing Twin Thruster Exhausts
    glm::vec3 glowColor(0.2f, 0.8f, 1.0f);
    AddQuad(verts, inds,
            {-0.4f, -0.1f, 1.42f}, {-0.15f, -0.1f, 1.42f},
            {-0.15f,  0.15f, 1.42f}, {-0.4f,  0.15f, 1.42f},
            glowColor);
    AddQuad(verts, inds,
            {0.15f, -0.1f, 1.42f}, {0.4f, -0.1f, 1.42f},
            {0.4f,  0.15f, 1.42f}, {0.15f,  0.15f, 1.42f},
            glowColor);

    return Mesh(verts, inds);
}

Mesh Mesh::CreateStarfighterFuselage(const glm::vec3& bodyColor, const glm::vec3& cockpitColor) {
    std::vector<Vertex> verts;
    std::vector<GLuint> inds;

    // Starfighter Key Landmarks:
    // Forward is -Z, Up is +Y, Right is +X
    glm::vec3 noseTip(0.0f, 0.0f, -2.4f);
    glm::vec3 noseTop(0.0f, 0.22f, -1.0f);
    glm::vec3 noseBottom(0.0f, -0.2f, -1.0f);
    glm::vec3 noseLeft(-0.35f, 0.0f, -1.0f);
    glm::vec3 noseRight(0.35f, 0.0f, -1.0f);

    glm::vec3 cockpitTop(0.0f, 0.45f, -0.1f);
    glm::vec3 cabinRear(0.0f, 0.35f, 0.9f);
    glm::vec3 cabinLeft(-0.5f, 0.1f, 0.8f);
    glm::vec3 cabinRight(0.5f, 0.1f, 0.8f);
    glm::vec3 keelBottom(0.0f, -0.32f, 0.8f);

    // 1. Sleek Nose Cone
    AddTriangle(verts, inds, noseTip, noseRight, noseTop, bodyColor);
    AddTriangle(verts, inds, noseTip, noseTop, noseLeft, bodyColor);
    AddTriangle(verts, inds, noseTip, noseLeft, noseBottom, bodyColor * 0.8f);
    AddTriangle(verts, inds, noseTip, noseBottom, noseRight, bodyColor * 0.8f);

    // 2. Cockpit Canopy (Translucent cyan/amber glass look)
    AddTriangle(verts, inds, noseTop, cabinRight, cockpitTop, cockpitColor);
    AddTriangle(verts, inds, noseTop, cockpitTop, cabinLeft, cockpitColor);
    AddTriangle(verts, inds, cockpitTop, cabinRight, cabinRear, cockpitColor * 0.9f);
    AddTriangle(verts, inds, cockpitTop, cabinRear, cabinLeft, cockpitColor * 0.9f);

    // 3. Main Fuselage flanks
    AddQuad(verts, inds, noseLeft, cabinLeft, keelBottom, noseBottom, bodyColor * 0.85f);
    AddQuad(verts, inds, noseRight, noseBottom, keelBottom, cabinRight, bodyColor * 0.85f);
    AddQuad(verts, inds, cabinLeft, cabinRight, keelBottom, keelBottom, bodyColor * 0.75f);

    // 4. Exposed Wing Mount Sockets (Mechanical dark alloy)
    glm::vec3 socketColor(0.24f, 0.26f, 0.3f);
    AddQuad(verts, inds,
            {-0.46f, -0.05f, 0.1f}, {-0.46f, -0.05f, 1.35f},
            {-0.46f,  0.15f, 1.35f}, {-0.46f,  0.15f, 0.1f},
            socketColor);
    AddQuad(verts, inds,
            {0.46f, -0.05f, 1.35f}, {0.46f, -0.05f, 0.1f},
            {0.46f,  0.15f, 0.1f}, {0.46f,  0.15f, 1.35f},
            socketColor);

    // 5. Glowing Twin Thruster Exhausts
    glm::vec3 glowColor(0.2f, 0.8f, 1.0f);
    AddQuad(verts, inds,
            {-0.4f, -0.1f, 1.42f}, {-0.15f, -0.1f, 1.42f},
            {-0.15f,  0.15f, 1.42f}, {-0.4f,  0.15f, 1.42f},
            glowColor);
    AddQuad(verts, inds,
            {0.15f, -0.1f, 1.42f}, {0.4f, -0.1f, 1.42f},
            {0.4f,  0.15f, 1.42f}, {0.15f,  0.15f, 1.42f},
            glowColor);

    return Mesh(verts, inds);
}

Mesh Mesh::CreateStarfighterLeftWing(const glm::vec3& wingColor) {
    std::vector<Vertex> verts;
    std::vector<GLuint> inds;

    glm::vec3 leftWingRoot(-0.45f, 0.05f, 0.1f);
    glm::vec3 leftWingTip(-2.6f, -0.05f, 0.9f);
    glm::vec3 leftWingRear(-0.45f, 0.05f, 1.4f);
    glm::vec3 leftFinTip(-2.6f, 0.65f, 1.1f);

    AddTriangle(verts, inds, leftWingRoot, leftWingTip, leftWingRear, wingColor);
    AddTriangle(verts, inds, leftWingRear, leftWingTip, leftWingRoot, wingColor * 0.7f); // underside
    // Winglet / vertical stabilizer
    AddTriangle(verts, inds, leftWingTip, leftFinTip, glm::vec3(-2.4f, 0.0f, 1.3f), wingColor * 1.15f);
    AddTriangle(verts, inds, glm::vec3(-2.4f, 0.0f, 1.3f), leftFinTip, leftWingTip, wingColor * 0.85f);

    // Blaster Cannon
    glm::vec3 leftCannonPos(-1.2f, -0.1f, 0.3f);
    AddQuad(verts, inds, 
            leftCannonPos + glm::vec3(-0.06f, -0.06f, -0.7f),
            leftCannonPos + glm::vec3( 0.06f, -0.06f, -0.7f),
            leftCannonPos + glm::vec3( 0.06f,  0.06f,  0.3f),
            leftCannonPos + glm::vec3(-0.06f,  0.06f,  0.3f),
            glm::vec3(0.2f, 0.2f, 0.25f));

    return Mesh(verts, inds);
}

Mesh Mesh::CreateStarfighterRightWing(const glm::vec3& wingColor) {
    std::vector<Vertex> verts;
    std::vector<GLuint> inds;

    glm::vec3 rightWingRoot(0.45f, 0.05f, 0.1f);
    glm::vec3 rightWingTip(2.6f, -0.05f, 0.9f);
    glm::vec3 rightWingRear(0.45f, 0.05f, 1.4f);
    glm::vec3 rightFinTip(2.6f, 0.65f, 1.1f);

    AddTriangle(verts, inds, rightWingRoot, rightWingRear, rightWingTip, wingColor);
    AddTriangle(verts, inds, rightWingTip, rightWingRear, rightWingRoot, wingColor * 0.7f); // underside
    // Right winglet
    AddTriangle(verts, inds, rightWingTip, glm::vec3(2.4f, 0.0f, 1.3f), rightFinTip, wingColor * 1.15f);
    AddTriangle(verts, inds, rightFinTip, glm::vec3(2.4f, 0.0f, 1.3f), rightWingTip, wingColor * 0.85f);

    // Blaster Cannon
    glm::vec3 rightCannonPos(1.2f, -0.1f, 0.3f);
    AddQuad(verts, inds,
            rightCannonPos + glm::vec3(-0.06f, -0.06f, -0.7f),
            rightCannonPos + glm::vec3( 0.06f, -0.06f, -0.7f),
            rightCannonPos + glm::vec3( 0.06f,  0.06f,  0.3f),
            rightCannonPos + glm::vec3(-0.06f,  0.06f,  0.3f),
            glm::vec3(0.2f, 0.2f, 0.25f));

    return Mesh(verts, inds);
}

Mesh Mesh::CreateEnemyDrone(const glm::vec3& bodyColor, const glm::vec3& eyeColor) {
    std::vector<Vertex> verts;
    std::vector<GLuint> inds;

    // Aggressive stealth-interceptor shape facing the player (towards +Z)
    glm::vec3 noseTip(0.0f, 0.0f, 1.6f);
    glm::vec3 wingLeft(-1.8f, 0.2f, -0.8f);
    glm::vec3 wingRight(1.8f, 0.2f, -0.8f);
    glm::vec3 topCenter(0.0f, 0.4f, -0.2f);
    glm::vec3 bottomCenter(0.0f, -0.3f, -0.2f);
    glm::vec3 rearCenter(0.0f, 0.0f, -1.2f);

    // Top Hull
    AddTriangle(verts, inds, noseTip, topCenter, wingLeft, bodyColor);
    AddTriangle(verts, inds, noseTip, wingRight, topCenter, bodyColor);
    AddTriangle(verts, inds, topCenter, rearCenter, wingLeft, bodyColor * 0.85f);
    AddTriangle(verts, inds, topCenter, wingRight, rearCenter, bodyColor * 0.85f);

    // Bottom Hull
    AddTriangle(verts, inds, noseTip, wingLeft, bottomCenter, bodyColor * 0.7f);
    AddTriangle(verts, inds, noseTip, bottomCenter, wingRight, bodyColor * 0.7f);
    AddTriangle(verts, inds, bottomCenter, wingLeft, rearCenter, bodyColor * 0.6f);
    AddTriangle(verts, inds, bottomCenter, rearCenter, wingRight, bodyColor * 0.6f);

    // Center targeting eye (red glow)
    glm::vec3 eyeOffset(0.0f, 0.05f, 1.1f);
    AddQuad(verts, inds,
            eyeOffset + glm::vec3(-0.2f, -0.1f, 0.0f),
            eyeOffset + glm::vec3( 0.2f, -0.1f, 0.0f),
            eyeOffset + glm::vec3( 0.2f,  0.15f, 0.0f),
            eyeOffset + glm::vec3(-0.2f,  0.15f, 0.0f),
            eyeColor);

    return Mesh(verts, inds);
}

Mesh Mesh::CreateRing(float innerRadius, float outerRadius, int segments, const glm::vec3& color) {
    std::vector<Vertex> verts;
    std::vector<GLuint> inds;

    float step = (2.0f * glm::pi<float>()) / segments;
    float depth = 0.35f;

    for (int i = 0; i < segments; ++i) {
        float a0 = i * step;
        float a1 = (i + 1) * step;

        float c0 = std::cos(a0), s0 = std::sin(a0);
        float c1 = std::cos(a1), s1 = std::sin(a1);

        glm::vec3 in0_front(c0 * innerRadius, s0 * innerRadius, -depth * 0.5f);
        glm::vec3 out0_front(c0 * outerRadius, s0 * outerRadius, -depth * 0.5f);
        glm::vec3 in1_front(c1 * innerRadius, s1 * innerRadius, -depth * 0.5f);
        glm::vec3 out1_front(c1 * outerRadius, s1 * outerRadius, -depth * 0.5f);

        glm::vec3 in0_back(c0 * innerRadius, s0 * innerRadius, depth * 0.5f);
        glm::vec3 out0_back(c0 * outerRadius, s0 * outerRadius, depth * 0.5f);
        glm::vec3 in1_back(c1 * innerRadius, s1 * innerRadius, depth * 0.5f);
        glm::vec3 out1_back(c1 * outerRadius, s1 * outerRadius, depth * 0.5f);

        // Front Face
        AddQuad(verts, inds, in0_front, out0_front, out1_front, in1_front, color);
        // Back Face
        AddQuad(verts, inds, out0_back, in0_back, in1_back, out1_back, color * 0.85f);
        // Outer Face
        AddQuad(verts, inds, out0_front, out0_back, out1_back, out1_front, color * 1.1f);
        // Inner Face
        AddQuad(verts, inds, in0_back, in0_front, in1_front, in1_back, color * 0.9f);
    }

    return Mesh(verts, inds);
}

Mesh Mesh::CreateLaserBolt(float width, float length, const glm::vec3& color) {
    std::vector<Vertex> verts;
    std::vector<GLuint> inds;

    float w = width * 0.5f;
    float l = length * 0.5f;

    // Cross-quad laser beam for bright visibility from any camera angle
    AddQuad(verts, inds, {-w, 0.0f, -l}, { w, 0.0f, -l}, { w, 0.0f,  l}, {-w, 0.0f,  l}, color);
    AddQuad(verts, inds, {0.0f, -w, -l}, {0.0f,  w, -l}, {0.0f,  w,  l}, {0.0f, -w,  l}, color);

    return Mesh(verts, inds);
}

Mesh Mesh::CreateCrosshair(float size, const glm::vec3& color) {
    std::vector<Vertex> verts;
    std::vector<GLuint> inds;

    float s = size * 0.5f;
    float t = size * 0.065f;  // line thickness
    float len = size * 0.32f; // corner bracket arm length

    // Ex-Zodiac 4-Corner L-Brackets (Top-Left, Top-Right, Bottom-Left, Bottom-Right)
    // 1. Top-Left Corner
    AddQuad(verts, inds, {-s, s - t, 0.0f}, {-s + len, s - t, 0.0f}, {-s + len, s, 0.0f}, {-s, s, 0.0f}, color);
    AddQuad(verts, inds, {-s, s - len, 0.0f}, {-s + t, s - len, 0.0f}, {-s + t, s, 0.0f}, {-s, s, 0.0f}, color);

    // 2. Top-Right Corner
    AddQuad(verts, inds, {s - len, s - t, 0.0f}, {s, s - t, 0.0f}, {s, s, 0.0f}, {s - len, s, 0.0f}, color);
    AddQuad(verts, inds, {s - t, s - len, 0.0f}, {s, s - len, 0.0f}, {s, s, 0.0f}, {s - t, s, 0.0f}, color);

    // 3. Bottom-Left Corner
    AddQuad(verts, inds, {-s, -s, 0.0f}, {-s + len, -s, 0.0f}, {-s + len, -s + t, 0.0f}, {-s, -s + t, 0.0f}, color);
    AddQuad(verts, inds, {-s, -s, 0.0f}, {-s + t, -s, 0.0f}, {-s + t, -s + len, 0.0f}, {-s, -s + len, 0.0f}, color);

    // 4. Bottom-Right Corner
    AddQuad(verts, inds, {s - len, -s, 0.0f}, {s, -s, 0.0f}, {s, -s + t, 0.0f}, {s - len, -s + t, 0.0f}, color);
    AddQuad(verts, inds, {s - t, -s, 0.0f}, {s, -s, 0.0f}, {s, -s + len, 0.0f}, {s - t, -s + len, 0.0f}, color);

    // Center Crosshair Micro-Tick
    float ct = t * 0.75f;
    float cl = size * 0.09f;
    AddQuad(verts, inds, {-cl, -ct, 0.0f}, {cl, -ct, 0.0f}, {cl, ct, 0.0f}, {-cl, ct, 0.0f}, color);
    AddQuad(verts, inds, {-ct, -cl, 0.0f}, {ct, -cl, 0.0f}, {ct, cl, 0.0f}, {-ct, cl, 0.0f}, color);

    return Mesh(verts, inds);
}

Mesh Mesh::CreateAsteroid(float radius, const glm::vec3& color) {
    std::vector<Vertex> verts;
    std::vector<GLuint> inds;

    // Faceted low-poly octahedron rock with irregular vertex offsets
    glm::vec3 top(0.0f, radius * 1.15f, 0.0f);
    glm::vec3 bottom(0.0f, -radius * 1.1f, 0.0f);
    glm::vec3 front(0.1f * radius, 0.1f * radius, radius * 1.2f);
    glm::vec3 back(-0.1f * radius, -0.1f * radius, -radius * 1.05f);
    glm::vec3 left(-radius * 1.25f, 0.05f * radius, -0.1f * radius);
    glm::vec3 right(radius * 1.1f, -0.15f * radius, 0.1f * radius);

    AddTriangle(verts, inds, top, right, front, color * 1.1f);
    AddTriangle(verts, inds, top, front, left, color * 0.95f);
    AddTriangle(verts, inds, top, left, back, color * 0.85f);
    AddTriangle(verts, inds, top, back, right, color * 1.0f);

    AddTriangle(verts, inds, bottom, front, right, color * 0.75f);
    AddTriangle(verts, inds, bottom, left, front, color * 0.65f);
    AddTriangle(verts, inds, bottom, back, left, color * 0.55f);
    AddTriangle(verts, inds, bottom, right, back, color * 0.7f);

    return Mesh(verts, inds);
}

Mesh Mesh::CreateSphere(float radius, int rings, int sectors, const glm::vec3& color) {
    std::vector<Vertex> verts;
    std::vector<GLuint> inds;

    float R = 1.0f / static_cast<float>(rings - 1);
    float S = 1.0f / static_cast<float>(sectors - 1);

    for (int r = 0; r < rings; ++r) {
        for (int s = 0; s < sectors; ++s) {
            float y = std::sin(-glm::half_pi<float>() + glm::pi<float>() * r * R);
            float x = std::cos(2.0f * glm::pi<float>() * s * S) * std::sin(glm::pi<float>() * r * R);
            float z = std::sin(2.0f * glm::pi<float>() * s * S) * std::sin(glm::pi<float>() * r * R);

            glm::vec3 pos(x * radius, y * radius, z * radius);
            glm::vec3 norm = glm::normalize(pos);
            verts.push_back({pos, norm, color});
        }
    }

    for (int r = 0; r < rings - 1; ++r) {
        for (int s = 0; s < sectors - 1; ++s) {
            GLuint cur = r * sectors + s;
            GLuint next = (r + 1) * sectors + s;

            inds.push_back(cur);
            inds.push_back(next);
            inds.push_back(next + 1);

            inds.push_back(cur);
            inds.push_back(next + 1);
            inds.push_back(cur + 1);
        }
    }

    return Mesh(verts, inds);
}

Mesh Mesh::CreateShockwave(float radius, float thickness, int segments, const glm::vec3& color) {
    std::vector<Vertex> verts;
    std::vector<GLuint> inds;

    float step = (2.0f * glm::pi<float>()) / segments;
    float innerR = radius - thickness * 0.5f;
    float outerR = radius + thickness * 0.5f;

    for (int i = 0; i < segments; ++i) {
        float a0 = i * step;
        float a1 = (i + 1) * step;

        glm::vec3 in0(std::cos(a0) * innerR, std::sin(a0) * innerR, 0.0f);
        glm::vec3 out0(std::cos(a0) * outerR, std::sin(a0) * outerR, 0.0f);
        glm::vec3 in1(std::cos(a1) * innerR, std::sin(a1) * innerR, 0.0f);
        glm::vec3 out1(std::cos(a1) * outerR, std::sin(a1) * outerR, 0.0f);

        AddQuad(verts, inds, in0, out0, out1, in1, color);
    }

    return Mesh(verts, inds);
}

Mesh Mesh::CreateLockOnDiamond(float size, const glm::vec3& color) {
    std::vector<Vertex> verts;
    std::vector<GLuint> inds;

    float s = size * 0.5f;
    float t = size * 0.1f;
    float l = size * 0.35f;

    // 4 diamond corners
    // Top corner
    AddQuad(verts, inds, {-t, s - l, 0.0f}, {t, s - l, 0.0f}, {t, s, 0.0f}, {-t, s, 0.0f}, color);
    // Bottom corner
    AddQuad(verts, inds, {-t, -s, 0.0f}, {t, -s, 0.0f}, {t, -s + l, 0.0f}, {-t, -s + l, 0.0f}, color);
    // Left corner
    AddQuad(verts, inds, {-s, -t, 0.0f}, {-s + l, -t, 0.0f}, {-s + l, t, 0.0f}, {-s, t, 0.0f}, color);
    // Right corner
    AddQuad(verts, inds, {s - l, -t, 0.0f}, {s, -t, 0.0f}, {s, t, 0.0f}, {s - l, t, 0.0f}, color);

    return Mesh(verts, inds);
}

Mesh Mesh::CreateCanyonSection(float length, float width, float wallHeight,
                               const glm::vec3& floorColA, const glm::vec3& floorColB,
                               const glm::vec3& wallCol) {
    std::vector<Vertex> verts;
    std::vector<GLuint> inds;

    float hw = width * 0.5f;
    float hl = length * 0.5f;
    float floorY = -7.5f;

    // 1. Procedural 3D Checkered Surface (8 columns across width, 12 rows along length)
    const int xSegs = 8;
    const int zSegs = 12;
    float dx = width / xSegs;
    float dz = length / zSegs;

    for (int iz = 0; iz < zSegs; ++iz) {
        float z0 = -hl + iz * dz;
        float z1 = z0 + dz;

        for (int ix = 0; ix < xSegs; ++ix) {
            float x0 = -hw + ix * dx;
            float x1 = x0 + dx;

            // Two-tone checkerboard tile selection (Space Harrier / Ex-Zodiac arcade grid)
            bool isTileA = ((ix + iz) % 2 == 0);
            glm::vec3 tileCol = isTileA ? floorColA : floorColB;

            // Subtle faceted water/canyon wave variations
            float y00 = floorY + std::sin(ix * 1.2f + iz * 1.5f) * 0.15f;
            float y10 = floorY + std::sin((ix + 1) * 1.2f + iz * 1.5f) * 0.15f;
            float y11 = floorY + std::sin((ix + 1) * 1.2f + (iz + 1) * 1.5f) * 0.15f;
            float y01 = floorY + std::sin(ix * 1.2f + (iz + 1) * 1.5f) * 0.15f;

            AddQuad(verts, inds,
                    {x0, y00, z0}, {x1, y10, z0}, {x1, y11, z1}, {x0, y01, z1},
                    tileCol);
        }
    }

    // 2. Left Cliff Wall (Ex-Zodiac Pastel Pink Mesa with Sandy Beach and Emerald Grassy Rim)
    glm::vec3 beachCol(0.96f, 0.92f, 0.80f); // Sandy shoreline
    glm::vec3 grassCol(0.42f, 0.85f, 0.50f); // Lush vibrant mesa top

    for (int iz = 0; iz < zSegs; ++iz) {
        float z0 = -hl + iz * dz;
        float z1 = z0 + dz;
        float shade = 0.90f + 0.18f * (iz % 2);

        // Tier 1: Sandy shoreline embankment
        glm::vec3 b0(-hw, floorY, z0);
        glm::vec3 b1(-hw, floorY, z1);
        glm::vec3 m0(-hw - 3.5f, floorY + wallHeight * 0.25f, z0);
        glm::vec3 m1(-hw - 3.5f, floorY + wallHeight * 0.25f, z1);
        AddQuad(verts, inds, b0, m0, m1, b1, beachCol * shade);

        // Tier 2: Pastel pink/peach sandstone cliff face
        glm::vec3 u0(-hw - 6.5f, floorY + wallHeight * 0.82f, z0);
        glm::vec3 u1(-hw - 6.5f, floorY + wallHeight * 0.82f, z1);
        AddQuad(verts, inds, m0, u0, u1, m1, wallCol * 1.05f * shade);

        // Tier 3: Upper rim / plateau lush green lawn
        glm::vec3 t0(-hw - 13.0f, floorY + wallHeight, z0);
        glm::vec3 t1(-hw - 13.0f, floorY + wallHeight, z1);
        AddQuad(verts, inds, u0, t0, t1, u1, grassCol * shade);
    }

    // 3. Right Cliff Wall (Ex-Zodiac Pastel Pink Mesa with Sandy Beach and Emerald Grassy Rim)
    for (int iz = 0; iz < zSegs; ++iz) {
        float z0 = -hl + iz * dz;
        float z1 = z0 + dz;
        float shade = 0.86f + 0.20f * ((iz + 1) % 2);

        // Tier 1: Sandy shoreline embankment
        glm::vec3 b0(hw, floorY, z0);
        glm::vec3 b1(hw, floorY, z1);
        glm::vec3 m0(hw + 3.5f, floorY + wallHeight * 0.25f, z0);
        glm::vec3 m1(hw + 3.5f, floorY + wallHeight * 0.25f, z1);
        AddQuad(verts, inds, b0, b1, m1, m0, beachCol * shade);

        // Tier 2: Pastel pink/peach sandstone cliff face
        glm::vec3 u0(hw + 6.5f, floorY + wallHeight * 0.82f, z0);
        glm::vec3 u1(hw + 6.5f, floorY + wallHeight * 0.82f, z1);
        AddQuad(verts, inds, m0, m1, u1, u0, wallCol * 0.98f * shade);

        // Tier 3: Upper rim / plateau lush green lawn
        glm::vec3 t0(hw + 13.0f, floorY + wallHeight, z0);
        glm::vec3 t1(hw + 13.0f, floorY + wallHeight, z1);
        AddQuad(verts, inds, u0, u1, t1, t0, grassCol * shade);
    }

    return Mesh(verts, inds);
}

Mesh Mesh::CreateArcadeHorizon(float radius, float height,
                               const glm::vec3& skyTopCol, const glm::vec3& horizonCol,
                               const glm::vec3& mountainCol) {
    std::vector<Vertex> verts;
    std::vector<GLuint> inds;

    const int hSegs = 32;
    float startAngle = -glm::radians(88.0f);
    float endAngle = glm::radians(88.0f);
    float dAngle = (endAngle - startAngle) / hSegs;

    // 1. Distant Sky Dome Bands (From Horizon to High Zenith)
    float yBot = -30.0f;
    float yLow = 15.0f;
    float yMid = 60.0f;
    float yHigh = 120.0f;
    float yTop = height;

    glm::vec3 colBot = horizonCol;
    glm::vec3 colLow = glm::mix(horizonCol, skyTopCol, 0.25f);
    glm::vec3 colMid = glm::mix(horizonCol, skyTopCol, 0.55f);
    glm::vec3 colHigh = glm::mix(horizonCol, skyTopCol, 0.85f);
    glm::vec3 colTop = skyTopCol;

    for (int i = 0; i < hSegs; ++i) {
        float a0 = startAngle + i * dAngle;
        float a1 = a0 + dAngle;

        float s0 = std::sin(a0), c0 = std::cos(a0);
        float s1 = std::sin(a1), c1 = std::cos(a1);

        glm::vec3 p0_bot(s0 * radius, yBot, -c0 * radius);
        glm::vec3 p1_bot(s1 * radius, yBot, -c1 * radius);

        glm::vec3 p0_low(s0 * radius, yLow, -c0 * radius);
        glm::vec3 p1_low(s1 * radius, yLow, -c1 * radius);

        glm::vec3 p0_mid(s0 * radius, yMid, -c0 * radius);
        glm::vec3 p1_mid(s1 * radius, yMid, -c1 * radius);

        glm::vec3 p0_high(s0 * radius, yHigh, -c0 * radius);
        glm::vec3 p1_high(s1 * radius, yHigh, -c1 * radius);

        glm::vec3 p0_top(s0 * radius, yTop, -c0 * radius);
        glm::vec3 p1_top(s1 * radius, yTop, -c1 * radius);

        AddQuad(verts, inds, p0_bot, p1_bot, p1_low, p0_low, colBot);
        AddQuad(verts, inds, p0_low, p1_low, p1_mid, p0_mid, colLow);
        AddQuad(verts, inds, p0_mid, p1_mid, p1_high, p0_high, colMid);
        AddQuad(verts, inds, p0_high, p1_high, p1_top, p0_top, colHigh);
    }

    // 2. Layer 1: Majestic Distant Sawtooth Peaks (High Horizon Mountains)
    float mRadius = radius * 0.94f;
    glm::vec3 snowCol(0.92f, 0.96f, 1.0f); // Alpine snowcaps

    for (int i = 0; i < hSegs; ++i) {
        float a0 = startAngle + i * dAngle;
        float a1 = a0 + dAngle;
        float aMid = (a0 + a1) * 0.5f;

        float s0 = std::sin(a0), c0 = std::cos(a0);
        float s1 = std::sin(a1), c1 = std::cos(a1);
        float sm = std::sin(aMid), cm = std::cos(aMid);

        // Prominent peak heights (35m to 85m tall)
        float peakY = 38.0f + std::sin(i * 1.6f + 0.4f) * 26.0f + std::cos(i * 3.1f) * 18.0f;
        float baseLeftY = -8.0f;
        float baseRightY = -8.0f;

        glm::vec3 b0(s0 * mRadius, baseLeftY, -c0 * mRadius);
        glm::vec3 b1(s1 * mRadius, baseRightY, -c1 * mRadius);
        glm::vec3 peak(sm * mRadius, peakY, -cm * mRadius);

        // Sunlit and shadow facets
        glm::vec3 sunCol = mountainCol * 1.25f;
        glm::vec3 shadeCol = mountainCol * 0.78f;
        glm::vec3 basePeakCol = (i % 2 == 0) ? sunCol : shadeCol;

        // Mountain base triangle
        AddTriangle(verts, inds, b0, peak, b1, basePeakCol);

        // Snowcap on top 28% of the highest peaks
        if (peakY > 48.0f) {
            float snowY = peakY - (peakY - baseLeftY) * 0.28f;
            float tSnow = (peakY - snowY) / (peakY - baseLeftY);
            glm::vec3 sLeft = glm::mix(peak, b0, tSnow);
            glm::vec3 sRight = glm::mix(peak, b1, tSnow);
            AddTriangle(verts, inds, sLeft, peak, sRight, (i % 2 == 0) ? snowCol : snowCol * 0.88f);
        }
    }

    // 3. Layer 2: Middle Foothills & Low-Poly Ridges (Foreground Parallax Layer)
    float fRadius = radius * 0.86f;
    glm::vec3 footColA(0.24f, 0.46f, 0.35f); // Sage green ridges
    glm::vec3 footColB(0.32f, 0.54f, 0.42f); // Sunlit hills

    for (int i = 0; i < hSegs; ++i) {
        float a0 = startAngle + i * dAngle;
        float a1 = a0 + dAngle;
        float aMid = (a0 + a1) * 0.5f;

        float s0 = std::sin(a0), c0 = std::cos(a0);
        float s1 = std::sin(a1), c1 = std::cos(a1);
        float sm = std::sin(aMid), cm = std::cos(aMid);

        float hillY = 8.0f + std::sin(i * 2.2f + 1.2f) * 12.0f + std::cos(i * 1.5f) * 8.0f;
        float baseLeftY = -8.0f;
        float baseRightY = -8.0f;

        glm::vec3 b0(s0 * fRadius, baseLeftY, -c0 * fRadius);
        glm::vec3 b1(s1 * fRadius, baseRightY, -c1 * fRadius);
        glm::vec3 peak(sm * fRadius, hillY, -cm * fRadius);

        glm::vec3 col = (i % 2 == 0) ? footColB : footColA;
        AddTriangle(verts, inds, b0, peak, b1, col);
    }

    return Mesh(verts, inds);
}

Mesh Mesh::CreateRockArch(float spanWidth, float height, float thickness, const glm::vec3& color) {
    std::vector<Vertex> verts;
    std::vector<GLuint> inds;

    float hw = spanWidth * 0.5f;
    float ht = thickness * 0.5f;
    float floorY = -7.5f;
    float topY = floorY + height;
    float colW = 4.5f;

    // Left Column
    AddQuad(verts, inds, {-hw, floorY,  ht}, {-hw + colW, floorY,  ht}, {-hw + colW, topY,  ht}, {-hw, topY,  ht}, color * 0.95f);
    AddQuad(verts, inds, {-hw + colW, floorY, -ht}, {-hw, floorY, -ht}, {-hw, topY, -ht}, {-hw + colW, topY, -ht}, color * 0.85f);
    AddQuad(verts, inds, {-hw, floorY, -ht}, {-hw, floorY,  ht}, {-hw, topY,  ht}, {-hw, topY, -ht}, color * 0.9f);
    AddQuad(verts, inds, {-hw + colW, floorY,  ht}, {-hw + colW, floorY, -ht}, {-hw + colW, topY, -ht}, {-hw + colW, topY,  ht}, color * 1.05f);

    // Right Column
    AddQuad(verts, inds, {hw - colW, floorY,  ht}, {hw, floorY,  ht}, {hw, topY,  ht}, {hw - colW, topY,  ht}, color * 0.95f);
    AddQuad(verts, inds, {hw, floorY, -ht}, {hw - colW, floorY, -ht}, {hw - colW, topY, -ht}, {hw, topY, -ht}, color * 0.85f);
    AddQuad(verts, inds, {hw - colW, floorY, -ht}, {hw - colW, floorY,  ht}, {hw - colW, topY,  ht}, {hw - colW, topY, -ht}, color * 1.05f);
    AddQuad(verts, inds, {hw, floorY,  ht}, {hw, floorY, -ht}, {hw, topY, -ht}, {hw, topY,  ht}, color * 0.9f);

    // Horizontal Arch Spanning Top
    float archBottomY = topY - 3.5f;
    // Front face
    AddQuad(verts, inds, {-hw, archBottomY, ht}, {hw, archBottomY, ht}, {hw, topY, ht}, {-hw, topY, ht}, color * 1.15f);
    // Back face
    AddQuad(verts, inds, {hw, archBottomY, -ht}, {-hw, archBottomY, -ht}, {-hw, topY, -ht}, {hw, topY, -ht}, color * 0.8f);
    // Underside (so you see the ceiling as you fly underneath!)
    AddQuad(verts, inds, {-hw, archBottomY, -ht}, {hw, archBottomY, -ht}, {hw, archBottomY, ht}, {-hw, archBottomY, ht}, color * 0.75f);
    // Top roof
    AddQuad(verts, inds, {-hw, topY, ht}, {hw, topY, ht}, {hw, topY, -ht}, {-hw, topY, -ht}, color * 1.2f);

    return Mesh(verts, inds);
}

Mesh Mesh::CreatePillar(float radius, float height, const glm::vec3& color) {
    std::vector<Vertex> verts;
    std::vector<GLuint> inds;

    const int sides = 6;
    float floorY = -7.5f;
    float topY = floorY + height;
    float step = (2.0f * glm::pi<float>()) / sides;

    for (int i = 0; i < sides; ++i) {
        float a0 = i * step;
        float a1 = (i + 1) * step;

        glm::vec3 b0(std::cos(a0) * radius, floorY, std::sin(a0) * radius);
        glm::vec3 b1(std::cos(a1) * radius, floorY, std::sin(a1) * radius);
        glm::vec3 t0(std::cos(a0) * (radius * 0.75f), topY, std::sin(a0) * (radius * 0.75f));
        glm::vec3 t1(std::cos(a1) * (radius * 0.75f), topY, std::sin(a1) * (radius * 0.75f));

        float shade = 0.8f + 0.35f * ((i % 2) ? 1.0f : 0.0f);
        AddQuad(verts, inds, b0, b1, t1, t0, color * shade);
    }

    return Mesh(verts, inds);
}

Mesh Mesh::CreateBossHull(const glm::vec3& mainCol, const glm::vec3& accentCol) {
    std::vector<Vertex> verts;
    std::vector<GLuint> inds;

    // Colossal Dreadnought Hull (Facing +Z toward player)
    // 1. Central Armored Wedge Bow & Fuselage
    glm::vec3 noseTop(0.0f, 1.0f, 6.5f);
    glm::vec3 noseBottom(0.0f, -1.2f, 6.5f);
    glm::vec3 noseLeft(-3.2f, 0.0f, 3.5f);
    glm::vec3 noseRight(3.2f, 0.0f, 3.5f);

    AddTriangle(verts, inds, noseTop, noseRight, noseLeft, mainCol);
    AddTriangle(verts, inds, noseBottom, noseLeft, noseRight, mainCol * 0.75f);
    AddTriangle(verts, inds, noseTop, noseLeft, noseBottom, mainCol * 0.85f);
    AddTriangle(verts, inds, noseTop, noseBottom, noseRight, mainCol * 0.85f);

    // 2. Main Superstructure Body
    glm::vec3 bodyTopLeft(-4.0f, 2.2f, -1.0f);
    glm::vec3 bodyTopRight(4.0f, 2.2f, -1.0f);
    glm::vec3 bodyBotLeft(-4.0f, -2.0f, -1.0f);
    glm::vec3 bodyBotRight(4.0f, -2.0f, -1.0f);

    AddQuad(verts, inds, noseLeft, noseRight, bodyTopRight, bodyTopLeft, mainCol * 0.95f);
    AddQuad(verts, inds, noseLeft, bodyBotLeft, bodyBotRight, noseRight, mainCol * 0.7f);
    AddQuad(verts, inds, noseLeft, bodyTopLeft, bodyBotLeft, noseLeft, mainCol * 0.8f);
    AddQuad(verts, inds, noseRight, bodyBotRight, bodyTopRight, noseRight, mainCol * 0.85f);

    // 3. Command Bridge Tower on Spine
    AddQuad(verts, inds,
            {-1.8f, 2.2f, 1.2f}, { 1.8f, 2.2f, 1.2f},
            { 1.2f, 4.2f, -0.5f}, {-1.2f, 4.2f, -0.5f}, accentCol);
    AddQuad(verts, inds,
            { 1.8f, 2.2f, 1.2f}, { 1.8f, 2.2f, -3.0f},
            { 1.2f, 4.2f, -3.0f}, { 1.2f, 4.2f, -0.5f}, accentCol * 0.85f);
    AddQuad(verts, inds,
            {-1.8f, 2.2f, -3.0f}, {-1.8f, 2.2f, 1.2f},
            {-1.2f, 4.2f, -0.5f}, {-1.2f, 4.2f, -3.0f}, accentCol * 0.85f);

    // 4. Heavy Outrigger Wings / Turret Sponsons (Left & Right)
    // Left Wing
    glm::vec3 lWingRootFwd(-4.0f, 0.5f, 2.0f);
    glm::vec3 lWingRootAft(-4.0f, 0.5f, -4.5f);
    glm::vec3 lWingTipFwd(-11.5f, -0.4f, 0.5f);
    glm::vec3 lWingTipAft(-11.5f, -0.4f, -4.0f);

    AddQuad(verts, inds, lWingRootFwd, lWingTipFwd, lWingTipAft, lWingRootAft, accentCol * 0.9f);
    AddQuad(verts, inds, lWingRootFwd, lWingRootAft, lWingTipAft, lWingTipFwd, mainCol * 0.65f); // underside

    // Right Wing
    glm::vec3 rWingRootFwd(4.0f, 0.5f, 2.0f);
    glm::vec3 rWingRootAft(4.0f, 0.5f, -4.5f);
    glm::vec3 rWingTipFwd(11.5f, -0.4f, 0.5f);
    glm::vec3 rWingTipAft(11.5f, -0.4f, -4.0f);

    AddQuad(verts, inds, rWingRootFwd, rWingRootAft, rWingTipAft, rWingTipFwd, accentCol * 0.9f);
    AddQuad(verts, inds, rWingRootFwd, rWingTipFwd, rWingTipAft, rWingRootAft, mainCol * 0.65f);

    // 5. Quad Engine Thruster Exhausts on Stern
    glm::vec3 thrusterGlow(0.2f, 0.85f, 1.0f);
    AddQuad(verts, inds, {-3.2f, -1.2f, -5.0f}, {-1.2f, -1.2f, -5.0f}, {-1.2f, 0.8f, -5.0f}, {-3.2f, 0.8f, -5.0f}, thrusterGlow);
    AddQuad(verts, inds, { 1.2f, -1.2f, -5.0f}, { 3.2f, -1.2f, -5.0f}, { 3.2f, 0.8f, -5.0f}, { 1.2f, 0.8f, -5.0f}, thrusterGlow);
    AddQuad(verts, inds, {-9.5f, -0.8f, -4.2f}, {-8.0f, -0.8f, -4.2f}, {-8.0f, 0.4f, -4.2f}, {-9.5f, 0.4f, -4.2f}, thrusterGlow);
    AddQuad(verts, inds, { 8.0f, -0.8f, -4.2f}, { 9.5f, -0.8f, -4.2f}, { 9.5f, 0.4f, -4.2f}, { 8.0f, 0.4f, -4.2f}, thrusterGlow);

    return Mesh(verts, inds);
}

Mesh Mesh::CreateBossTurret(const glm::vec3& turretColor) {
    std::vector<Vertex> verts;
    std::vector<GLuint> inds;

    // Armored Swivel Base Dome
    float baseSize = 1.4f;
    AddQuad(verts, inds,
            {-baseSize, 0.0f, -baseSize}, {baseSize, 0.0f, -baseSize},
            {baseSize, 0.0f, baseSize}, {-baseSize, 0.0f, baseSize}, turretColor * 0.8f);
    AddQuad(verts, inds,
            {-baseSize * 0.8f, 0.7f, -baseSize * 0.8f}, {baseSize * 0.8f, 0.7f, -baseSize * 0.8f},
            {baseSize * 0.8f, 0.7f, baseSize * 0.8f}, {-baseSize * 0.8f, 0.7f, baseSize * 0.8f}, turretColor * 1.15f);

    // Twin Heavy Plasma Barrels
    float barrelLen = 3.2f;
    float bRadius = 0.22f;

    // Left barrel
    glm::vec3 lb(-0.45f, 0.45f, 0.0f);
    AddQuad(verts, inds,
            lb + glm::vec3(-bRadius, -bRadius, 0.0f),
            lb + glm::vec3( bRadius, -bRadius, 0.0f),
            lb + glm::vec3( bRadius,  bRadius, barrelLen),
            lb + glm::vec3(-bRadius,  bRadius, barrelLen), glm::vec3(0.2f, 0.2f, 0.25f));

    // Right barrel
    glm::vec3 rb(0.45f, 0.45f, 0.0f);
    AddQuad(verts, inds,
            rb + glm::vec3(-bRadius, -bRadius, 0.0f),
            rb + glm::vec3( bRadius, -bRadius, 0.0f),
            rb + glm::vec3( bRadius,  bRadius, barrelLen),
            rb + glm::vec3(-bRadius,  bRadius, barrelLen), glm::vec3(0.2f, 0.2f, 0.25f));

    return Mesh(verts, inds);
}

Mesh Mesh::CreateBossCore(const glm::vec3& coreColor) {
    return CreateSphere(1.8f, 10, 12, coreColor);
}

Mesh Mesh::CreateRadarRelay(float size, const glm::vec3& baseColor, const glm::vec3& dishColor, const glm::vec3& beaconColor) {
    std::vector<Vertex> verts;
    std::vector<GLuint> inds;

    float s = size;
    // 1. Heavy Tripod Base Pedestal
    AddQuad(verts, inds,
            {-s * 0.9f, 0.0f, -s * 0.9f}, {s * 0.9f, 0.0f, -s * 0.9f},
            {s * 0.9f, 0.0f,  s * 0.9f}, {-s * 0.9f, 0.0f,  s * 0.9f}, baseColor * 0.7f);
    AddQuad(verts, inds,
            {-s * 0.5f, s * 0.5f, -s * 0.5f}, {s * 0.5f, s * 0.5f, -s * 0.5f},
            {s * 0.5f, s * 0.5f,  s * 0.5f}, {-s * 0.5f, s * 0.5f,  s * 0.5f}, baseColor * 1.1f);

    // 2. Main Central Mast / Spire
    AddQuad(verts, inds,
            {-s * 0.15f, s * 0.5f, -s * 0.15f}, {s * 0.15f, s * 0.5f, -s * 0.15f},
            {s * 0.15f, s * 1.8f, -s * 0.15f}, {-s * 0.15f, s * 1.8f, -s * 0.15f}, baseColor * 0.9f);
    AddQuad(verts, inds,
            {-s * 0.15f, s * 0.5f,  s * 0.15f}, {s * 0.15f, s * 0.5f,  s * 0.15f},
            {s * 0.15f, s * 1.8f,  s * 0.15f}, {-s * 0.15f, s * 1.8f,  s * 0.15f}, baseColor * 0.9f);

    // 3. Parabolic Radar Dish (Hexagonal curved dish)
    float dishY = s * 1.4f;
    float dishR = s * 1.1f;
    const int dishSegs = 8;
    for (int i = 0; i < dishSegs; ++i) {
        float a0 = (static_cast<float>(i) / dishSegs) * 6.2831853f;
        float a1 = (static_cast<float>(i + 1) / dishSegs) * 6.2831853f;
        glm::vec3 center(0.0f, dishY, 0.0f);
        glm::vec3 p0(std::cos(a0) * dishR, dishY + std::sin(a0) * dishR * 0.8f, 0.5f * s);
        glm::vec3 p1(std::cos(a1) * dishR, dishY + std::sin(a1) * dishR * 0.8f, 0.5f * s);
        AddTriangle(verts, inds, center, p0, p1, dishColor * (0.85f + 0.3f * (i % 2)));
    }

    // 4. Glowing Transmitter Beacon at Dish Center
    AddQuad(verts, inds,
            {-s * 0.12f, dishY - s * 0.12f, 0.6f * s}, {s * 0.12f, dishY - s * 0.12f, 0.6f * s},
            {s * 0.12f, dishY + s * 0.12f, 0.6f * s}, {-s * 0.12f, dishY + s * 0.12f, 0.6f * s}, beaconColor);
    AddQuad(verts, inds,
            {0.0f, dishY + s * 0.5f, 0.0f}, {0.0f, dishY + s * 0.9f, 0.0f},
            {0.0f, dishY + s * 0.9f, 0.2f * s}, {0.0f, dishY + s * 0.5f, 0.2f * s}, beaconColor);

    return Mesh(verts, inds);
}

Mesh Mesh::CreateSpaceDebris(float length, float width, const glm::vec3& color) {
    std::vector<Vertex> verts;
    std::vector<GLuint> inds;

    float hl = length * 0.5f;
    float hw = width * 0.5f;

    // Longitudinal truss beams
    AddQuad(verts, inds, {-hw, -hw, -hl}, {hw, -hw, -hl}, {hw, -hw, hl}, {-hw, -hw, hl}, color * 0.8f);
    AddQuad(verts, inds, {-hw,  hw, -hl}, {hw,  hw, -hl}, {hw,  hw, hl}, {-hw,  hw, hl}, color * 1.1f);
    AddQuad(verts, inds, {-hw, -hw, -hl}, {-hw,  hw, -hl}, {-hw,  hw, hl}, {-hw, -hw, hl}, color * 0.9f);
    AddQuad(verts, inds, { hw, -hw, -hl}, { hw,  hw, -hl}, { hw,  hw, hl}, { hw, -hw, hl}, color * 0.95f);

    // Shattered solar panel wing fragment attached to one side
    glm::vec3 panelColor(0.12f, 0.35f, 0.65f); // Iridescent solar cell blue
    AddQuad(verts, inds, {hw, 0.0f, -hl * 0.6f}, {hw + width * 2.5f, 0.0f, -hl * 0.4f},
            {hw + width * 2.8f, 0.0f, hl * 0.5f}, {hw, 0.0f, hl * 0.7f}, panelColor);

    return Mesh(verts, inds);
}

Mesh Mesh::CreateShadowDisc(float radius, int segments) {
    std::vector<Vertex> verts;
    std::vector<GLuint> inds;
    glm::vec3 n(0.0f, 1.0f, 0.0f);
    glm::vec3 c(0.02f, 0.04f, 0.08f);

    // Center vertex (index 0)
    verts.push_back({{0.0f, 0.0f, 0.0f}, n, c});

    // Perimeter vertices
    for (int i = 0; i < segments; ++i) {
        float angle = (float)i / segments * glm::two_pi<float>();
        float x = std::cos(angle) * radius;
        float z = std::sin(angle) * radius;
        verts.push_back({{x, 0.0f, z}, n, c * 0.4f});
    }

    for (int i = 0; i < segments; ++i) {
        int next = (i + 1) % segments;
        inds.push_back(0);
        inds.push_back(i + 1);
        inds.push_back(next + 1);
    }

    return Mesh(verts, inds);
}

Mesh Mesh::CreateFloatingDome(float radius, const glm::vec3& domeColor, const glm::vec3& ringColor) {
    std::vector<Vertex> verts;
    std::vector<GLuint> inds;

    const int segs = 16;
    float r = radius;
    float domeH = radius * 0.75f;

    // 1. Glowing Neon Equator Ring
    float ringThickness = radius * 0.16f;
    for (int i = 0; i < segs; ++i) {
        float a0 = (float)i / segs * glm::two_pi<float>();
        float a1 = (float)(i + 1) / segs * glm::two_pi<float>();

        glm::vec3 r0(std::cos(a0) * r, 0.0f, std::sin(a0) * r);
        glm::vec3 r1(std::cos(a1) * r, 0.0f, std::sin(a1) * r);
        glm::vec3 r0_up(std::cos(a0) * r, ringThickness, std::sin(a0) * r);
        glm::vec3 r1_up(std::cos(a1) * r, ringThickness, std::sin(a1) * r);

        AddQuad(verts, inds, r0, r1, r1_up, r0_up, ringColor);
    }

    // 2. Translucent Upper Observation Dome
    glm::vec3 apex(0.0f, domeH, 0.0f);
    for (int i = 0; i < segs; ++i) {
        float a0 = (float)i / segs * glm::two_pi<float>();
        float a1 = (float)(i + 1) / segs * glm::two_pi<float>();

        glm::vec3 r0_up(std::cos(a0) * r * 0.95f, ringThickness, std::sin(a0) * r * 0.95f);
        glm::vec3 r1_up(std::cos(a1) * r * 0.95f, ringThickness, std::sin(a1) * r * 0.95f);

        AddTriangle(verts, inds, apex, r0_up, r1_up, domeColor * (0.85f + 0.25f * (i % 2)));
    }

    // 3. Saucer Inverted Base Hull
    glm::vec3 baseApex(0.0f, -radius * 0.35f, 0.0f);
    glm::vec3 baseHullCol(0.12f, 0.14f, 0.20f);
    for (int i = 0; i < segs; ++i) {
        float a0 = (float)i / segs * glm::two_pi<float>();
        float a1 = (float)(i + 1) / segs * glm::two_pi<float>();

        glm::vec3 r0(std::cos(a0) * r, 0.0f, std::sin(a0) * r);
        glm::vec3 r1(std::cos(a1) * r, 0.0f, std::sin(a1) * r);

        AddTriangle(verts, inds, baseApex, r1, r0, baseHullCol);
    }

    return Mesh(verts, inds);
}

Mesh Mesh::CreateWindTurbine(float towerHeight, float bladeRadius,
                            const glm::vec3& towerColor, const glm::vec3& bladeColor) {
    return CreateWindTurbineTower(towerHeight, towerColor, glm::vec3(0.20f, 0.22f, 0.28f));
}

Mesh Mesh::CreateWindTurbineTower(float towerHeight, const glm::vec3& towerColor, const glm::vec3& nacelleColor) {
    std::vector<Vertex> verts;
    std::vector<GLuint> inds;

    // 1. Tapered Hexagonal Turbine Tower
    const int segs = 6;
    float rBase = 1.8f;
    float rTop = 0.95f;

    for (int i = 0; i < segs; ++i) {
        float a0 = (float)i / segs * glm::two_pi<float>();
        float a1 = (float)(i + 1) / segs * glm::two_pi<float>();

        glm::vec3 b0(std::cos(a0) * rBase, 0.0f, std::sin(a0) * rBase);
        glm::vec3 b1(std::cos(a1) * rBase, 0.0f, std::sin(a1) * rBase);
        glm::vec3 t0(std::cos(a0) * rTop, towerHeight, std::sin(a0) * rTop);
        glm::vec3 t1(std::cos(a1) * rTop, towerHeight, std::sin(a1) * rTop);

        float shade = 0.86f + 0.22f * (i % 2);
        AddQuad(verts, inds, b0, b1, t1, t0, towerColor * shade);
    }

    // 2. Nacelle Generator Housing Pod at Top
    float hubY = towerHeight;
    float nacelleLen = 4.2f;
    float nacelleRad = 1.25f;

    glm::vec3 p0(-nacelleRad, hubY - nacelleRad * 0.6f, -nacelleLen * 0.65f);
    glm::vec3 p1( nacelleRad, hubY - nacelleRad * 0.6f, -nacelleLen * 0.65f);
    glm::vec3 p2( nacelleRad, hubY + nacelleRad * 0.7f, -nacelleLen * 0.65f);
    glm::vec3 p3(-nacelleRad, hubY + nacelleRad * 0.7f, -nacelleLen * 0.65f);

    glm::vec3 f0(-nacelleRad * 0.85f, hubY - nacelleRad * 0.5f, nacelleLen * 0.45f);
    glm::vec3 f1( nacelleRad * 0.85f, hubY - nacelleRad * 0.5f, nacelleLen * 0.45f);
    glm::vec3 f2( nacelleRad * 0.85f, hubY + nacelleRad * 0.6f, nacelleLen * 0.45f);
    glm::vec3 f3(-nacelleRad * 0.85f, hubY + nacelleRad * 0.6f, nacelleLen * 0.45f);

    AddQuad(verts, inds, p0, p1, p2, p3, nacelleColor * 0.75f); // Back
    AddQuad(verts, inds, f1, f0, f3, f2, nacelleColor * 0.95f); // Front
    AddQuad(verts, inds, p1, f1, f2, p2, nacelleColor * 0.88f); // Right
    AddQuad(verts, inds, f0, p0, p3, f3, nacelleColor * 0.82f); // Left
    AddQuad(verts, inds, p3, p2, f2, f3, nacelleColor * 1.05f); // Top
    AddQuad(verts, inds, p0, f0, f1, p1, nacelleColor * 0.70f); // Bottom

    // Front axle hub socket extending forward to meet blades
    float socketZ = nacelleLen * 0.45f + 0.35f;
    glm::vec3 socketFront(0.0f, hubY, socketZ);
    for (int i = 0; i < 6; ++i) {
        float a0 = (float)i / 6 * glm::two_pi<float>();
        float a1 = (float)(i + 1) / 6 * glm::two_pi<float>();
        glm::vec3 s0(std::cos(a0) * 0.65f, hubY + std::sin(a0) * 0.65f, socketZ - 0.35f);
        glm::vec3 s1(std::cos(a1) * 0.65f, hubY + std::sin(a1) * 0.65f, socketZ - 0.35f);
        AddTriangle(verts, inds, socketFront, s0, s1, nacelleColor * 0.65f);
    }

    return Mesh(verts, inds);
}

Mesh Mesh::CreateWindTurbineBlades(float bladeRadius, const glm::vec3& bladeColor, const glm::vec3& tipColor) {
    std::vector<Vertex> verts;
    std::vector<GLuint> inds;

    // 1. Central Aerodynamic Spinner Cone pointing forward (+Z)
    float hubRadius = 1.05f;
    float hubDepth = 1.35f;
    glm::vec3 hubTip(0.0f, 0.0f, hubDepth);
    glm::vec3 hubCol(0.20f, 0.22f, 0.28f);

    const int hubSegs = 8;
    for (int i = 0; i < hubSegs; ++i) {
        float a0 = (float)i / hubSegs * glm::two_pi<float>();
        float a1 = (float)(i + 1) / hubSegs * glm::two_pi<float>();

        glm::vec3 h0(std::cos(a0) * hubRadius, std::sin(a0) * hubRadius, 0.0f);
        glm::vec3 h1(std::cos(a1) * hubRadius, std::sin(a1) * hubRadius, 0.0f);

        float shade = 0.85f + 0.25f * (i % 2);
        AddTriangle(verts, inds, hubTip, h0, h1, hubCol * shade);
    }

    // 2. Three Aerofoil Rotor Blades centered at origin in XY plane (Ex-Zodiac Image 3)
    float rootRadius = hubRadius * 0.70f;
    float bladeRootW = 0.90f;
    float bladeMidW = 0.72f;
    float bladeTipW = 0.48f;
    float tipRatio = 0.26f;

    for (int b = 0; b < 3; ++b) {
        float angle = (float)b / 3.0f * glm::two_pi<float>();
        glm::vec3 dir(std::cos(angle), std::sin(angle), 0.0f);
        glm::vec3 perp(-std::sin(angle), std::cos(angle), 0.0f);

        glm::vec3 r0 = dir * rootRadius - perp * (bladeRootW * 0.5f);
        glm::vec3 r1 = dir * rootRadius + perp * (bladeRootW * 0.5f);

        float midDist = rootRadius + (bladeRadius - rootRadius) * (1.0f - tipRatio);
        glm::vec3 m0 = dir * midDist - perp * (bladeMidW * 0.5f);
        glm::vec3 m1 = dir * midDist + perp * (bladeMidW * 0.5f);

        glm::vec3 t0 = dir * bladeRadius - perp * (bladeTipW * 0.5f);
        glm::vec3 t1 = dir * bladeRadius + perp * (bladeTipW * 0.5f);

        float bladeShade = 0.90f + 0.08f * b;
        AddQuad(verts, inds, r0, r1, m1, m0, bladeColor * bladeShade);
        AddQuad(verts, inds, m0, m1, t1, t0, tipColor);
    }

    return Mesh(verts, inds);
}

Mesh Mesh::CreateOpenFieldTerrain(float length, float width,
                                 const glm::vec3& floorColA, const glm::vec3& floorColB) {
    std::vector<Vertex> verts;
    std::vector<GLuint> inds;

    float hw = width * 0.5f;
    float hl = length * 0.5f;
    float floorY = -7.5f;

    // 20 columns across width, 12 rows along length
    const int xSegs = 20;
    const int zSegs = 12;
    float dx = width / xSegs;
    float dz = length / zSegs;

    for (int iz = 0; iz < zSegs; ++iz) {
        float z0 = -hl + iz * dz;
        float z1 = z0 + dz;

        for (int ix = 0; ix < xSegs; ++ix) {
            float x0 = -hw + ix * dx;
            float x1 = x0 + dx;

            // Two-tone checkerboard tile pattern (Ex-Zodiac signature arcade green plains)
            bool isTileA = ((ix + iz) % 2 == 0);
            glm::vec3 tileCol = isTileA ? floorColA : floorColB;

            // Rolling prairie contours across the vast open landscape
            float y00 = floorY + std::sin(ix * 0.75f + iz * 1.25f) * 0.12f;
            float y10 = floorY + std::sin((ix + 1) * 0.75f + iz * 1.25f) * 0.12f;
            float y11 = floorY + std::sin((ix + 1) * 0.75f + (iz + 1) * 1.25f) * 0.12f;
            float y01 = floorY + std::sin(ix * 0.75f + (iz + 1) * 1.25f) * 0.12f;

            // Gently ease outer edges to blend with horizon plane
            if (ix == 0) {
                y00 -= 2.2f;
                y01 -= 2.2f;
            } else if (ix == xSegs - 1) {
                y10 -= 2.2f;
                y11 -= 2.2f;
            }

            AddQuad(verts, inds,
                    {x0, y00, z0}, {x1, y10, z0}, {x1, y11, z1}, {x0, y01, z1},
                    tileCol);
        }
    }

    return Mesh(verts, inds);
}

Mesh Mesh::CreateLowPolyTree(float trunkHeight, float trunkRadius,
                            float foliageHeight, float foliageRadius,
                            const glm::vec3& trunkCol, const glm::vec3& foliageCol) {
    std::vector<Vertex> verts;
    std::vector<GLuint> inds;

    // 1. Tapered Hexagonal Tree Trunk
    const int segs = 6;
    float rBase = trunkRadius;
    float rTop = trunkRadius * 0.70f;

    for (int i = 0; i < segs; ++i) {
        float a0 = (float)i / segs * glm::two_pi<float>();
        float a1 = (float)(i + 1) / segs * glm::two_pi<float>();

        glm::vec3 b0(std::cos(a0) * rBase, 0.0f, std::sin(a0) * rBase);
        glm::vec3 b1(std::cos(a1) * rBase, 0.0f, std::sin(a1) * rBase);
        glm::vec3 t0(std::cos(a0) * rTop, trunkHeight, std::sin(a0) * rTop);
        glm::vec3 t1(std::cos(a1) * rTop, trunkHeight, std::sin(a1) * rTop);

        float shade = 0.82f + 0.28f * (i % 2);
        AddQuad(verts, inds, b0, b1, t1, t0, trunkCol * shade);
    }

    // 2. Three Tiered Low-Poly Polygonal Foliage Cones (Retro arcade pine trees)
    struct FoliageTier {
        float baseY;
        float height;
        float radius;
        float colorTint;
    };

    FoliageTier tiers[3] = {
        { trunkHeight * 0.65f, foliageHeight * 0.55f, foliageRadius, 0.86f },
        { trunkHeight * 0.65f + foliageHeight * 0.32f, foliageHeight * 0.50f, foliageRadius * 0.74f, 1.0f },
        { trunkHeight * 0.65f + foliageHeight * 0.62f, foliageHeight * 0.48f, foliageRadius * 0.48f, 1.18f }
    };

    for (int t = 0; t < 3; ++t) {
        float by = tiers[t].baseY;
        float py = by + tiers[t].height;
        float tr = tiers[t].radius;
        glm::vec3 peak(0.0f, py, 0.0f);
        glm::vec3 tierCol = foliageCol * tiers[t].colorTint;

        for (int i = 0; i < segs; ++i) {
            float a0 = (float)i / segs * glm::two_pi<float>();
            float a1 = (float)(i + 1) / segs * glm::two_pi<float>();

            glm::vec3 c0(std::cos(a0) * tr, by, std::sin(a0) * tr);
            glm::vec3 c1(std::cos(a1) * tr, by, std::sin(a1) * tr);

            float shade = 0.84f + 0.26f * (i % 2);
            AddTriangle(verts, inds, peak, c0, c1, tierCol * shade);
            AddTriangle(verts, inds, glm::vec3(0.0f, by, 0.0f), c1, c0, tierCol * 0.65f);
        }
    }

    return Mesh(verts, inds);
}

Mesh Mesh::CreateSpikedMace(float radius, float spikeLen,
                           const glm::vec3& coreColor, const glm::vec3& spikeColor) {
    std::vector<Vertex> verts;
    std::vector<GLuint> inds;

    float r = radius;

    // 1. Faceted Octahedron Core Sphere
    glm::vec3 top(0.0f, r, 0.0f);
    glm::vec3 bottom(0.0f, -r, 0.0f);
    glm::vec3 front(0.0f, 0.0f, r);
    glm::vec3 back(0.0f, 0.0f, -r);
    glm::vec3 left(-r, 0.0f, 0.0f);
    glm::vec3 right(r, 0.0f, 0.0f);

    AddTriangle(verts, inds, top, right, front, coreColor * 1.15f);
    AddTriangle(verts, inds, top, front, left, coreColor * 0.95f);
    AddTriangle(verts, inds, top, left, back, coreColor * 0.85f);
    AddTriangle(verts, inds, top, back, right, coreColor * 1.05f);

    AddTriangle(verts, inds, bottom, front, right, coreColor * 0.75f);
    AddTriangle(verts, inds, bottom, left, front, coreColor * 0.65f);
    AddTriangle(verts, inds, bottom, back, left, coreColor * 0.55f);
    AddTriangle(verts, inds, bottom, right, back, coreColor * 0.70f);

    // 2. Dangerous Conical/Pyramidal Spikes in 6 Principal Axes + 8 Diagonals
    std::vector<glm::vec3> spikeDirs = {
        { 1.0f,  0.0f,  0.0f}, {-1.0f,  0.0f,  0.0f},
        { 0.0f,  1.0f,  0.0f}, { 0.0f, -1.0f,  0.0f},
        { 0.0f,  0.0f,  1.0f}, { 0.0f,  0.0f, -1.0f},
        glm::normalize(glm::vec3( 1.0f,  1.0f,  1.0f)),
        glm::normalize(glm::vec3(-1.0f,  1.0f,  1.0f)),
        glm::normalize(glm::vec3( 1.0f, -1.0f,  1.0f)),
        glm::normalize(glm::vec3(-1.0f, -1.0f,  1.0f)),
        glm::normalize(glm::vec3( 1.0f,  1.0f, -1.0f)),
        glm::normalize(glm::vec3(-1.0f,  1.0f, -1.0f)),
        glm::normalize(glm::vec3( 1.0f, -1.0f, -1.0f)),
        glm::normalize(glm::vec3(-1.0f, -1.0f, -1.0f))
    };

    float baseW = radius * 0.28f;
    for (const auto& d : spikeDirs) {
        glm::vec3 tip = d * (radius + spikeLen);
        glm::vec3 up = std::abs(d.y) > 0.9f ? glm::vec3(1, 0, 0) : glm::vec3(0, 1, 0);
        glm::vec3 u = glm::normalize(glm::cross(d, up)) * baseW;
        glm::vec3 v = glm::normalize(glm::cross(d, u)) * baseW;
        glm::vec3 b = d * radius;

        AddTriangle(verts, inds, tip, b + u, b + v, spikeColor);
        AddTriangle(verts, inds, tip, b + v, b - u, spikeColor * 0.9f);
        AddTriangle(verts, inds, tip, b - u, b - v, spikeColor * 0.8f);
        AddTriangle(verts, inds, tip, b - v, b + u, spikeColor * 0.95f);
    }

    return Mesh(verts, inds);
}

Mesh Mesh::CreateGroundTank(const glm::vec3& treadCol, const glm::vec3& hullCol, const glm::vec3& turretCol) {
    std::vector<Vertex> verts;
    std::vector<GLuint> inds;

    // 1. Left and Right Caterpillar Treads
    float treadW = 0.95f;
    float treadH = 0.85f;
    float treadLen = 3.8f;
    float halfLen = treadLen * 0.5f;

    for (int side = -1; side <= 1; side += 2) {
        float cx = side * 1.75f;
        float x0 = cx - treadW * 0.5f;
        float x1 = cx + treadW * 0.5f;

        // Beveled tread loop
        glm::vec3 b0(x0, 0.0f, -halfLen + 0.5f);
        glm::vec3 b1(x1, 0.0f, -halfLen + 0.5f);
        glm::vec3 b2(x1, 0.0f,  halfLen - 0.5f);
        glm::vec3 b3(x0, 0.0f,  halfLen - 0.5f);

        glm::vec3 t0(x0, treadH, -halfLen + 0.4f);
        glm::vec3 t1(x1, treadH, -halfLen + 0.4f);
        glm::vec3 t2(x1, treadH,  halfLen - 0.4f);
        glm::vec3 t3(x0, treadH,  halfLen - 0.4f);

        glm::vec3 front(cx, treadH * 0.45f, halfLen);
        glm::vec3 back(cx, treadH * 0.45f, -halfLen);

        // Top and bottom track runs
        AddQuad(verts, inds, t0, t1, t2, t3, treadCol * 0.85f);
        AddQuad(verts, inds, b3, b2, b1, b0, treadCol * 0.55f);
        // Sloped front/back track runs
        AddQuad(verts, inds, b3, b2, t2, t3, treadCol * 0.95f);
        AddQuad(verts, inds, t0, t1, b1, b0, treadCol * 0.75f);
        // Outer track sides
        glm::vec3 outCol = treadCol * (side < 0 ? 0.80f : 1.05f);
        if (side < 0) {
            AddQuad(verts, inds, b0, t0, t3, b3, outCol);
        } else {
            AddQuad(verts, inds, b2, t2, t1, b1, outCol);
        }
    }

    // 2. Sloped Armored Tank Chassis / Hull
    float hw = 1.35f;
    float hullBotY = 0.45f;
    float hullTopY = 1.25f;
    float hFrontZ = 1.8f;
    float hBackZ = -1.8f;

    glm::vec3 hb0(-hw, hullBotY, hBackZ);
    glm::vec3 hb1( hw, hullBotY, hBackZ);
    glm::vec3 hb2( hw, hullBotY, hFrontZ);
    glm::vec3 hb3(-hw, hullBotY, hFrontZ);

    float tw = 1.05f;
    glm::vec3 ht0(-tw, hullTopY, hBackZ + 0.3f);
    glm::vec3 ht1( tw, hullTopY, hBackZ + 0.3f);
    glm::vec3 ht2( tw, hullTopY, hFrontZ - 0.5f);
    glm::vec3 ht3(-tw, hullTopY, hFrontZ - 0.5f);

    AddQuad(verts, inds, ht0, ht1, ht2, ht3, hullCol * 1.15f); // Roof
    AddQuad(verts, inds, hb3, hb2, ht2, ht3, hullCol * 1.25f); // Front glacis (sloped)
    AddQuad(verts, inds, hb1, hb0, ht0, ht1, hullCol * 0.75f); // Rear armor
    AddQuad(verts, inds, hb0, hb3, ht3, ht0, hullCol * 0.85f); // Left flank
    AddQuad(verts, inds, hb2, hb1, ht1, ht2, hullCol * 1.05f); // Right flank

    // 3. Rotating Turret Cupola
    float turRad = 0.95f;
    float turY = hullTopY;
    float turH = 0.65f;
    glm::vec3 turTop(0.0f, turY + turH, 0.0f);

    for (int i = 0; i < 6; ++i) {
        float a0 = (float)i / 6.0f * glm::two_pi<float>();
        float a1 = (float)(i + 1) / 6.0f * glm::two_pi<float>();

        glm::vec3 c0(std::cos(a0) * turRad, turY, std::sin(a0) * turRad);
        glm::vec3 c1(std::cos(a1) * turRad, turY, std::sin(a1) * turRad);
        glm::vec3 ct0(std::cos(a0) * turRad * 0.75f, turY + turH, std::sin(a0) * turRad * 0.75f);
        glm::vec3 ct1(std::cos(a1) * turRad * 0.75f, turY + turH, std::sin(a1) * turRad * 0.75f);

        float shade = 0.85f + 0.25f * (i % 2);
        AddQuad(verts, inds, c0, c1, ct1, ct0, turretCol * shade);
        AddTriangle(verts, inds, turTop, ct0, ct1, turretCol * 1.2f);
    }

    // 4. Twin Elevated Heavy Cannon Barrels (+Z, elevated ~14 degrees)
    float barrelLen = 2.4f;
    float bRadius = 0.16f;
    float elevY = std::sin(glm::radians(14.0f)) * barrelLen;
    float forwardZ = std::cos(glm::radians(14.0f)) * barrelLen;

    for (float bx : {-0.32f, 0.32f}) {
        glm::vec3 root(bx, turY + turH * 0.55f, 0.6f);
        glm::vec3 tip(bx, root.y + elevY, root.z + forwardZ);

        for (int i = 0; i < 4; ++i) {
            float a0 = (float)i / 4.0f * glm::two_pi<float>();
            float a1 = (float)(i + 1) / 4.0f * glm::two_pi<float>();
            glm::vec3 r0 = root + glm::vec3(std::cos(a0) * bRadius, std::sin(a0) * bRadius, 0.0f);
            glm::vec3 r1 = root + glm::vec3(std::cos(a1) * bRadius, std::sin(a1) * bRadius, 0.0f);
            glm::vec3 t0 = tip  + glm::vec3(std::cos(a0) * bRadius, std::sin(a0) * bRadius, 0.0f);
            glm::vec3 t1 = tip  + glm::vec3(std::cos(a1) * bRadius, std::sin(a1) * bRadius, 0.0f);

            glm::vec3 barrelCol(0.18f, 0.20f, 0.24f);
            AddQuad(verts, inds, r0, r1, t1, t0, barrelCol * (0.8f + 0.2f * (i % 2)));
        }
        // Glowing orange muzzle tip
        AddQuad(verts, inds,
                tip + glm::vec3(-bRadius, -bRadius, 0.0f),
                tip + glm::vec3( bRadius, -bRadius, 0.0f),
                tip + glm::vec3( bRadius,  bRadius, 0.0f),
                tip + glm::vec3(-bRadius,  bRadius, 0.0f),
                glm::vec3(1.0f, 0.4f, 0.1f));
    }

    return Mesh(verts, inds);
}

Mesh Mesh::CreateFlakTurret(const glm::vec3& bunkerCol, const glm::vec3& barrelCol) {
    std::vector<Vertex> verts;
    std::vector<GLuint> inds;

    // 1. Octagonal Fortified Pillbox Bunker Base
    const int segs = 8;
    float rBase = 2.4f;
    float rTop = 1.75f;
    float bunkerH = 1.35f;

    for (int i = 0; i < segs; ++i) {
        float a0 = (float)i / segs * glm::two_pi<float>();
        float a1 = (float)(i + 1) / segs * glm::two_pi<float>();

        glm::vec3 b0(std::cos(a0) * rBase, 0.0f, std::sin(a0) * rBase);
        glm::vec3 b1(std::cos(a1) * rBase, 0.0f, std::sin(a1) * rBase);
        glm::vec3 t0(std::cos(a0) * rTop, bunkerH, std::sin(a0) * rTop);
        glm::vec3 t1(std::cos(a1) * rTop, bunkerH, std::sin(a1) * rTop);

        float shade = 0.82f + 0.26f * (i % 2);
        AddQuad(verts, inds, b0, b1, t1, t0, bunkerCol * shade);
    }

    // 2. Swivel Dome Cupola
    float domeH = 0.8f;
    float domeRad = 1.25f;
    glm::vec3 domeApex(0.0f, bunkerH + domeH, 0.0f);

    for (int i = 0; i < segs; ++i) {
        float a0 = (float)i / segs * glm::two_pi<float>();
        float a1 = (float)(i + 1) / segs * glm::two_pi<float>();

        glm::vec3 d0(std::cos(a0) * domeRad, bunkerH, std::sin(a0) * domeRad);
        glm::vec3 d1(std::cos(a1) * domeRad, bunkerH, std::sin(a1) * domeRad);

        float shade = 0.88f + 0.22f * (i % 2);
        AddTriangle(verts, inds, domeApex, d0, d1, bunkerCol * 1.18f * shade);
    }

    // 3. Dual Anti-Air Flak Cannon Barrels (Elevated ~32 degrees into flight corridor)
    float flakLen = 2.8f;
    float fRadius = 0.14f;
    float elevY = std::sin(glm::radians(32.0f)) * flakLen;
    float forwardZ = std::cos(glm::radians(32.0f)) * flakLen;

    for (float bx : {-0.42f, 0.42f}) {
        glm::vec3 root(bx, bunkerH + domeH * 0.45f, 0.4f);
        glm::vec3 tip(bx, root.y + elevY, root.z + forwardZ);

        for (int i = 0; i < 4; ++i) {
            float a0 = (float)i / 4.0f * glm::two_pi<float>();
            float a1 = (float)(i + 1) / 4.0f * glm::two_pi<float>();
            glm::vec3 r0 = root + glm::vec3(std::cos(a0) * fRadius, std::sin(a0) * fRadius, 0.0f);
            glm::vec3 r1 = root + glm::vec3(std::cos(a1) * fRadius, std::sin(a1) * fRadius, 0.0f);
            glm::vec3 t0 = tip  + glm::vec3(std::cos(a0) * fRadius, std::sin(a0) * fRadius, 0.0f);
            glm::vec3 t1 = tip  + glm::vec3(std::cos(a1) * fRadius, std::sin(a1) * fRadius, 0.0f);

            AddQuad(verts, inds, r0, r1, t1, t0, barrelCol * (0.85f + 0.18f * (i % 2)));
        }

        // Flak Muzzle Flash Suppressor Funnel
        float mRad = fRadius * 1.7f;
        glm::vec3 muzzleExt = tip + glm::normalize(tip - root) * 0.35f;
        AddQuad(verts, inds,
                tip + glm::vec3(-mRad, -mRad, 0.0f),
                tip + glm::vec3( mRad, -mRad, 0.0f),
                muzzleExt + glm::vec3( mRad,  mRad, 0.0f),
                muzzleExt + glm::vec3(-mRad,  mRad, 0.0f),
                glm::vec3(0.98f, 0.25f, 0.15f));
    }

    return Mesh(verts, inds);
}

Mesh Mesh::CreateEliteInterceptor(const glm::vec3& hullCol, const glm::vec3& accentCol, const glm::vec3& cockpitCol) {
    std::vector<Vertex> verts;
    std::vector<GLuint> inds;

    // 1. Sleek Aggressive Fuselage (Needle nose forward (+Z))
    glm::vec3 nose(0.0f, 0.0f, 2.6f);
    glm::vec3 midTop(0.0f, 0.55f, 0.2f);
    glm::vec3 midBot(0.0f, -0.45f, 0.2f);
    glm::vec3 midL(-0.85f, 0.0f, 0.2f);
    glm::vec3 midR( 0.85f, 0.0f, 0.2f);

    glm::vec3 tailTop(0.0f, 0.40f, -2.2f);
    glm::vec3 tailBot(0.0f, -0.35f, -2.2f);
    glm::vec3 tailL(-0.65f, 0.0f, -2.2f);
    glm::vec3 tailR( 0.65f, 0.0f, -2.2f);

    // Forward fuselage facets
    AddTriangle(verts, inds, nose, midTop, midR, hullCol * 1.15f);
    AddTriangle(verts, inds, nose, midL, midTop, hullCol * 0.95f);
    AddTriangle(verts, inds, nose, midR, midBot, hullCol * 0.85f);
    AddTriangle(verts, inds, nose, midBot, midL, hullCol * 0.75f);

    // Mid to tail fuselage
    AddQuad(verts, inds, midTop, midR, tailR, tailTop, hullCol * 1.05f);
    AddQuad(verts, inds, midL, midTop, tailTop, tailL, hullCol * 0.90f);
    AddQuad(verts, inds, midR, midBot, tailBot, tailR, hullCol * 0.70f);
    AddQuad(verts, inds, midBot, midL, tailL, tailBot, hullCol * 0.60f);

    // 2. Forward-Swept Wings with Dihedral Anhedral Angle (Star Wolf / Bird of Prey aesthetic)
    float span = 3.6f;
    float wingTipZ = 1.1f; // Sweeps forward!
    float wingRootZ = -0.5f;

    // Right wing
    glm::vec3 rRoot0( 0.75f, 0.0f, wingRootZ + 0.8f);
    glm::vec3 rRoot1( 0.65f, 0.0f, wingRootZ - 1.2f);
    glm::vec3 rTip0(  span, -0.35f, wingTipZ);
    glm::vec3 rTip1(  span + 0.4f, -0.45f, wingTipZ - 0.7f);
    AddQuad(verts, inds, rRoot0, rRoot1, rTip1, rTip0, accentCol * 1.10f);
    AddQuad(verts, inds, rTip0, rTip1, rRoot1, rRoot0, accentCol * 0.85f);

    // Left wing
    glm::vec3 lRoot0(-0.75f, 0.0f, wingRootZ + 0.8f);
    glm::vec3 lRoot1(-0.65f, 0.0f, wingRootZ - 1.2f);
    glm::vec3 lTip0( -span, -0.35f, wingTipZ);
    glm::vec3 lTip1( -span - 0.4f, -0.45f, wingTipZ - 0.7f);
    AddQuad(verts, inds, lRoot1, lRoot0, lTip0, lTip1, accentCol * 0.92f);
    AddQuad(verts, inds, lTip1, lTip0, lRoot0, lRoot1, accentCol * 0.78f);

    // Wingtip Plasma Pods
    for (float wx : {-span, span}) {
        glm::vec3 podCenter(wx, -0.38f, wingTipZ - 0.2f);
        glm::vec3 podTip(wx, -0.38f, wingTipZ + 0.5f);
        for (int i = 0; i < 4; ++i) {
            float a0 = (float)i / 4.0f * glm::two_pi<float>();
            float a1 = (float)(i + 1) / 4.0f * glm::two_pi<float>();
            glm::vec3 c0 = podCenter + glm::vec3(std::cos(a0) * 0.18f, std::sin(a0) * 0.18f, -0.4f);
            glm::vec3 c1 = podCenter + glm::vec3(std::cos(a1) * 0.18f, std::sin(a1) * 0.18f, -0.4f);
            AddTriangle(verts, inds, podTip, c0, c1, glm::vec3(1.0f, 0.2f, 0.15f));
        }
    }

    // 3. Cockpit Canopy Glass (Luminous Amber / Crimson)
    glm::vec3 cNose(0.0f, 0.25f, 1.4f);
    glm::vec3 cPeak(0.0f, 0.65f, 0.5f);
    glm::vec3 cRear(0.0f, 0.48f, -0.3f);
    glm::vec3 cL(-0.35f, 0.32f, 0.4f);
    glm::vec3 cR( 0.35f, 0.32f, 0.4f);

    AddTriangle(verts, inds, cPeak, cNose, cR, cockpitCol * 1.25f);
    AddTriangle(verts, inds, cPeak, cL, cNose, cockpitCol * 1.05f);
    AddTriangle(verts, inds, cPeak, cR, cRear, cockpitCol * 0.90f);
    AddTriangle(verts, inds, cPeak, cRear, cL, cockpitCol * 0.80f);

    // 4. Twin Afterburner Thruster Rings at Tail
    for (float tx : {-0.28f, 0.28f}) {
        glm::vec3 ex(tx, 0.0f, -2.3f);
        AddQuad(verts, inds,
                ex + glm::vec3(-0.20f, -0.20f, 0.0f),
                ex + glm::vec3( 0.20f, -0.20f, 0.0f),
                ex + glm::vec3( 0.20f,  0.20f, 0.0f),
                ex + glm::vec3(-0.20f,  0.20f, 0.0f),
                glm::vec3(0.2f, 0.85f, 1.0f)); // Electric cyan exhaust glow
    }

    return Mesh(verts, inds);
}
