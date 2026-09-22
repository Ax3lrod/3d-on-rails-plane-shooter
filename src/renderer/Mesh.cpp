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
                                 const glm::vec3& floorColA, const glm::vec3& floorColB,
                                 int rowOffset) {
    std::vector<Vertex> verts;
    std::vector<GLuint> inds;

    float hw = width  * 0.5f;
    float hl = length * 0.5f;
    float floorY = -7.5f;

    // Ex-Zodiac / Star Fox checkerboard:
    // tileSize=20 divides evenly into sliceLen=60 (3 rows exactly).
    // rowOffset lets adjacent slabs alternate parity for seamless continuity.
    const float tileSize = 20.0f;
    const int xSegs = static_cast<int>(std::round(width  / tileSize)); // 17 cols for width=340
    const int zSegs = static_cast<int>(std::round(length / tileSize)); // 3 rows for length=60

    float dx = width  / xSegs;
    float dz = length / zSegs;

    // Darken colB heavily so the two tiles have strong contrast (arcade look)
    glm::vec3 colA = floorColA;
    glm::vec3 colB = floorColB * 0.62f;

    for (int iz = 0; iz < zSegs; ++iz) {
        float z0 = -hl + iz * dz;
        float z1 = z0 + dz;

        for (int ix = 0; ix < xSegs; ++ix) {
            float x0 = -hw + ix * dx;
            float x1 = x0 + dx;

            // rowOffset shifts parity so slab N+1 continues from where slab N left off
            bool isTileA = ((ix + iz + rowOffset) % 2 == 0);
            glm::vec3 tileCol = isTileA ? colA : colB;

            // Perfectly flat — Ex-Zodiac ground has no vertex waviness
            AddQuad(verts, inds,
                    {x0, floorY, z0}, {x1, floorY, z0},
                    {x1, floorY, z1}, {x0, floorY, z1},
                    tileCol);
        }
    }

    // Side berms: slightly raised colored edge strips — gives the "road track" feel
    float bermH = 2.2f;
    float bermW = 8.0f;
    glm::vec3 bermCol = floorColA * 0.55f; // dark toned berm, biome-neutral

    // Left berm top
    AddQuad(verts, inds,
            {-hw - bermW, floorY + bermH, -hl}, {-hw, floorY + bermH, -hl},
            {-hw,         floorY + bermH,  hl}, {-hw - bermW, floorY + bermH, hl},
            bermCol);
    // Left berm inner wall
    AddQuad(verts, inds,
            {-hw, floorY,         -hl}, {-hw, floorY + bermH, -hl},
            {-hw, floorY + bermH,  hl}, {-hw, floorY,          hl},
            bermCol * 0.78f);

    // Right berm top
    AddQuad(verts, inds,
            {hw,         floorY + bermH, -hl}, {hw + bermW, floorY + bermH, -hl},
            {hw + bermW, floorY + bermH,  hl}, {hw,         floorY + bermH,  hl},
            bermCol);
    // Right berm inner wall
    AddQuad(verts, inds,
            {hw, floorY + bermH, -hl}, {hw, floorY,        -hl},
            {hw, floorY,          hl}, {hw, floorY + bermH,  hl},
            bermCol * 0.78f);

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

// -------------------------------------------------------------
// Phase 33 & 34: Train & Boss Procedural Meshes
// -------------------------------------------------------------

Mesh Mesh::CreateTrainLocomotive(const glm::vec3& bodyCol, const glm::vec3& trimCol) {
    std::vector<Vertex> verts;
    std::vector<GLuint> inds;

    // 1. Wheelbase & Under-chassis
    float len = 9.0f;
    float w = 2.4f;
    float h = 0.6f;
    glm::vec3 c0(-w*0.5f, 0.2f, -len*0.5f);
    glm::vec3 c1( w*0.5f, 0.2f, -len*0.5f);
    glm::vec3 c2( w*0.5f, 0.2f + h, -len*0.5f);
    glm::vec3 c3(-w*0.5f, 0.2f + h, -len*0.5f);
    glm::vec3 cf0(-w*0.5f, 0.2f, len*0.5f);
    glm::vec3 cf1( w*0.5f, 0.2f, len*0.5f);
    glm::vec3 cf2( w*0.5f, 0.2f + h, len*0.5f);
    glm::vec3 cf3(-w*0.5f, 0.2f + h, len*0.5f);

    AddQuad(verts, inds, c0, c1, c2, c3, bodyCol * 0.7f);
    AddQuad(verts, inds, cf1, cf0, cf3, cf2, bodyCol * 0.9f);
    AddQuad(verts, inds, c1, cf1, cf2, c2, bodyCol * 0.8f);
    AddQuad(verts, inds, cf0, c0, c3, cf3, bodyCol * 0.75f);
    AddQuad(verts, inds, c3, c2, cf2, cf3, bodyCol);

    // 2. Cowcatcher / Wedge Plow in Front (+Z)
    float plowLen = 1.4f;
    glm::vec3 pTip(0.0f, 0.15f, len*0.5f + plowLen);
    glm::vec3 pL(-w*0.55f, 0.15f, len*0.5f);
    glm::vec3 pR( w*0.55f, 0.15f, len*0.5f);
    glm::vec3 pTop(0.0f, 0.2f + h*1.4f, len*0.5f);
    AddTriangle(verts, inds, pTip, pR, pTop, trimCol * 1.1f);
    AddTriangle(verts, inds, pTip, pTop, pL, trimCol * 0.9f);
    AddTriangle(verts, inds, pL, pTop, cf3, trimCol * 0.8f);
    AddTriangle(verts, inds, pTop, pR, cf2, trimCol * 1.0f);

    // 3. Engine Boiler / Hood
    float bW = 1.8f;
    float bH = 1.6f;
    float bL = 5.2f;
    glm::vec3 b0(-bW*0.5f, 0.2f + h, 0.0f);
    glm::vec3 b1( bW*0.5f, 0.2f + h, 0.0f);
    glm::vec3 b2( bW*0.5f, 0.2f + h + bH, 0.0f);
    glm::vec3 b3(-bW*0.5f, 0.2f + h + bH, 0.0f);
    glm::vec3 bf0(-bW*0.5f, 0.2f + h, bL);
    glm::vec3 bf1( bW*0.5f, 0.2f + h, bL);
    glm::vec3 bf2( bW*0.5f, 0.2f + h + bH, bL);
    glm::vec3 bf3(-bW*0.5f, 0.2f + h + bH, bL);

    AddQuad(verts, inds, bf1, bf0, bf3, bf2, bodyCol * 1.15f); // Front
    AddQuad(verts, inds, b1, bf1, bf2, b2, bodyCol * 0.95f);   // Right
    AddQuad(verts, inds, bf0, b0, b3, bf3, bodyCol * 0.85f);   // Left
    AddQuad(verts, inds, b3, b2, bf2, bf3, bodyCol * 1.25f);   // Top

    // Glowing front headlight
    glm::vec3 hl(0.0f, 0.2f + h + bH*0.5f, bL + 0.1f);
    float hlR = 0.35f;
    AddQuad(verts, inds,
            hl + glm::vec3(-hlR, -hlR, 0.0f),
            hl + glm::vec3( hlR, -hlR, 0.0f),
            hl + glm::vec3( hlR,  hlR, 0.0f),
            hl + glm::vec3(-hlR,  hlR, 0.0f),
            glm::vec3(1.0f, 0.92f, 0.4f)); // Radiant gold light

    // 4. Operator Cabin at Rear (-Z)
    float cW = 2.2f;
    float cH = 2.4f;
    float cL = 3.6f;
    glm::vec3 r0(-cW*0.5f, 0.2f + h, -len*0.5f);
    glm::vec3 r1( cW*0.5f, 0.2f + h, -len*0.5f);
    glm::vec3 r2( cW*0.5f, 0.2f + h + cH, -len*0.5f);
    glm::vec3 r3(-cW*0.5f, 0.2f + h + cH, -len*0.5f);
    glm::vec3 rf0(-cW*0.5f, 0.2f + h, 0.0f);
    glm::vec3 rf1( cW*0.5f, 0.2f + h, 0.0f);
    glm::vec3 rf2( cW*0.5f, 0.2f + h + cH, 0.0f);
    glm::vec3 rf3(-cW*0.5f, 0.2f + h + cH, 0.0f);

    AddQuad(verts, inds, r0, r1, r2, r3, trimCol * 0.8f);    // Back
    AddQuad(verts, inds, rf1, rf0, rf3, rf2, trimCol * 1.0f); // Forward
    AddQuad(verts, inds, r1, rf1, rf2, r2, trimCol * 0.9f);   // Right
    AddQuad(verts, inds, rf0, r0, r3, rf3, trimCol * 0.85f);  // Left
    AddQuad(verts, inds, r3, r2, rf2, rf3, trimCol * 1.15f);  // Roof

    // Cabin Windows
    glm::vec3 winCol(0.2f, 0.85f, 1.0f);
    AddQuad(verts, inds,
            glm::vec3(-cW*0.4f, 0.2f + h + cH*0.6f, 0.02f),
            glm::vec3( cW*0.4f, 0.2f + h + cH*0.6f, 0.02f),
            glm::vec3( cW*0.4f, 0.2f + h + cH*0.85f, 0.02f),
            glm::vec3(-cW*0.4f, 0.2f + h + cH*0.85f, 0.02f),
            winCol);

    // 5. Smokestack Exhaust Pipe
    float sY = 0.2f + h + bH;
    float sH = 0.85f;
    float sR = 0.25f;
    glm::vec3 sPos(0.0f, sY, bL * 0.7f);
    for (int i = 0; i < 6; ++i) {
        float a0 = (float)i / 6.0f * glm::two_pi<float>();
        float a1 = (float)(i + 1) / 6.0f * glm::two_pi<float>();
        glm::vec3 t0 = sPos + glm::vec3(std::cos(a0)*sR, 0.0f, std::sin(a0)*sR);
        glm::vec3 t1 = sPos + glm::vec3(std::cos(a1)*sR, 0.0f, std::sin(a1)*sR);
        glm::vec3 u0 = t0 + glm::vec3(0.0f, sH, 0.0f);
        glm::vec3 u1 = t1 + glm::vec3(0.0f, sH, 0.0f);
        AddQuad(verts, inds, t0, t1, u1, u0, glm::vec3(0.2f, 0.22f, 0.25f));
    }

    return Mesh(verts, inds);
}

Mesh Mesh::CreateTrainCargoCar(const glm::vec3& carCol, const glm::vec3& containerCol) {
    std::vector<Vertex> verts;
    std::vector<GLuint> inds;

    float len = 8.5f;
    float w = 2.4f;
    float baseH = 0.5f;

    // Flatbed Base
    glm::vec3 b0(-w*0.5f, 0.2f, -len*0.5f);
    glm::vec3 b1( w*0.5f, 0.2f, -len*0.5f);
    glm::vec3 b2( w*0.5f, 0.2f + baseH, -len*0.5f);
    glm::vec3 b3(-w*0.5f, 0.2f + baseH, -len*0.5f);
    glm::vec3 f0(-w*0.5f, 0.2f,  len*0.5f);
    glm::vec3 f1( w*0.5f, 0.2f,  len*0.5f);
    glm::vec3 f2( w*0.5f, 0.2f + baseH,  len*0.5f);
    glm::vec3 f3(-w*0.5f, 0.2f + baseH,  len*0.5f);

    AddQuad(verts, inds, b0, b1, b2, b3, carCol * 0.7f);
    AddQuad(verts, inds, f1, f0, f3, f2, carCol * 0.9f);
    AddQuad(verts, inds, b1, f1, f2, b2, carCol * 0.8f);
    AddQuad(verts, inds, f0, b0, b3, f3, carCol * 0.75f);
    AddQuad(verts, inds, b3, b2, f2, f3, carCol);

    // Armored Shipping Container Box
    float cW = 2.2f;
    float cH = 2.2f;
    float cL = 7.8f;
    float y0 = 0.2f + baseH;
    float y1 = y0 + cH;

    glm::vec3 c0(-cW*0.5f, y0, -cL*0.5f);
    glm::vec3 c1( cW*0.5f, y0, -cL*0.5f);
    glm::vec3 c2( cW*0.5f, y1, -cL*0.5f);
    glm::vec3 c3(-cW*0.5f, y1, -cL*0.5f);
    glm::vec3 cf0(-cW*0.5f, y0, cL*0.5f);
    glm::vec3 cf1( cW*0.5f, y0, cL*0.5f);
    glm::vec3 cf2( cW*0.5f, y1, cL*0.5f);
    glm::vec3 cf3(-cW*0.5f, y1, cL*0.5f);

    AddQuad(verts, inds, c0, c1, c2, c3, containerCol * 0.85f);
    AddQuad(verts, inds, cf1, cf0, cf3, cf2, containerCol * 1.05f);
    AddQuad(verts, inds, c1, cf1, cf2, c2, containerCol * 0.95f);
    AddQuad(verts, inds, cf0, c0, c3, cf3, containerCol * 0.80f);
    AddQuad(verts, inds, c3, c2, cf2, cf3, containerCol * 1.15f);

    return Mesh(verts, inds);
}

Mesh Mesh::CreateRailTracks(float length, float gauge, const glm::vec3& railCol, const glm::vec3& tieCol) {
    std::vector<Vertex> verts;
    std::vector<GLuint> inds;

    float halfLen = length * 0.5f;
    float railW = 0.12f;
    float railH = 0.15f;
    float halfG = gauge * 0.5f;

    // 1. Two continuous steel rails
    for (float rx : {-halfG, halfG}) {
        glm::vec3 r0(rx - railW*0.5f, 0.12f, -halfLen);
        glm::vec3 r1(rx + railW*0.5f, 0.12f, -halfLen);
        glm::vec3 r2(rx + railW*0.5f, 0.12f + railH, -halfLen);
        glm::vec3 r3(rx - railW*0.5f, 0.12f + railH, -halfLen);

        glm::vec3 f0(rx - railW*0.5f, 0.12f, halfLen);
        glm::vec3 f1(rx + railW*0.5f, 0.12f, halfLen);
        glm::vec3 f2(rx + railW*0.5f, 0.12f + railH, halfLen);
        glm::vec3 f3(rx - railW*0.5f, 0.12f + railH, halfLen);

        AddQuad(verts, inds, r3, r2, f2, f3, railCol * 1.25f); // Top surface
        AddQuad(verts, inds, r1, f1, f2, r2, railCol * 0.95f); // Outer
        AddQuad(verts, inds, f0, r0, r3, f3, railCol * 0.85f); // Inner
    }

    // 2. Cross ties spaced every 2.4m
    float tieSpacing = 2.4f;
    int tieCount = static_cast<int>(length / tieSpacing);
    float tieW = gauge * 1.4f;
    float tieLen = 0.5f;
    float tieH = 0.12f;

    for (int i = 0; i < tieCount; ++i) {
        float tz = -halfLen + i * tieSpacing;
        glm::vec3 t0(-tieW*0.5f, 0.0f, tz - tieLen*0.5f);
        glm::vec3 t1( tieW*0.5f, 0.0f, tz - tieLen*0.5f);
        glm::vec3 t2( tieW*0.5f, tieH, tz - tieLen*0.5f);
        glm::vec3 t3(-tieW*0.5f, tieH, tz - tieLen*0.5f);

        glm::vec3 tf0(-tieW*0.5f, 0.0f, tz + tieLen*0.5f);
        glm::vec3 tf1( tieW*0.5f, 0.0f, tz + tieLen*0.5f);
        glm::vec3 tf2( tieW*0.5f, tieH, tz + tieLen*0.5f);
        glm::vec3 tf3(-tieW*0.5f, tieH, tz + tieLen*0.5f);

        AddQuad(verts, inds, t3, t2, tf2, tf3, tieCol * (0.85f + 0.15f * (i % 2)));
        AddQuad(verts, inds, tf1, tf0, tf3, tf2, tieCol * 0.7f);
        AddQuad(verts, inds, t0, t1, t2, t3, tieCol * 0.7f);
    }

    return Mesh(verts, inds);
}

Mesh Mesh::CreateHelicopterFuselage(const glm::vec3& bodyCol, const glm::vec3& canopyCol) {
    std::vector<Vertex> verts;
    std::vector<GLuint> inds;

    // 1. Sleek Attack Helicopter Fuselage
    glm::vec3 nose(0.0f, -0.2f, 3.2f);
    glm::vec3 midTop(0.0f, 0.9f, 0.4f);
    glm::vec3 midBot(0.0f, -0.7f, 0.4f);
    glm::vec3 midL(-1.1f, 0.0f, 0.4f);
    glm::vec3 midR( 1.1f, 0.0f, 0.4f);

    glm::vec3 tailRootTop(0.0f, 0.4f, -2.5f);
    glm::vec3 tailRootBot(0.0f, -0.3f, -2.5f);
    glm::vec3 tailRootL(-0.5f, 0.0f, -2.5f);
    glm::vec3 tailRootR( 0.5f, 0.0f, -2.5f);

    // Nose facets
    AddTriangle(verts, inds, nose, midTop, midR, bodyCol * 1.15f);
    AddTriangle(verts, inds, nose, midL, midTop, bodyCol * 0.95f);
    AddTriangle(verts, inds, nose, midR, midBot, bodyCol * 0.85f);
    AddTriangle(verts, inds, nose, midBot, midL, bodyCol * 0.75f);

    // Mid section
    AddQuad(verts, inds, midTop, midR, tailRootR, tailRootTop, bodyCol * 1.05f);
    AddQuad(verts, inds, midL, midTop, tailRootTop, tailRootL, bodyCol * 0.90f);
    AddQuad(verts, inds, midR, midBot, tailRootBot, tailRootR, bodyCol * 0.70f);
    AddQuad(verts, inds, midBot, midL, tailRootL, tailRootBot, bodyCol * 0.60f);

    // 2. Long Tail Boom & Vertical Fin (-Z)
    float boomLen = 4.2f;
    glm::vec3 tailTip(0.0f, 0.2f, -2.5f - boomLen);
    glm::vec3 finTop(0.0f, 1.6f, -2.5f - boomLen * 0.9f);
    glm::vec3 finBot(0.0f, -0.4f, -2.5f - boomLen);

    AddTriangle(verts, inds, tailRootTop, tailTip, finTop, bodyCol * 1.1f);
    AddTriangle(verts, inds, tailRootBot, finBot, tailTip, bodyCol * 0.75f);
    AddTriangle(verts, inds, tailRootL, tailTip, tailRootTop, bodyCol * 0.85f);
    AddTriangle(verts, inds, tailRootR, tailRootTop, tailTip, bodyCol * 1.0f);

    // 3. Stub Weapon Wings with Dual Rocket Pods
    float wingSpan = 2.4f;
    for (float side : {-1.0f, 1.0f}) {
        glm::vec3 wRoot(side * 0.9f, 0.0f, 0.8f);
        glm::vec3 wTip(side * wingSpan, -0.1f, 0.6f);
        glm::vec3 wRootR(side * 0.9f, 0.0f, -0.2f);
        glm::vec3 wTipR(side * wingSpan, -0.1f, -0.3f);

        AddQuad(verts, inds, wRoot, wTip, wTipR, wRootR, bodyCol * 0.95f);

        // Rocket Pod Cylinder
        glm::vec3 podCenter = (wTip + wTipR) * 0.5f + glm::vec3(0.0f, -0.3f, 0.0f);
        for (int i = 0; i < 6; ++i) {
            float a0 = (float)i / 6.0f * glm::two_pi<float>();
            float a1 = (float)(i + 1) / 6.0f * glm::two_pi<float>();
            glm::vec3 p0 = podCenter + glm::vec3(std::cos(a0)*0.25f, std::sin(a0)*0.25f, -0.8f);
            glm::vec3 p1 = podCenter + glm::vec3(std::cos(a1)*0.25f, std::sin(a1)*0.25f, -0.8f);
            glm::vec3 q0 = podCenter + glm::vec3(std::cos(a0)*0.25f, std::sin(a0)*0.25f,  0.8f);
            glm::vec3 q1 = podCenter + glm::vec3(std::cos(a1)*0.25f, std::sin(a1)*0.25f,  0.8f);
            AddQuad(verts, inds, p0, p1, q1, q0, glm::vec3(0.22f, 0.24f, 0.28f));
            // Red missile tips
            AddTriangle(verts, inds, podCenter + glm::vec3(0.0f, 0.0f, 0.95f), q0, q1, glm::vec3(0.95f, 0.2f, 0.15f));
        }
    }

    // 4. Tandem Cockpit Glass Canopy
    glm::vec3 cpNose(0.0f, 0.1f, 2.2f);
    glm::vec3 cpMid(0.0f, 0.75f, 1.1f);
    glm::vec3 cpRear(0.0f, 0.65f, 0.0f);
    glm::vec3 cpL(-0.55f, 0.35f, 1.0f);
    glm::vec3 cpR( 0.55f, 0.35f, 1.0f);

    AddTriangle(verts, inds, cpMid, cpNose, cpR, canopyCol * 1.25f);
    AddTriangle(verts, inds, cpMid, cpL, cpNose, canopyCol * 1.05f);
    AddTriangle(verts, inds, cpMid, cpR, cpRear, canopyCol * 0.95f);
    AddTriangle(verts, inds, cpMid, cpRear, cpL, canopyCol * 0.85f);

    // 5. Chin Gatling Minigun
    glm::vec3 chinGun(0.0f, -0.85f, 2.4f);
    glm::vec3 gunTip(0.0f, -0.85f, 3.4f);
    AddQuad(verts, inds,
            chinGun + glm::vec3(-0.08f, -0.08f, 0.0f),
            chinGun + glm::vec3( 0.08f, -0.08f, 0.0f),
            gunTip  + glm::vec3( 0.08f,  0.08f, 0.0f),
            gunTip  + glm::vec3(-0.08f,  0.08f, 0.0f),
            glm::vec3(0.18f, 0.20f, 0.22f));

    return Mesh(verts, inds);
}

Mesh Mesh::CreateHelicopterMainRotor(float radius, const glm::vec3& bladeCol) {
    std::vector<Vertex> verts;
    std::vector<GLuint> inds;

    // Central hub
    float hubR = 0.45f;
    for (int i = 0; i < 6; ++i) {
        float a0 = (float)i / 6.0f * glm::two_pi<float>();
        float a1 = (float)(i + 1) / 6.0f * glm::two_pi<float>();
        AddTriangle(verts, inds,
                    glm::vec3(0.0f, 0.15f, 0.0f),
                    glm::vec3(std::cos(a0)*hubR, 0.0f, std::sin(a0)*hubR),
                    glm::vec3(std::cos(a1)*hubR, 0.0f, std::sin(a1)*hubR),
                    glm::vec3(0.2f, 0.22f, 0.26f));
    }

    // 4 Aerofoil Rotor Blades (spinning in XZ plane)
    float bladeW = 0.42f;
    for (int b = 0; b < 4; ++b) {
        float angle = (float)b / 4.0f * glm::two_pi<float>();
        glm::vec3 dir(std::cos(angle), 0.0f, std::sin(angle));
        glm::vec3 perp(-std::sin(angle), 0.0f, std::cos(angle));

        glm::vec3 r0 = dir * hubR - perp * (bladeW * 0.5f);
        glm::vec3 r1 = dir * hubR + perp * (bladeW * 0.5f);
        glm::vec3 t0 = dir * radius - perp * (bladeW * 0.35f);
        glm::vec3 t1 = dir * radius + perp * (bladeW * 0.35f);

        AddQuad(verts, inds, r0, r1, t1, t0, bladeCol);
        // Yellow caution tip
        AddQuad(verts, inds,
                dir * (radius * 0.85f) - perp * (bladeW * 0.38f),
                dir * (radius * 0.85f) + perp * (bladeW * 0.38f),
                t1, t0, glm::vec3(1.0f, 0.85f, 0.1f));
    }

    return Mesh(verts, inds);
}

Mesh Mesh::CreateHelicopterTailRotor(float radius, const glm::vec3& bladeCol) {
    std::vector<Vertex> verts;
    std::vector<GLuint> inds;

    float bladeW = 0.16f;
    for (int b = 0; b < 2; ++b) {
        float angle = (float)b / 2.0f * glm::pi<float>();
        glm::vec3 dir(0.0f, std::cos(angle), std::sin(angle));
        glm::vec3 perp(0.0f, -std::sin(angle), std::cos(angle));

        glm::vec3 r0 = -dir * radius - perp * (bladeW * 0.5f);
        glm::vec3 r1 = -dir * radius + perp * (bladeW * 0.5f);
        glm::vec3 t0 =  dir * radius - perp * (bladeW * 0.5f);
        glm::vec3 t1 =  dir * radius + perp * (bladeW * 0.5f);

        AddQuad(verts, inds, r0, r1, t1, t0, bladeCol);
    }
    return Mesh(verts, inds);
}

Mesh Mesh::CreateMegaTankChassis(const glm::vec3& treadCol, const glm::vec3& armorCol) {
    std::vector<Vertex> verts;
    std::vector<GLuint> inds;

    // Massive 18m long land fortress chassis
    float len = 18.0f;
    float hw = 5.5f;
    float h = 3.2f;

    // 1. Giant Quad Treads (Left Front, Left Rear, Right Front, Right Rear)
    for (float side : {-1.0f, 1.0f}) {
        float cx = side * (hw + 1.2f);
        float tw = 2.2f;
        glm::vec3 t0(cx - tw*0.5f, 0.0f, -len*0.48f);
        glm::vec3 t1(cx + tw*0.5f, 0.0f, -len*0.48f);
        glm::vec3 t2(cx + tw*0.5f, 2.2f, -len*0.44f);
        glm::vec3 t3(cx - tw*0.5f, 2.2f, -len*0.44f);

        glm::vec3 tf0(cx - tw*0.5f, 0.0f, len*0.48f);
        glm::vec3 tf1(cx + tw*0.5f, 0.0f, len*0.48f);
        glm::vec3 tf2(cx + tw*0.5f, 2.2f, len*0.44f);
        glm::vec3 tf3(cx - tw*0.5f, 2.2f, len*0.44f);

        AddQuad(verts, inds, t3, t2, tf2, tf3, treadCol * 0.85f);
        AddQuad(verts, inds, tf0, tf1, t1, t0, treadCol * 0.55f);
        AddQuad(verts, inds, t1, tf1, tf2, t2, treadCol * (side < 0 ? 0.8f : 1.1f));
        AddQuad(verts, inds, tf0, t0, t3, tf3, treadCol * (side < 0 ? 1.1f : 0.8f));
        AddQuad(verts, inds, tf3, tf2, tf1, tf0, treadCol * 0.95f);
        AddQuad(verts, inds, t0, t1, t2, t3, treadCol * 0.75f);
    }

    // 2. Heavy Armored Citadel Hull
    glm::vec3 b0(-hw, 1.2f, -len*0.5f);
    glm::vec3 b1( hw, 1.2f, -len*0.5f);
    glm::vec3 b2( hw, 1.2f + h, -len*0.42f);
    glm::vec3 b3(-hw, 1.2f + h, -len*0.42f);

    glm::vec3 f0(-hw, 1.2f,  len*0.5f);
    glm::vec3 f1( hw, 1.2f,  len*0.5f);
    glm::vec3 f2( hw, 1.2f + h,  len*0.42f);
    glm::vec3 f3(-hw, 1.2f + h,  len*0.42f);

    AddQuad(verts, inds, b3, b2, f2, f3, armorCol * 1.15f); // Deck
    AddQuad(verts, inds, f0, f1, f2, f3, armorCol * 1.25f); // Glacis
    AddQuad(verts, inds, b1, b0, b3, b2, armorCol * 0.75f); // Rear
    AddQuad(verts, inds, b0, f0, f3, b3, armorCol * 0.85f); // Left
    AddQuad(verts, inds, f1, b1, b2, f2, armorCol * 1.05f); // Right

    return Mesh(verts, inds);
}

Mesh Mesh::CreateMegaTankTurret(const glm::vec3& turretCol, const glm::vec3& barrelCol) {
    std::vector<Vertex> verts;
    std::vector<GLuint> inds;

    // Rotating heavy gun cupola
    float r = 2.4f;
    float h = 1.8f;
    for (int i = 0; i < 8; ++i) {
        float a0 = (float)i / 8.0f * glm::two_pi<float>();
        float a1 = (float)(i + 1) / 8.0f * glm::two_pi<float>();
        glm::vec3 b0(std::cos(a0)*r, 0.0f, std::sin(a0)*r);
        glm::vec3 b1(std::cos(a1)*r, 0.0f, std::sin(a1)*r);
        glm::vec3 t0(std::cos(a0)*r*0.8f, h, std::sin(a0)*r*0.8f);
        glm::vec3 t1(std::cos(a1)*r*0.8f, h, std::sin(a1)*r*0.8f);
        AddQuad(verts, inds, b0, b1, t1, t0, turretCol * (0.85f + 0.18f*(i%2)));
        AddTriangle(verts, inds, glm::vec3(0.0f, h + 0.3f, 0.0f), t0, t1, turretCol * 1.2f);
    }

    // Dual Gigantic Barrels (+Z)
    float bLen = 6.5f;
    float bRad = 0.32f;
    for (float bx : {-0.75f, 0.75f}) {
        glm::vec3 root(bx, h * 0.6f, 1.2f);
        glm::vec3 tip(bx, h * 0.6f + 0.5f, 1.2f + bLen);
        for (int i = 0; i < 4; ++i) {
            float a0 = (float)i / 4.0f * glm::two_pi<float>();
            float a1 = (float)(i + 1) / 4.0f * glm::two_pi<float>();
            glm::vec3 r0 = root + glm::vec3(std::cos(a0)*bRad, std::sin(a0)*bRad, 0.0f);
            glm::vec3 r1 = root + glm::vec3(std::cos(a1)*bRad, std::sin(a1)*bRad, 0.0f);
            glm::vec3 t0 = tip  + glm::vec3(std::cos(a0)*bRad, std::sin(a0)*bRad, 0.0f);
            glm::vec3 t1 = tip  + glm::vec3(std::cos(a1)*bRad, std::sin(a1)*bRad, 0.0f);
            AddQuad(verts, inds, r0, r1, t1, t0, barrelCol * (0.8f + 0.2f*(i%2)));
        }
        // Glowing orange muzzle
        AddQuad(verts, inds,
                tip + glm::vec3(-bRad, -bRad, 0.0f),
                tip + glm::vec3( bRad, -bRad, 0.0f),
                tip + glm::vec3( bRad,  bRad, 0.0f),
                tip + glm::vec3(-bRad,  bRad, 0.0f),
                glm::vec3(1.0f, 0.45f, 0.1f));
    }

    return Mesh(verts, inds);
}

Mesh Mesh::CreateMegaTankCore(const glm::vec3& coreCol) {
    std::vector<Vertex> verts;
    std::vector<GLuint> inds;

    float r = 1.6f;
    glm::vec3 top(0.0f, r, 0.0f);
    glm::vec3 bot(0.0f, -r, 0.0f);
    const int segs = 8;
    for (int i = 0; i < segs; ++i) {
        float a0 = (float)i / segs * glm::two_pi<float>();
        float a1 = (float)(i + 1) / segs * glm::two_pi<float>();
        glm::vec3 p0(std::cos(a0)*r, 0.0f, std::sin(a0)*r);
        glm::vec3 p1(std::cos(a1)*r, 0.0f, std::sin(a1)*r);
        AddTriangle(verts, inds, top, p0, p1, coreCol * 1.25f);
        AddTriangle(verts, inds, bot, p1, p0, coreCol * 0.85f);
    }
    return Mesh(verts, inds);
}

Mesh Mesh::CreateWormHead(const glm::vec3& headCol, const glm::vec3& mandibleCol, const glm::vec3& eyeCol) {
    std::vector<Vertex> verts;
    std::vector<GLuint> inds;

    // 1. Armored triangular head dome
    float r = 2.6f;
    glm::vec3 apex(0.0f, 0.2f, 3.4f);
    glm::vec3 bTop(0.0f, 2.0f, 0.0f);
    glm::vec3 bBot(0.0f, -1.8f, 0.0f);
    glm::vec3 bL(-r, 0.0f, 0.0f);
    glm::vec3 bR( r, 0.0f, 0.0f);

    AddTriangle(verts, inds, apex, bTop, bR, headCol * 1.15f);
    AddTriangle(verts, inds, apex, bL, bTop, headCol * 0.95f);
    AddTriangle(verts, inds, apex, bR, bBot, headCol * 0.85f);
    AddTriangle(verts, inds, apex, bBot, bL, headCol * 0.75f);

    // 2. Left and Right Massive Curved Mandible Tusk Pincers
    for (float side : {-1.0f, 1.0f}) {
        glm::vec3 mRoot(side * (r * 0.6f), -0.4f, 1.8f);
        glm::vec3 mMid(side * (r * 1.3f), -0.2f, 3.2f);
        glm::vec3 mTip(side * (r * 0.2f),  0.1f, 4.6f); // Curves inward!

        AddTriangle(verts, inds, mRoot, mMid, mTip, mandibleCol * 1.1f);
        AddTriangle(verts, inds, mRoot, mTip, mMid + glm::vec3(0.0f, -0.6f, 0.0f), mandibleCol * 0.85f);
    }

    // 3. Multi-cluster Optic Sensors
    for (float ex : {-0.85f, 0.0f, 0.85f}) {
        glm::vec3 eyePos(ex, 0.85f - std::abs(ex)*0.3f, 2.2f);
        float eR = (ex == 0.0f) ? 0.38f : 0.28f;
        AddQuad(verts, inds,
                eyePos + glm::vec3(-eR, -eR, 0.0f),
                eyePos + glm::vec3( eR, -eR, 0.0f),
                eyePos + glm::vec3( eR,  eR, 0.0f),
                eyePos + glm::vec3(-eR,  eR, 0.0f),
                eyeCol);
    }

    return Mesh(verts, inds);
}

Mesh Mesh::CreateWormSegment(float radius, const glm::vec3& armorCol, const glm::vec3& coreCol) {
    std::vector<Vertex> verts;
    std::vector<GLuint> inds;

    float r = radius;
    const int segs = 8;

    // Overlapping armored ring plate
    for (int i = 0; i < segs; ++i) {
        float a0 = (float)i / segs * glm::two_pi<float>();
        float a1 = (float)(i + 1) / segs * glm::two_pi<float>();

        glm::vec3 f0(std::cos(a0)*r, std::sin(a0)*r, 1.1f);
        glm::vec3 f1(std::cos(a1)*r, std::sin(a1)*r, 1.1f);
        glm::vec3 b0(std::cos(a0)*r*0.85f, std::sin(a0)*r*0.85f, -1.1f);
        glm::vec3 b1(std::cos(a1)*r*0.85f, std::sin(a1)*r*0.85f, -1.1f);

        float shade = 0.84f + 0.26f * (i % 2);
        AddQuad(verts, inds, f0, f1, b1, b0, armorCol * shade);
    }

    // Glowing ventral underbelly power node
    glm::vec3 nodePos(0.0f, -r * 0.82f, 0.0f);
    float nodeR = r * 0.38f;
    AddQuad(verts, inds,
            nodePos + glm::vec3(-nodeR, 0.0f, -nodeR),
            nodePos + glm::vec3( nodeR, 0.0f, -nodeR),
            nodePos + glm::vec3( nodeR, 0.0f,  nodeR),
            nodePos + glm::vec3(-nodeR, 0.0f,  nodeR),
            coreCol);

    return Mesh(verts, inds);
}

// ============================================================
// Phase 35: Biome Prop & Horizon Mesh Factories
// ============================================================

// CreateIceCrystal: cluster of 5 hexagonal spire prisms arranged in a pentagon
Mesh Mesh::CreateIceCrystal(float height, float radius, const glm::vec3& iceCol, const glm::vec3& coreCol) {
    std::vector<Vertex> verts;
    std::vector<GLuint> inds;

    float angles[5]  = { 0.0f, 72.0f, 144.0f, 216.0f, 288.0f };
    float heights[5] = { height, height*0.55f, height*0.85f, height*0.45f, height*0.70f };
    float offsets[5] = { 0.0f, radius*0.55f, radius*0.35f, radius*0.65f, radius*0.25f };
    float baseY = -7.5f;

    for (int c = 0; c < 5; ++c) {
        float a  = glm::radians(angles[c]);
        float cx = std::cos(a) * offsets[c];
        float cz = std::sin(a) * offsets[c];
        float h  = heights[c];
        float r  = radius * 0.22f;

        for (int s = 0; s < 6; ++s) {
            float a0 = glm::radians(s * 60.0f);
            float a1 = glm::radians((s + 1) * 60.0f);
            glm::vec3 b0(cx + std::cos(a0)*r, baseY,   cz + std::sin(a0)*r);
            glm::vec3 b1(cx + std::cos(a1)*r, baseY,   cz + std::sin(a1)*r);
            glm::vec3 apex(cx, baseY + h, cz);
            glm::vec3 faceCol = (s % 2 == 0) ? iceCol : glm::mix(iceCol, coreCol, 0.45f);
            AddTriangle(verts, inds, b0, b1, apex, faceCol);
            // Floor cap
            AddTriangle(verts, inds, glm::vec3(cx, baseY, cz), b1, b0, iceCol * 0.45f);
        }
    }
    return Mesh(verts, inds);
}

// CreateCactus: 6-sided cylinder trunk with two branching arms
Mesh Mesh::CreateCactus(float trunkHeight, float trunkRadius, float armHeight, const glm::vec3& cactusCol) {
    std::vector<Vertex> verts;
    std::vector<GLuint> inds;

    float baseY = -7.5f;
    const int segs = 6;

    auto addCylinder = [&](glm::vec3 bot, glm::vec3 top, float r, glm::vec3 col) {
        for (int s = 0; s < segs; ++s) {
            float a0 = glm::radians(s * 360.0f / segs);
            float a1 = glm::radians((s + 1) * 360.0f / segs);
            glm::vec3 b0v = bot + glm::vec3(std::cos(a0)*r, 0.0f, std::sin(a0)*r);
            glm::vec3 b1v = bot + glm::vec3(std::cos(a1)*r, 0.0f, std::sin(a1)*r);
            glm::vec3 t0v = top + glm::vec3(std::cos(a0)*r, 0.0f, std::sin(a0)*r);
            glm::vec3 t1v = top + glm::vec3(std::cos(a1)*r, 0.0f, std::sin(a1)*r);
            float shade = 0.82f + 0.18f * (float)s / segs;
            AddQuad(verts, inds, b0v, b1v, t1v, t0v, col * shade);
            AddTriangle(verts, inds, top, t0v, t1v, col * 0.70f);
        }
    };

    addCylinder(glm::vec3(0.0f, baseY, 0.0f), glm::vec3(0.0f, baseY + trunkHeight, 0.0f), trunkRadius, cactusCol);

    float branchY = baseY + trunkHeight * 0.60f;
    // Left arm
    addCylinder(glm::vec3(-trunkRadius*0.5f, branchY, 0.0f), glm::vec3(-trunkHeight*0.35f, branchY, 0.0f),         trunkRadius*0.65f, cactusCol * 0.88f);
    addCylinder(glm::vec3(-trunkHeight*0.35f, branchY, 0.0f), glm::vec3(-trunkHeight*0.35f, branchY+armHeight, 0.0f), trunkRadius*0.55f, cactusCol * 0.88f);
    // Right arm (offset slightly higher)
    float rOY = armHeight * 0.15f;
    addCylinder(glm::vec3(trunkRadius*0.5f, branchY+rOY, 0.0f), glm::vec3(trunkHeight*0.35f, branchY+rOY, 0.0f),           trunkRadius*0.65f, cactusCol * 0.88f);
    addCylinder(glm::vec3(trunkHeight*0.35f, branchY+rOY, 0.0f), glm::vec3(trunkHeight*0.35f, branchY+rOY+armHeight*0.70f, 0.0f), trunkRadius*0.55f, cactusCol * 0.88f);

    return Mesh(verts, inds);
}

// CreateDesertPyramid: 4-sided pyramid with directional sun/shadow shading
Mesh Mesh::CreateDesertPyramid(float baseSize, float height, const glm::vec3& stoneCol, const glm::vec3& shadowCol) {
    std::vector<Vertex> verts;
    std::vector<GLuint> inds;

    float h = baseSize * 0.5f;
    float baseY = -7.5f;
    glm::vec3 apex(0.0f, baseY + height, 0.0f);
    glm::vec3 bl(-h, baseY, -h), br( h, baseY, -h);
    glm::vec3 fr( h, baseY,  h), fl(-h, baseY,  h);

    AddTriangle(verts, inds, fl, fr, apex, stoneCol);                          // Front (sunlit)
    AddTriangle(verts, inds, fr, br, apex, glm::mix(stoneCol, shadowCol, 0.40f)); // Right
    AddTriangle(verts, inds, br, bl, apex, shadowCol);                         // Back (shadow)
    AddTriangle(verts, inds, bl, fl, apex, glm::mix(stoneCol, shadowCol, 0.28f)); // Left
    AddQuad(verts, inds, fl, bl, br, fr, stoneCol * 0.35f);                   // Base

    return Mesh(verts, inds);
}

// CreateBiomeHorizon: coarse flat-shaded pixel-art style horizon per biome theme
Mesh Mesh::CreateBiomeHorizon(const std::string& biome, float radius, float height,
                               const glm::vec3& skyTopCol, const glm::vec3& horizonCol) {
    std::vector<Vertex> verts;
    std::vector<GLuint> inds;

    // 16 segments for chunky look (half the resolution of the old ArcadeHorizon)
    const int hSegs = 16;
    float startAngle = -glm::radians(88.0f);
    float endAngle   =  glm::radians(88.0f);
    float dAngle = (endAngle - startAngle) / hSegs;

    float yBot = -30.0f, yMid = 40.0f, yTop = height;
    glm::vec3 colBot = horizonCol;
    glm::vec3 colMid = glm::mix(horizonCol, skyTopCol, 0.5f);

    for (int i = 0; i < hSegs; ++i) {
        float a0 = startAngle + i * dAngle, a1 = a0 + dAngle;
        float s0 = std::sin(a0), c0 = std::cos(a0);
        float s1 = std::sin(a1), c1 = std::cos(a1);
        glm::vec3 p0b(s0*radius, yBot, -c0*radius), p1b(s1*radius, yBot, -c1*radius);
        glm::vec3 p0m(s0*radius, yMid, -c0*radius), p1m(s1*radius, yMid, -c1*radius);
        glm::vec3 p0t(s0*radius, yTop, -c0*radius), p1t(s1*radius, yTop, -c1*radius);
        AddQuad(verts, inds, p0b, p1b, p1m, p0m, colBot);
        AddQuad(verts, inds, p0m, p1m, p1t, p0t, colMid);
    }

    float sRadius = radius * 0.92f, baseY = -8.0f;

    if (biome == "dune_pass") {
        glm::vec3 pyrCol(0.75f, 0.62f, 0.35f), pyrShd(0.55f, 0.42f, 0.22f), duneCol(0.82f, 0.68f, 0.42f);
        float pyA[3] = {-0.6f, 0.1f, 0.75f}, pyW[3] = {0.38f, 0.50f, 0.30f}, pyH[3] = {60.0f, 90.0f, 45.0f};
        for (int p = 0; p < 3; ++p) {
            float aL = pyA[p]-pyW[p], aR = pyA[p]+pyW[p], am = (aL+aR)*0.5f;
            glm::vec3 bL(std::sin(aL)*sRadius, baseY, -std::cos(aL)*sRadius);
            glm::vec3 bR(std::sin(aR)*sRadius, baseY, -std::cos(aR)*sRadius);
            glm::vec3 pyApex(std::sin(am)*sRadius, baseY+pyH[p], -std::cos(am)*sRadius);
            glm::vec3 midP = (bL+bR)*0.5f;
            AddTriangle(verts, inds, bL, midP, pyApex, pyrCol);
            AddTriangle(verts, inds, midP, bR, pyApex, pyrShd);
        }
        float dA[5] = {-1.1f, -0.3f, 0.35f, 0.9f, 1.3f};
        for (int d = 0; d < 5; ++d) {
            float hw = 0.28f, dh = 18.0f + (d%3)*8.0f;
            glm::vec3 dL(std::sin(dA[d]-hw)*sRadius, baseY, -std::cos(dA[d]-hw)*sRadius);
            glm::vec3 dR(std::sin(dA[d]+hw)*sRadius, baseY, -std::cos(dA[d]+hw)*sRadius);
            glm::vec3 dApex(std::sin(dA[d])*(sRadius*0.97f), baseY+dh, -std::cos(dA[d])*(sRadius*0.97f));
            AddTriangle(verts, inds, dL, dR, dApex, duneCol);
        }
    } else if (biome == "glacial") {
        glm::vec3 iceC(0.75f, 0.90f, 1.0f), iceS(0.55f, 0.72f, 0.88f), snowC(0.96f, 0.98f, 1.0f);
        for (int s = 0; s < 12; ++s) {
            float ac = -1.4f + s*0.25f, hw = 0.08f + (s%3)*0.04f, sh = 35.0f + (s%4)*22.0f + (s%2)*15.0f;
            glm::vec3 sL(std::sin(ac-hw)*sRadius, baseY, -std::cos(ac-hw)*sRadius);
            glm::vec3 sR(std::sin(ac+hw)*sRadius, baseY, -std::cos(ac+hw)*sRadius);
            glm::vec3 sA(std::sin(ac)*(sRadius*0.96f), baseY+sh, -std::cos(ac)*(sRadius*0.96f));
            glm::vec3 midI = (sL+sR)*0.5f;
            AddTriangle(verts, inds, sL, midI, sA, (s%2==0) ? iceC : iceS);
            AddTriangle(verts, inds, midI, sR, sA, (s%2==0) ? iceS : iceC);
            if (sh > 60.0f) {
                glm::vec3 cL = glm::mix(sA, sL, 0.22f), cR = glm::mix(sA, sR, 0.22f);
                AddTriangle(verts, inds, cL, sA, cR, snowC);
            }
        }
        for (int s = 0; s < 8; ++s) {
            float ac = -1.1f + s*0.32f;
            glm::vec3 iL(std::sin(ac-0.18f)*sRadius, baseY, -std::cos(ac-0.18f)*sRadius);
            glm::vec3 iR(std::sin(ac+0.18f)*sRadius, baseY, -std::cos(ac+0.18f)*sRadius);
            glm::vec3 iT(std::sin(ac-0.12f)*(sRadius*0.97f), baseY+12.0f, -std::cos(ac-0.12f)*(sRadius*0.97f));
            glm::vec3 iT2(std::sin(ac+0.12f)*(sRadius*0.97f), baseY+12.0f, -std::cos(ac+0.12f)*(sRadius*0.97f));
            AddQuad(verts, inds, iL, iR, iT2, iT, iceS * 0.78f);
        }
    } else if (biome == "iron_fortress") {
        glm::vec3 basaltC(0.22f, 0.24f, 0.28f), battleC(0.30f, 0.32f, 0.38f), lavaC(0.9f, 0.3f, 0.1f);
        float tA[3] = {-0.8f, 0.0f, 0.85f}, tW[3] = {0.20f, 0.30f, 0.18f}, tH[3] = {55.0f, 80.0f, 48.0f};
        for (int t = 0; t < 3; ++t) {
            glm::vec3 ttL(std::sin(tA[t]-tW[t])*sRadius, baseY, -std::cos(tA[t]-tW[t])*sRadius);
            glm::vec3 ttR(std::sin(tA[t]+tW[t])*sRadius, baseY, -std::cos(tA[t]+tW[t])*sRadius);
            glm::vec3 ttTL(std::sin(tA[t]-tW[t])*(sRadius*0.97f), baseY+tH[t], -std::cos(tA[t]-tW[t])*(sRadius*0.97f));
            glm::vec3 ttTR(std::sin(tA[t]+tW[t])*(sRadius*0.97f), baseY+tH[t], -std::cos(tA[t]+tW[t])*(sRadius*0.97f));
            AddQuad(verts, inds, ttL, ttR, ttTR, ttTL, basaltC);
            for (int n = 0; n < 3; ++n) {
                float na = glm::mix(tA[t]-tW[t], tA[t]+tW[t], (n+0.5f)/3.0f), nw = tW[t]*0.12f;
                glm::vec3 nL(std::sin(na-nw)*(sRadius*0.97f), baseY+tH[t], -std::cos(na-nw)*(sRadius*0.97f));
                glm::vec3 nR(std::sin(na+nw)*(sRadius*0.97f), baseY+tH[t], -std::cos(na+nw)*(sRadius*0.97f));
                glm::vec3 nTL(std::sin(na-nw)*(sRadius*0.96f), baseY+tH[t]+12.0f, -std::cos(na-nw)*(sRadius*0.96f));
                glm::vec3 nTR(std::sin(na+nw)*(sRadius*0.96f), baseY+tH[t]+12.0f, -std::cos(na+nw)*(sRadius*0.96f));
                AddQuad(verts, inds, nL, nR, nTR, nTL, battleC);
            }
        }
        for (int w = 0; w < 2; ++w) {
            float wA = tA[w]+tW[w], wB = tA[w+1]-tW[w+1];
            glm::vec3 wL(std::sin(wA)*sRadius, baseY, -std::cos(wA)*sRadius);
            glm::vec3 wR(std::sin(wB)*sRadius, baseY, -std::cos(wB)*sRadius);
            glm::vec3 wTL(std::sin(wA)*(sRadius*0.97f), baseY+32.0f, -std::cos(wA)*(sRadius*0.97f));
            glm::vec3 wTR(std::sin(wB)*(sRadius*0.97f), baseY+32.0f, -std::cos(wB)*(sRadius*0.97f));
            AddQuad(verts, inds, wL, wR, wTR, wTL, basaltC * 0.85f);
        }
        for (int l = 0; l < hSegs; ++l) {
            float a0 = startAngle + l*dAngle, a1 = a0+dAngle;
            glm::vec3 lb(std::sin(a0)*sRadius, baseY-2.0f, -std::cos(a0)*sRadius);
            glm::vec3 lb2(std::sin(a1)*sRadius, baseY-2.0f, -std::cos(a1)*sRadius);
            glm::vec3 lt(std::sin(a0)*(sRadius*0.99f), baseY+6.0f, -std::cos(a0)*(sRadius*0.99f));
            glm::vec3 lt2(std::sin(a1)*(sRadius*0.99f), baseY+6.0f, -std::cos(a1)*(sRadius*0.99f));
            AddQuad(verts, inds, lb, lb2, lt2, lt, lavaC);
        }
    } else if (biome == "city") {
        // Urban skyline: irregular building silhouettes on the horizon
        glm::vec3 skylineCol(0.08f, 0.08f, 0.12f); // near-black buildings
        glm::vec3 neonCol(0.25f, 0.60f, 0.95f);    // blue neon glow
        glm::vec3 neonPink(0.85f, 0.15f, 0.55f);   // pink neon accent
        // 10 building silhouettes of varied width/height at different horizon angles
        float bA[10]  = {-1.35f,-1.10f,-0.82f,-0.55f,-0.28f, 0.05f, 0.32f, 0.60f, 0.88f, 1.18f};
        float bW[10]  = {0.12f, 0.08f, 0.15f, 0.10f, 0.18f, 0.09f, 0.14f, 0.11f, 0.16f, 0.10f};
        float bH[10]  = {70.0f,45.0f,90.0f,55.0f,110.0f,40.0f,80.0f,60.0f,95.0f,50.0f};
        for (int b = 0; b < 10; ++b) {
            float aL = bA[b]-bW[b], aR = bA[b]+bW[b];
            glm::vec3 bBL(std::sin(aL)*sRadius, baseY, -std::cos(aL)*sRadius);
            glm::vec3 bBR(std::sin(aR)*sRadius, baseY, -std::cos(aR)*sRadius);
            glm::vec3 bTL(std::sin(aL)*(sRadius*0.97f), baseY+bH[b], -std::cos(aL)*(sRadius*0.97f));
            glm::vec3 bTR(std::sin(aR)*(sRadius*0.97f), baseY+bH[b], -std::cos(aR)*(sRadius*0.97f));
            AddQuad(verts, inds, bBL, bBR, bTR, bTL, skylineCol);
            // Neon window strip near top of tall buildings
            if (bH[b] > 60.0f) {
                glm::vec3 nL(std::sin(aL-0.01f)*(sRadius*0.97f), baseY+bH[b]-8.0f, -std::cos(aL-0.01f)*(sRadius*0.97f));
                glm::vec3 nR(std::sin(aR+0.01f)*(sRadius*0.97f), baseY+bH[b]-8.0f, -std::cos(aR+0.01f)*(sRadius*0.97f));
                glm::vec3 nTL(std::sin(aL-0.01f)*(sRadius*0.97f), baseY+bH[b]-2.0f, -std::cos(aL-0.01f)*(sRadius*0.97f));
                glm::vec3 nTR(std::sin(aR+0.01f)*(sRadius*0.97f), baseY+bH[b]-2.0f, -std::cos(aR+0.01f)*(sRadius*0.97f));
                glm::vec3 nc = (b % 2 == 0) ? neonCol : neonPink;
                AddQuad(verts, inds, nL, nR, nTR, nTL, nc);
            }
        }
        // Neon ground glow strip
        for (int l = 0; l < hSegs; ++l) {
            float a0 = startAngle + l*dAngle, a1 = a0+dAngle;
            glm::vec3 lb(std::sin(a0)*sRadius, baseY-1.0f, -std::cos(a0)*sRadius);
            glm::vec3 lb2(std::sin(a1)*sRadius, baseY-1.0f, -std::cos(a1)*sRadius);
            glm::vec3 lt(std::sin(a0)*(sRadius*0.99f), baseY+5.0f, -std::cos(a0)*(sRadius*0.99f));
            glm::vec3 lt2(std::sin(a1)*(sRadius*0.99f), baseY+5.0f, -std::cos(a1)*(sRadius*0.99f));
            AddQuad(verts, inds, lb, lb2, lt2, lt, (l%2==0) ? neonCol*0.4f : neonPink*0.3f);
        }
    } else {
        // Canyon / default: stepped flat-topped mesa cliffs
        glm::vec3 mesaC(0.78f, 0.52f, 0.38f), grassT(0.42f, 0.72f, 0.35f);
        float mA[4] = {-1.2f, -0.4f, 0.5f, 1.1f}, mW[4] = {0.32f, 0.45f, 0.38f, 0.28f}, mH[4] = {38.0f, 52.0f, 44.0f, 32.0f};
        for (int m = 0; m < 4; ++m) {
            glm::vec3 mL(std::sin(mA[m]-mW[m])*sRadius, baseY, -std::cos(mA[m]-mW[m])*sRadius);
            glm::vec3 mR(std::sin(mA[m]+mW[m])*sRadius, baseY, -std::cos(mA[m]+mW[m])*sRadius);
            glm::vec3 mTL(std::sin(mA[m]-mW[m])*(sRadius*0.97f), baseY+mH[m], -std::cos(mA[m]-mW[m])*(sRadius*0.97f));
            glm::vec3 mTR(std::sin(mA[m]+mW[m])*(sRadius*0.97f), baseY+mH[m], -std::cos(mA[m]+mW[m])*(sRadius*0.97f));
            AddQuad(verts, inds, mL, mR, mTR, mTL, mesaC * (0.85f + m*0.05f));
            glm::vec3 cL(std::sin(mA[m]-mW[m]-0.04f)*(sRadius*0.97f), baseY+mH[m]+2.0f, -std::cos(mA[m]-mW[m]-0.04f)*(sRadius*0.97f));
            glm::vec3 cR(std::sin(mA[m]+mW[m]+0.04f)*(sRadius*0.97f), baseY+mH[m]+2.0f, -std::cos(mA[m]+mW[m]+0.04f)*(sRadius*0.97f));
            AddQuad(verts, inds, mTL, mTR, cR, cL, grassT);
            float sh2 = mH[m]*0.58f;
            glm::vec3 sL2(std::sin(mA[m]-mW[m]*1.18f)*sRadius, baseY+sh2, -std::cos(mA[m]-mW[m]*1.18f)*sRadius);
            glm::vec3 sR2(std::sin(mA[m]+mW[m]*1.18f)*sRadius, baseY+sh2, -std::cos(mA[m]+mW[m]*1.18f)*sRadius);
            AddTriangle(verts, inds, mL, sL2, sR2, mesaC * 0.72f);
            AddTriangle(verts, inds, mL, sR2, mR,  mesaC * 0.78f);
        }
    }

    return Mesh(verts, inds);
}

Mesh Mesh::CreateBuilding(float w, float h, float d,
                          const glm::vec3& wallCol, const glm::vec3& windowCol,
                          const glm::vec3& roofCol, int windowRows, int windowCols) {
    std::vector<Vertex> verts;
    std::vector<GLuint> inds;

    float hw = w * 0.5f, hd = d * 0.5f;

    // Main 6 box faces
    // Front (Z+)
    AddQuad(verts, inds, {-hw,0,hd}, {hw,0,hd}, {hw,h,hd}, {-hw,h,hd}, wallCol * 0.92f);
    // Back (Z-)
    AddQuad(verts, inds, {hw,0,-hd}, {-hw,0,-hd}, {-hw,h,-hd}, {hw,h,-hd}, wallCol * 0.70f);
    // Left (X-)
    AddQuad(verts, inds, {-hw,0,-hd}, {-hw,0,hd}, {-hw,h,hd}, {-hw,h,-hd}, wallCol * 0.80f);
    // Right (X+)
    AddQuad(verts, inds, {hw,0,hd}, {hw,0,-hd}, {hw,h,-hd}, {hw,h,hd}, wallCol * 0.76f);
    // Roof
    AddQuad(verts, inds, {-hw,h,-hd}, {hw,h,-hd}, {hw,h,hd}, {-hw,h,hd}, roofCol);
    // Bottom
    AddQuad(verts, inds, {-hw,0,hd}, {hw,0,hd}, {hw,0,-hd}, {-hw,0,-hd}, wallCol * 0.4f);

    // Neon window quads on front and back faces
    float ww = (w / windowCols) * 0.32f;
    float wh = (h / windowRows) * 0.28f;
    float eps = 0.06f;
    for (int r = 0; r < windowRows; ++r) {
        float wy = (h / windowRows) * (r + 0.5f);
        for (int c = 0; c < windowCols; ++c) {
            float wx = -hw + (w / windowCols) * (c + 0.5f);
            // Front windows
            AddQuad(verts, inds,
                {wx-ww,wy-wh,hd+eps}, {wx+ww,wy-wh,hd+eps},
                {wx+ww,wy+wh,hd+eps}, {wx-ww,wy+wh,hd+eps},
                windowCol);
            // Back windows
            AddQuad(verts, inds,
                {wx+ww,wy-wh,-hd-eps}, {wx-ww,wy-wh,-hd-eps},
                {wx-ww,wy+wh,-hd-eps}, {wx+ww,wy+wh,-hd-eps},
                windowCol * 0.7f);
        }
    }
    return Mesh(verts, inds);
}

Mesh Mesh::CreateRubblePile(float radius, const glm::vec3& concreteCol, const glm::vec3& dirtCol) {
    std::vector<Vertex> verts;
    std::vector<GLuint> inds;
    // 8 random-ish angular slabs arranged in a pile
    float slabs[8][4] = { // {x,z,angle,size}
        {0.0f,0.0f,15.0f,1.0f}, {radius*0.4f,radius*0.3f,45.0f,0.7f},
        {-radius*0.5f,radius*0.2f,80.0f,0.8f}, {radius*0.2f,-radius*0.4f,120.0f,0.6f},
        {-radius*0.3f,-radius*0.3f,200.0f,0.9f}, {radius*0.6f,0.0f,30.0f,0.5f},
        {0.0f,radius*0.5f,165.0f,0.75f}, {-radius*0.6f,radius*0.1f,70.0f,0.65f}
    };
    for (auto& s : slabs) {
        float cx = s[0], cz = s[1], a = glm::radians(s[2]), sz = s[3] * radius;
        float ca = std::cos(a), sa = std::sin(a);
        float hw2 = sz * 0.6f, hd2 = sz * 0.3f;
        float baseY = -0.2f, topY = sz * 0.35f;
        glm::vec3 col = (int(s[2]) % 2 == 0) ? concreteCol : dirtCol;
        // 4 verts of rotated slab
        glm::vec3 bl(cx + (-hw2*ca - (-hd2)*sa), baseY, cz + (-hw2*sa + (-hd2)*ca));
        glm::vec3 br(cx + ( hw2*ca - (-hd2)*sa), baseY, cz + ( hw2*sa + (-hd2)*ca));
        glm::vec3 tr(cx + ( hw2*ca - hd2*sa), topY, cz + ( hw2*sa + hd2*ca));
        glm::vec3 tl(cx + (-hw2*ca - hd2*sa), topY, cz + (-hw2*sa + hd2*ca));
        AddQuad(verts, inds, bl, br, tr, tl, col);
        AddQuad(verts, inds, br, bl, tl, tr, col * 0.7f);
    }
    return Mesh(verts, inds);
}

Mesh Mesh::CreateCityRoad(float length, float width) {
    std::vector<Vertex> verts;
    std::vector<GLuint> inds;
    float hw = width * 0.5f, hl = length * 0.5f;
    float roadY = -7.5f;
    glm::vec3 asphaltCol(0.18f, 0.18f, 0.20f);
    glm::vec3 laneCol(0.85f, 0.78f, 0.25f); // yellow lane markings
    glm::vec3 sidewalkCol(0.32f, 0.30f, 0.28f);

    // Main asphalt
    AddQuad(verts, inds, {-hw,roadY,-hl},{hw,roadY,-hl},{hw,roadY,hl},{-hw,roadY,hl}, asphaltCol);

    // Lane markings: dashed center line every 40 units
    float dashLen = 12.0f, dashGap = 16.0f, period = dashLen + dashGap;
    float markW = 0.6f, laneY = roadY + 0.05f;
    for (float z = -hl; z < hl; z += period) {
        float z1 = std::min(z + dashLen, hl);
        AddQuad(verts, inds, {-markW,laneY,z},{markW,laneY,z},{markW,laneY,z1},{-markW,laneY,z1}, laneCol);
    }

    // Sidewalks (raised strips on each edge)
    float swW = 12.0f, swH = 1.2f;
    glm::vec3 swTop = sidewalkCol * 1.1f;
    // Left sidewalk
    AddQuad(verts, inds, {-hw-swW,roadY,-hl},{-hw,roadY,-hl},{-hw,roadY,hl},{-hw-swW,roadY,hl}, sidewalkCol);
    AddQuad(verts, inds, {-hw-swW,roadY+swH,-hl},{-hw,roadY+swH,-hl},{-hw,roadY+swH,hl},{-hw-swW,roadY+swH,hl}, swTop);
    AddQuad(verts, inds, {-hw,roadY,-hl},{-hw,roadY+swH,-hl},{-hw,roadY+swH,hl},{-hw,roadY,hl}, sidewalkCol * 0.8f);
    // Right sidewalk
    AddQuad(verts, inds, {hw,roadY,-hl},{hw+swW,roadY,-hl},{hw+swW,roadY,hl},{hw,roadY,hl}, sidewalkCol);
    AddQuad(verts, inds, {hw,roadY+swH,-hl},{hw+swW,roadY+swH,-hl},{hw+swW,roadY+swH,hl},{hw,roadY+swH,hl}, swTop);
    AddQuad(verts, inds, {hw,roadY+swH,-hl},{hw,roadY,-hl},{hw,roadY,hl},{hw,roadY+swH,hl}, sidewalkCol * 0.8f);

    return Mesh(verts, inds);
}

Mesh Mesh::CreateBipedalWalkerMesh(float scale, const glm::vec3& bodyCol,
                                    const glm::vec3& legCol, const glm::vec3& coreCol) {
    std::vector<Vertex> verts;
    std::vector<GLuint> inds;

    float s = scale;
    glm::vec3 darkBody = bodyCol * 0.85f;
    glm::vec3 lightBody = bodyCol * 1.05f;
    glm::vec3 accentCol = bodyCol * 0.6f;

    // Helper: add box at position with half-extents
    auto addBox = [&](glm::vec3 pos, float hw, float hh, float hd, glm::vec3 col) {
        glm::vec3 p = pos;
        AddQuad(verts, inds, p+glm::vec3(-hw,-hh,hd), p+glm::vec3(hw,-hh,hd), p+glm::vec3(hw,hh,hd), p+glm::vec3(-hw,hh,hd), col);
        AddQuad(verts, inds, p+glm::vec3(hw,-hh,-hd), p+glm::vec3(-hw,-hh,-hd), p+glm::vec3(-hw,hh,-hd), p+glm::vec3(hw,hh,-hd), col*0.7f);
        AddQuad(verts, inds, p+glm::vec3(-hw,-hh,-hd), p+glm::vec3(-hw,-hh,hd), p+glm::vec3(-hw,hh,hd), p+glm::vec3(-hw,hh,-hd), col*0.8f);
        AddQuad(verts, inds, p+glm::vec3(hw,-hh,hd), p+glm::vec3(hw,-hh,-hd), p+glm::vec3(hw,hh,-hd), p+glm::vec3(hw,hh,hd), col*0.75f);
        AddQuad(verts, inds, p+glm::vec3(-hw,hh,-hd), p+glm::vec3(hw,hh,-hd), p+glm::vec3(hw,hh,hd), p+glm::vec3(-hw,hh,hd), col*1.1f);
        AddQuad(verts, inds, p+glm::vec3(-hw,-hh,hd), p+glm::vec3(hw,-hh,hd), p+glm::vec3(hw,-hh,-hd), p+glm::vec3(-hw,-hh,-hd), col*0.5f);
    };

    float bodyTopY = s * 4.5f; // top of body from ground

    // Body
    addBox({0.0f, bodyTopY - s*1.5f, 0.0f}, s*2.2f, s*1.5f, s*1.1f, darkBody);

    // Left shoulder
    addBox({-(s*2.2f + s*0.8f), bodyTopY - s*0.8f, 0.0f}, s*0.8f, s*0.7f, s*0.7f, bodyCol);
    // Left cannon: 6-sided cylinder pointing forward
    for (int seg = 0; seg < 6; ++seg) {
        float a0 = glm::radians(seg * 60.0f), a1 = glm::radians((seg+1) * 60.0f);
        float cr = s * 0.22f;
        glm::vec3 c(-s*3.2f, bodyTopY - s*0.8f, 0.0f);
        glm::vec3 b0 = c + glm::vec3(std::cos(a0)*cr, std::sin(a0)*cr, 0.0f);
        glm::vec3 b1 = c + glm::vec3(std::cos(a1)*cr, std::sin(a1)*cr, 0.0f);
        glm::vec3 t0 = c + glm::vec3(std::cos(a0)*cr, std::sin(a0)*cr, s*2.2f);
        glm::vec3 t1 = c + glm::vec3(std::cos(a1)*cr, std::sin(a1)*cr, s*2.2f);
        AddQuad(verts, inds, b0, b1, t1, t0, accentCol);
    }
    // Right shoulder
    addBox({s*2.2f + s*0.8f, bodyTopY - s*0.8f, 0.0f}, s*0.8f, s*0.7f, s*0.7f, bodyCol);
    for (int seg = 0; seg < 6; ++seg) {
        float a0 = glm::radians(seg * 60.0f), a1 = glm::radians((seg+1) * 60.0f);
        float cr = s * 0.22f;
        glm::vec3 c(s*3.2f, bodyTopY - s*0.8f, 0.0f);
        glm::vec3 b0 = c + glm::vec3(std::cos(a0)*cr, std::sin(a0)*cr, 0.0f);
        glm::vec3 b1 = c + glm::vec3(std::cos(a1)*cr, std::sin(a1)*cr, 0.0f);
        glm::vec3 t0 = c + glm::vec3(std::cos(a0)*cr, std::sin(a0)*cr, s*2.2f);
        glm::vec3 t1 = c + glm::vec3(std::cos(a1)*cr, std::sin(a1)*cr, s*2.2f);
        AddQuad(verts, inds, b0, b1, t1, t0, accentCol);
    }

    // Neck
    addBox({0.0f, bodyTopY + s*0.4f, 0.0f}, s*0.5f, s*0.4f, s*0.5f, darkBody * 0.9f);
    // Head
    addBox({0.0f, bodyTopY + s*1.5f, 0.0f}, s*1.2f, s*0.8f, s*0.9f, bodyCol);
    // Glowing eye/core on head front (the weak point)
    AddQuad(verts, inds,
        {-s*0.35f, bodyTopY + s*0.85f, s*0.91f},
        { s*0.35f, bodyTopY + s*0.85f, s*0.91f},
        { s*0.35f, bodyTopY + s*1.32f, s*0.91f},
        {-s*0.35f, bodyTopY + s*1.32f, s*0.91f},
        coreCol * 2.5f);

    // Rear Exhaust Coolant Port (Tactical Granga weak point exposed during stagger)
    float rearZ = -s * 1.1f - 0.05f;
    float ventY = bodyTopY - s * 1.5f;
    glm::vec3 ventCol(1.0f, 0.72f, 0.15f); // radiant amber/gold exhaust glow
    glm::vec3 grateCol(0.12f, 0.12f, 0.15f);
    // Outer vent housing
    addBox({0.0f, ventY, rearZ + 0.05f}, s * 1.1f, s * 0.9f, 0.2f, grateCol);
    // 3 glowing horizontal exhaust slats
    for (int slat = -1; slat <= 1; ++slat) {
        float sy = ventY + slat * s * 0.45f;
        AddQuad(verts, inds,
            { -s * 0.85f, sy - s * 0.14f, rearZ - 0.08f },
            {  s * 0.85f, sy - s * 0.14f, rearZ - 0.08f },
            {  s * 0.85f, sy + s * 0.14f, rearZ - 0.08f },
            { -s * 0.85f, sy + s * 0.14f, rearZ - 0.08f },
            ventCol * 2.8f);
    }

    // Left upper leg
    addBox({-s*1.2f, s*1.8f, 0.0f}, s*0.55f, s*1.8f, s*0.55f, legCol);
    // Left knee joint actuator (destructible weak joint)
    addBox({-s * 1.2f, s * 1.8f, s * 0.62f}, s * 0.65f, s * 0.45f, s * 0.32f, glm::vec3(0.98f, 0.48f, 0.15f));
    // Left lower leg (angled slightly forward)
    addBox({-s*1.1f, s*0.55f - s*0.1f, s*0.15f}, s*0.45f, s*0.8f, s*0.45f, legCol * 0.9f);
    // Left foot
    addBox({-s*1.1f, -s*7.5f + s*0.15f, s*0.2f}, s*0.8f, s*0.15f, s*1.0f, legCol * 0.8f);

    // Right upper leg
    addBox({s*1.2f, s*1.8f, 0.0f}, s*0.55f, s*1.8f, s*0.55f, legCol);
    // Right knee joint actuator (destructible weak joint)
    addBox({s * 1.2f, s * 1.8f, s * 0.62f}, s * 0.65f, s * 0.45f, s * 0.32f, glm::vec3(0.98f, 0.48f, 0.15f));
    // Right lower leg
    addBox({s*1.1f, s*0.55f - s*0.1f, s*0.15f}, s*0.45f, s*0.8f, s*0.45f, legCol * 0.9f);
    // Right foot
    addBox({s*1.1f, -s*7.5f + s*0.15f, s*0.2f}, s*0.8f, s*0.15f, s*1.0f, legCol * 0.8f);

    return Mesh(verts, inds);
}

Mesh Mesh::CreateCityGantry(float spanWidth, float height) {
    std::vector<Vertex> verts;
    std::vector<GLuint> inds;
    float baseY = -7.5f;
    float hw = spanWidth * 0.5f;
    float topY = baseY + height;
    glm::vec3 steelCol(0.28f, 0.30f, 0.35f);
    glm::vec3 darkSteel = steelCol * 0.75f;
    glm::vec3 signGreen(0.12f, 0.55f, 0.32f);
    glm::vec3 amberText(0.95f, 0.75f, 0.20f);

    auto addPost = [&](float x) {
        float pw = 1.2f;
        AddQuad(verts, inds, {x - pw, baseY, -pw}, {x + pw, baseY, -pw}, {x + pw, topY, -pw}, {x - pw, topY, -pw}, steelCol);
        AddQuad(verts, inds, {x + pw, baseY, pw}, {x - pw, baseY, pw}, {x - pw, topY, pw}, {x + pw, topY, pw}, darkSteel);
        AddQuad(verts, inds, {x - pw, baseY, pw}, {x - pw, baseY, -pw}, {x - pw, topY, -pw}, {x - pw, topY, pw}, steelCol * 0.85f);
        AddQuad(verts, inds, {x + pw, baseY, -pw}, {x + pw, baseY, pw}, {x + pw, topY, pw}, {x + pw, topY, -pw}, steelCol * 0.85f);
    };

    addPost(-hw);
    addPost(hw);

    float trussH = 3.5f;
    float trussD = 2.0f;
    AddQuad(verts, inds, {-hw, topY - trussH, -trussD}, {hw, topY - trussH, -trussD}, {hw, topY, -trussD}, {-hw, topY, -trussD}, steelCol * 0.9f);
    AddQuad(verts, inds, {hw, topY - trussH, trussD}, {-hw, topY - trussH, trussD}, {-hw, topY, trussD}, {hw, topY, trussD}, steelCol * 0.75f);
    AddQuad(verts, inds, {-hw, topY, -trussD}, {hw, topY, -trussD}, {hw, topY, trussD}, {-hw, topY, trussD}, steelCol * 1.1f);
    AddQuad(verts, inds, {-hw, topY - trussH, trussD}, {hw, topY - trussH, trussD}, {hw, topY - trussH, -trussD}, {-hw, topY - trussH, -trussD}, darkSteel);

    float signW = 16.0f, signH = 2.4f;
    float signY = topY - 0.5f - signH;
    AddQuad(verts, inds, {-signW, signY, -trussD - 0.2f}, {signW, signY, -trussD - 0.2f}, {signW, signY + signH, -trussD - 0.2f}, {-signW, signY + signH, -trussD - 0.2f}, signGreen);
    AddQuad(verts, inds, {-signW + 1.0f, signY + 0.4f, -trussD - 0.25f}, {-signW + 4.0f, signY + 0.4f, -trussD - 0.25f}, {-signW + 4.0f, signY + signH - 0.4f, -trussD - 0.25f}, {-signW + 1.0f, signY + signH - 0.4f, -trussD - 0.25f}, amberText * 1.5f);
    AddQuad(verts, inds, {signW - 4.0f, signY + 0.4f, -trussD - 0.25f}, {signW - 1.0f, signY + 0.4f, -trussD - 0.25f}, {signW - 1.0f, signY + signH - 0.4f, -trussD - 0.25f}, {signW - 4.0f, signY + signH - 0.4f, -trussD - 0.25f}, amberText * 1.5f);

    return Mesh(verts, inds);
}

Mesh Mesh::CreateArenaPlaza(float radius, const glm::vec3& floorColA, const glm::vec3& floorColB, const glm::vec3& ringCol) {
    std::vector<Vertex> verts;
    std::vector<GLuint> inds;
    float baseY = -7.5f;
    const int segs = 36;
    float dAngle = glm::two_pi<float>() / segs;

    float r1 = radius * 0.25f;
    float r2 = radius * 0.65f;
    float r3 = radius * 0.92f;
    float r4 = radius;

    for (int i = 0; i < segs; ++i) {
        float a0 = i * dAngle;
        float a1 = (i + 1) * dAngle;
        float c0 = std::cos(a0), s0 = std::sin(a0);
        float c1 = std::cos(a1), s1 = std::sin(a1);

        glm::vec3 colA = (i % 2 == 0) ? floorColA : floorColB;
        glm::vec3 colB = (i % 2 == 0) ? floorColB : floorColA;

        AddTriangle(verts, inds, {0.0f, baseY, 0.0f}, {c0 * r1, baseY, s0 * r1}, {c1 * r1, baseY, s1 * r1}, colA * 1.15f);
        AddQuad(verts, inds, {c0 * r1, baseY, s0 * r1}, {c1 * r1, baseY, s1 * r1}, {c1 * r2, baseY, s1 * r2}, {c0 * r2, baseY, s0 * r2}, colB);
        AddQuad(verts, inds, {c0 * r2, baseY, s0 * r2}, {c1 * r2, baseY, s1 * r2}, {c1 * r3, baseY, s1 * r3}, {c0 * r3, baseY, s0 * r3}, colA);

        glm::vec3 neonCol = (i % 3 == 0) ? ringCol * 2.2f : ringCol * 1.4f;
        AddQuad(verts, inds, {c0 * r3, baseY + 0.1f, s0 * r3}, {c1 * r3, baseY + 0.1f, s1 * r3}, {c1 * r4, baseY + 0.1f, s1 * r4}, {c0 * r4, baseY + 0.1f, s0 * r4}, neonCol);
    }

    return Mesh(verts, inds);
}

Mesh Mesh::CreateArenaPillar(float height, float radius, const glm::vec3& stoneCol, const glm::vec3& neonCol) {
    std::vector<Vertex> verts;
    std::vector<GLuint> inds;
    float baseY = -7.5f;
    const int segs = 8;
    for (int i = 0; i < segs; ++i) {
        float a0 = (float)i / segs * glm::two_pi<float>();
        float a1 = (float)(i + 1) / segs * glm::two_pi<float>();
        glm::vec3 b0(std::cos(a0) * radius, baseY, std::sin(a0) * radius);
        glm::vec3 b1(std::cos(a1) * radius, baseY, std::sin(a1) * radius);
        glm::vec3 t0(std::cos(a0) * (radius * 0.85f), baseY + height, std::sin(a0) * (radius * 0.85f));
        glm::vec3 t1(std::cos(a1) * (radius * 0.85f), baseY + height, std::sin(a1) * (radius * 0.85f));
        glm::vec3 c = (i % 2 == 0) ? stoneCol : stoneCol * 0.75f;
        AddQuad(verts, inds, b0, b1, t1, t0, c);

        if (i == 0 || i == 4) {
            glm::vec3 n0 = glm::mix(b0, b1, 0.35f);
            glm::vec3 n1 = glm::mix(b0, b1, 0.65f);
            glm::vec3 nt0 = glm::mix(t0, t1, 0.35f);
            glm::vec3 nt1 = glm::mix(t0, t1, 0.65f);
            AddQuad(verts, inds, n0 * 1.02f, n1 * 1.02f, nt1 * 1.02f, nt0 * 1.02f, neonCol * 2.0f);
        }
    }
    for (int i = 0; i < segs; ++i) {
        float a0 = (float)i / segs * glm::two_pi<float>();
        float a1 = (float)(i + 1) / segs * glm::two_pi<float>();
        glm::vec3 t0(std::cos(a0) * (radius * 0.85f), baseY + height, std::sin(a0) * (radius * 0.85f));
        glm::vec3 t1(std::cos(a1) * (radius * 0.85f), baseY + height, std::sin(a1) * (radius * 0.85f));
        AddTriangle(verts, inds, {0.0f, baseY + height + 2.0f, 0.0f}, t0, t1, stoneCol * 1.2f);
    }
    return Mesh(verts, inds);
}

Mesh Mesh::CreateCollapsingSpire(float height, float baseRadius, const glm::vec3& trussCol, const glm::vec3& beaconCol) {
    std::vector<Vertex> verts;
    std::vector<GLuint> inds;
    float baseY = 0.0f; // Pivot at ground origin
    float topY = height;
    float br = baseRadius;
    float tr = baseRadius * 0.22f;

    // 4 Corner Pillars (Square cross-section pyramid)
    glm::vec3 bCorners[4] = {
        {-br, baseY, -br},
        { br, baseY, -br},
        { br, baseY,  br},
        {-br, baseY,  br}
    };
    glm::vec3 tCorners[4] = {
        {-tr, topY, -tr},
        { tr, topY, -tr},
        { tr, topY,  tr},
        {-tr, topY,  tr}
    };

    // 4 Slanted Truss Faces with X-bracing tiers
    const int tiers = 8;
    for (int t = 0; t < tiers; ++t) {
        float f0 = (float)t / tiers;
        float f1 = (float)(t + 1) / tiers;
        glm::vec3 tierCol = (t % 2 == 0) ? trussCol : trussCol * 0.75f;
        // Hazard warning bands on tier 2 and tier 5
        if (t == 2 || t == 5) {
            tierCol = glm::vec3(0.95f, 0.45f, 0.12f);
        }

        for (int side = 0; side < 4; ++side) {
            int nextSide = (side + 1) % 4;
            glm::vec3 p0 = glm::mix(bCorners[side], tCorners[side], f0);
            glm::vec3 p1 = glm::mix(bCorners[nextSide], tCorners[nextSide], f0);
            glm::vec3 p2 = glm::mix(bCorners[nextSide], tCorners[nextSide], f1);
            glm::vec3 p3 = glm::mix(bCorners[side], tCorners[side], f1);

            // Cross truss X-struts
            glm::vec3 mid = (p0 + p1 + p2 + p3) * 0.25f;
            AddTriangle(verts, inds, p0, p1, mid, tierCol);
            AddTriangle(verts, inds, p1, p2, mid, tierCol * 0.85f);
            AddTriangle(verts, inds, p2, p3, mid, tierCol);
            AddTriangle(verts, inds, p3, p0, mid, tierCol * 0.85f);

            // Outer perimeter strut band
            glm::vec3 thick(0.0f, 0.35f, 0.0f);
            AddQuad(verts, inds, p0 - thick, p1 - thick, p1 + thick, p0 + thick, trussCol * 1.1f);
        }
    }

    // Glowing Warning Beacon at top
    float beaconH = 3.5f;
    glm::vec3 apex(0.0f, topY + beaconH, 0.0f);
    for (int side = 0; side < 4; ++side) {
        int nextSide = (side + 1) % 4;
        AddTriangle(verts, inds, tCorners[side], tCorners[nextSide], apex, beaconCol * 3.0f);
    }

    return Mesh(verts, inds);
}
