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

enum class SectorStage {
    Sector1_Canyon,
    Sector2_DeepSpace
};

struct SecretRelay {
    glm::vec3 position;
    float radius;
    float health;
    float maxHealth;
    bool destroyed;
    float pulseTimer;
};

struct SpaceDebris {
    glm::vec3 position;
    glm::vec3 rotation;
    glm::vec3 rotSpeed;
    float radius;
};

class WorldEnvironment {
public:
    SectorStage currentSector;

    std::vector<RingGate> rings;
    std::vector<AsteroidObstacle> asteroids;
    std::vector<CanyonSlice> canyonSlices;
    std::vector<RockArchway> rockArches;
    std::vector<HazardPillar> pillars;
    std::vector<SecretRelay> secretRelays;
    std::vector<SpaceDebris> spaceDebris;

    Mesh goldRingMesh;
    Mesh silverRingMesh;
    Mesh asteroidMesh;
    Mesh canyonMesh;
    Mesh rockArchMesh;
    Mesh pillarMesh;
    Mesh relayMesh;
    Mesh debrisMesh;
    Mesh horizonMesh;

    float nextSpawnZ;
    float despawnDistBehind;
    float lastPlayerZ;

    WorldEnvironment();

    void SetSector(SectorStage sector);
    int GetDestroyedRelayCount() const;
    int GetTotalRelayCount() const { return 3; }

    void Update(float playerZ, float dt);
    void Draw(const Shader& shader) const;
    void Clear();

private:
    void GenerateChunk(float startZ, float endZ);
};

#endif
