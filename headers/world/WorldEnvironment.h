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

enum class TerrainSliceType {
    Default,
    Coastline,
    HighwayTransition,
    ElevatedHighway,
    CityRoad,
    CityIntersection,
    SunkenCanal
};

struct OceanIslandObstacle {
    glm::vec3 position;
    float scale;
    float rotation;
};

struct NavalFortressObstacle {
    glm::vec3 position;
    float scale;
    float rotation;
};

struct CargoShipObstacle {
    glm::vec3 position;
    float scale;
    float rotation;
    float speed = 3.0f;
};

struct SkybridgeObstacle {
    glm::vec3 position;
    float scale;
};

struct CanyonSlice {
    glm::vec3 position;
    TerrainSliceType sliceType = TerrainSliceType::Default;
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

struct BuildingObstacle {
    glm::vec3 position;
    float width, height, depth;
    float rotation; // degrees around Y
    int meshVariant; // 0..4 (fixed identity, never changes!)
    float scale;
};

struct CityGantryObstacle {
    glm::vec3 position;
    float spanWidth;
};

struct CollapsingSpireHazard {
    glm::vec3 position;
    float height = 55.0f;
    float currentAngle = 0.0f;
    float targetAngle = 54.0f;
    float collapseSpeed = 24.0f;
    float triggerDist = 210.0f;
    bool isTriggered = false;
    bool destroyed = false;
    float health = 160.0f;
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
    std::vector<OceanIslandObstacle> oceanIslands;
    std::vector<NavalFortressObstacle> navalFortresses;
    std::vector<CargoShipObstacle> cargoShips;
    // Phase 35: biome-specific prop instance lists
    std::vector<IceCrystalSpire> iceCrystals;
    std::vector<CactusObstacle> cacti;
    std::vector<DesertPyramidObstacle> desertPyramids;
    std::vector<BuildingObstacle> buildings;
    std::vector<CityGantryObstacle> cityGantries;
    std::vector<CollapsingSpireHazard> collapsingSpires;
    std::vector<SkybridgeObstacle> skybridges;

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
    Mesh buildingMeshes[8];
    Mesh ruinedBuildingMesh;
    Mesh skybridgeMesh;
    Mesh cityIntersectionMesh;
    Mesh billboardBuildingMesh[3];
    Mesh roadMesh;
    Mesh rubbleMesh;
    // City gantries & arena plaza
    Mesh gantryMesh;
    Mesh spireMesh;
    Mesh arenaPlazaMesh;
    Mesh arenaPillarMesh;
    // Phase 40: Corneria-style 5-zone modular terrain meshes
    Mesh coastlineMesh;
    Mesh oceanIslandMesh;
    Mesh highwayTransitionMesh;
    Mesh elevatedHighwayMesh;
    Mesh navalFortressMesh;
    Mesh cargoShipMesh;
    Mesh canalMesh;
    // Odd-parity terrain mesh: same geometry but rowOffset=1 so adjacent slabs tile seamlessly
    Mesh canyonMeshOdd;

    float nextSpawnZ;
    float despawnDistBehind;
    float lastPlayerZ;
    bool isAllRangeActive;
    bool arenaSpawned;
    glm::vec3 arenaCenter;

    WorldEnvironment();

    void SetSector(SectorStage sector);
    void SetTerrainTheme(const std::string& theme);
    void SpawnCollapsingSpire(float z, float x = -32.0f);
    int GetDestroyedRelayCount() const;
    int GetTotalRelayCount() const { return 3; }

    std::string terrainTheme;

    void Update(float playerZ, float dt, bool allRange = false);
    void Draw(const Shader& shader) const;
    void Clear();

private:
    void GenerateChunk(float startZ, float endZ);
};

#endif
