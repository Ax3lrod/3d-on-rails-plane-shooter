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
    float t = size * 0.08f;
    float gap = size * 0.2f;

    // 4 reticle brackets (Top, Bottom, Left, Right)
    // Top bracket
    AddQuad(verts, inds, {-t, gap, 0.0f}, {t, gap, 0.0f}, {t, s, 0.0f}, {-t, s, 0.0f}, color);
    // Bottom bracket
    AddQuad(verts, inds, {-t, -s, 0.0f}, {t, -s, 0.0f}, {t, -gap, 0.0f}, {-t, -gap, 0.0f}, color);
    // Left bracket
    AddQuad(verts, inds, {-s, -t, 0.0f}, {-gap, -t, 0.0f}, {-gap, t, 0.0f}, {-s, t, 0.0f}, color);
    // Right bracket
    AddQuad(verts, inds, {gap, -t, 0.0f}, {s, -t, 0.0f}, {s, t, 0.0f}, {gap, t, 0.0f}, color);

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
                               const glm::vec3& floorCol, const glm::vec3& wallCol) {
    std::vector<Vertex> verts;
    std::vector<GLuint> inds;

    float hw = width * 0.5f;
    float hl = length * 0.5f;
    float floorY = -7.5f;

    // 1. Canyon Floor Grid (3 segments across width, 4 segments along length with low-poly variations)
    const int xSegs = 4;
    const int zSegs = 6;
    float dx = width / xSegs;
    float dz = length / zSegs;

    for (int iz = 0; iz < zSegs; ++iz) {
        float z0 = -hl + iz * dz;
        float z1 = z0 + dz;

        for (int ix = 0; ix < xSegs; ++ix) {
            float x0 = -hw + ix * dx;
            float x1 = x0 + dx;

            // Subtle elevation variation for natural canyon terrain
            float y00 = floorY + std::sin(ix * 1.5f + iz * 2.1f) * 0.35f;
            float y10 = floorY + std::sin((ix + 1) * 1.5f + iz * 2.1f) * 0.35f;
            float y11 = floorY + std::sin((ix + 1) * 1.5f + (iz + 1) * 2.1f) * 0.35f;
            float y01 = floorY + std::sin(ix * 1.5f + (iz + 1) * 2.1f) * 0.35f;

            // Alternate color tone slightly for faceted terrain look
            float shade = 0.92f + 0.16f * ((ix + iz) % 2);
            AddQuad(verts, inds,
                    {x0, y00, z0}, {x1, y10, z0}, {x1, y11, z1}, {x0, y01, z1},
                    floorCol * shade);
        }
    }

    // 2. Left Cliff Wall (Rising from -hw to -hw - 9, up to floorY + wallHeight)
    for (int iz = 0; iz < zSegs; ++iz) {
        float z0 = -hl + iz * dz;
        float z1 = z0 + dz;

        // Tier 1: Steep wall face
        glm::vec3 b0(-hw, floorY, z0);
        glm::vec3 b1(-hw, floorY, z1);
        glm::vec3 m0(-hw - 4.0f, floorY + wallHeight * 0.55f, z0);
        glm::vec3 m1(-hw - 4.0f, floorY + wallHeight * 0.55f, z1);
        AddQuad(verts, inds, b0, m0, m1, b1, wallCol * 0.9f);

        // Tier 2: Upper ridge face
        glm::vec3 t0(-hw - 8.0f, floorY + wallHeight, z0);
        glm::vec3 t1(-hw - 8.0f, floorY + wallHeight, z1);
        AddQuad(verts, inds, m0, t0, t1, m1, wallCol * 1.08f);
    }

    // 3. Right Cliff Wall (Rising from hw to hw + 9, up to floorY + wallHeight)
    for (int iz = 0; iz < zSegs; ++iz) {
        float z0 = -hl + iz * dz;
        float z1 = z0 + dz;

        glm::vec3 b0(hw, floorY, z0);
        glm::vec3 b1(hw, floorY, z1);
        glm::vec3 m0(hw + 4.0f, floorY + wallHeight * 0.55f, z0);
        glm::vec3 m1(hw + 4.0f, floorY + wallHeight * 0.55f, z1);
        AddQuad(verts, inds, b0, b1, m1, m0, wallCol * 0.85f);

        glm::vec3 t0(hw + 8.0f, floorY + wallHeight, z0);
        glm::vec3 t1(hw + 8.0f, floorY + wallHeight, z1);
        AddQuad(verts, inds, m0, m1, t1, t0, wallCol * 1.02f);
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
