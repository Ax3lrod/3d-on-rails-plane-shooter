#ifndef WORLD_ENVIRONMENT_H
#define WORLD_ENVIRONMENT_H

#include <glm/glm.hpp>
#include <vector>
#include <string>
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

struct FloatingDomeObstacle {
    glm::vec3 position;
    float radius;
};

struct WindTurbineObstacle {
    glm::vec3 position;
    float rotation;
    float rotSpeed;
};

struct TreeObstacle {
    glm::vec3 position;
    float scale;
    float rotation;
};

// Phase 35: Biome-specific prop structs
struct IceCrystalSpire {
    glm::vec3 position;
    float scale;
    float rotation;
};
struct CactusObstacle {
    glm::vec3 position;
    float scale;
    float rotation;
};
struct DesertPyramidObstacle {
    glm::vec3 position;
    float scale;
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
    std::vector<FloatingDomeObstacle> floatingDomes;
    std::vector<WindTurbineObstacle> windTurbines;
    std::vector<TreeObstacle> trees;
    // Phase 35: biome-specific prop instance lists
    std::vector<IceCrystalSpire> iceCrystals;
    std::vector<CactusObstacle> cacti;
    std::vector<DesertPyramidObstacle> desertPyramids;

    Mesh goldRingMesh;
    Mesh silverRingMesh;
    Mesh asteroidMesh;
    Mesh canyonMesh;
    Mesh rockArchMesh;
    Mesh pillarMesh;
    Mesh relayMesh;
    Mesh debrisMesh;
    Mesh horizonMesh;
    Mesh domeMesh;
    Mesh turbineMesh;
    Mesh turbineTowerMesh;
    Mesh turbineBladesMesh;
    Mesh treeMesh;
    // Phase 35: biome prop meshes
    Mesh iceCrystalMesh;
    Mesh cactusMesh;
    Mesh desertPyramidMesh;

    float nextSpawnZ;
    float despawnDistBehind;
    float lastPlayerZ;

    WorldEnvironment();

    void SetSector(SectorStage sector);
    void SetTerrainTheme(const std::string& theme);
    int GetDestroyedRelayCount() const;
    int GetTotalRelayCount() const { return 3; }

    std::string terrainTheme;

    void Update(float playerZ, float dt);
    void Draw(const Shader& shader) const;
    void Clear();

private:
    void GenerateChunk(float startZ, float endZ);
};

#endif
