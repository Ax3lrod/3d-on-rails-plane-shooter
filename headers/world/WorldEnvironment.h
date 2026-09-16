#ifndef WORLD_ENVIRONMENT_H
#define WORLD_ENVIRONMENT_H

#include <glm/glm.hpp>
#include <vector>
#include "Mesh.h"
#include "shaderClass.h"

struct RingGate {
    glm::vec3 position;
    float radius;
    float rotation;
    bool isGold;
    bool collected;
};

struct AsteroidObstacle {
    glm::vec3 position;
    glm::vec3 rotation;
    glm::vec3 rotSpeed;
    float radius;
    bool destroyed;
};

struct CanyonSlice {
    glm::vec3 position;
};

struct RockArchway {
    glm::vec3 position;
    float spanWidth;
    float height;
};

struct HazardPillar {
    glm::vec3 position;
    float radius;
    float height;
    float health;
    bool destroyed;
};

class WorldEnvironment {
public:
    std::vector<RingGate> rings;
    std::vector<AsteroidObstacle> asteroids;
    std::vector<CanyonSlice> canyonSlices;
    std::vector<RockArchway> rockArches;
    std::vector<HazardPillar> pillars;

    Mesh goldRingMesh;
    Mesh silverRingMesh;
    Mesh asteroidMesh;
    Mesh canyonMesh;
    Mesh rockArchMesh;
    Mesh pillarMesh;

    float nextSpawnZ;
    float despawnDistBehind;

    WorldEnvironment();

    void Update(float playerZ, float dt);
    void Draw(const Shader& shader) const;
    void Clear();

private:
    void GenerateChunk(float startZ, float endZ);
};

#endif
