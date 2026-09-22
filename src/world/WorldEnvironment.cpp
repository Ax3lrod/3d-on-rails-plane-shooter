#include "WorldEnvironment.h"
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>
#include <cstdlib>
#include <cmath>

WorldEnvironment::WorldEnvironment()
    : currentSector(SectorStage::Sector1_Canyon),
      goldRingMesh(Mesh::CreateRing(2.6f, 3.2f, 16, glm::vec3(1.0f, 0.85f, 0.2f))),
      silverRingMesh(Mesh::CreateRing(2.6f, 3.2f, 16, glm::vec3(0.3f, 0.85f, 1.0f))),
      asteroidMesh(Mesh::CreateAsteroid(2.4f, glm::vec3(0.55f, 0.52f, 0.48f))),
      canyonMesh(Mesh::CreateOpenFieldTerrain(60.0f, 340.0f,
                                            glm::vec3(0.18f, 0.65f, 0.28f),
                                            glm::vec3(0.28f, 0.84f, 0.38f), 0)),
      canyonMeshOdd(Mesh::CreateOpenFieldTerrain(60.0f, 340.0f,
                                            glm::vec3(0.18f, 0.65f, 0.28f),
                                            glm::vec3(0.28f, 0.84f, 0.38f), 1)),
      rockArchMesh(Mesh::CreateRockArch(54.0f, 22.0f, 8.5f, glm::vec3(0.92f, 0.68f, 0.72f))),
      pillarMesh(Mesh::CreatePillar(1.8f, 16.0f, glm::vec3(0.90f, 0.75f, 0.78f))),
      relayMesh(Mesh::CreateRadarRelay(2.6f, glm::vec3(0.35f, 0.40f, 0.45f), glm::vec3(0.85f, 0.88f, 0.92f), glm::vec3(1.0f, 0.2f, 0.15f))),
      debrisMesh(Mesh::CreateSpaceDebris(14.0f, 2.0f, glm::vec3(0.45f, 0.48f, 0.52f))),
      horizonMesh(Mesh::CreateBiomeHorizon("canyon", 460.0f, 220.0f,
                                            glm::vec3(0.16f, 0.46f, 0.92f),   // Sega blue skies
                                            glm::vec3(0.75f, 0.90f, 0.98f))), // Radiant horizon light
      domeMesh(Mesh::CreateFloatingDome(6.5f, glm::vec3(0.92f, 0.95f, 1.0f), glm::vec3(0.25f, 0.95f, 1.0f))),
      turbineMesh(Mesh::CreateWindTurbineTower(26.0f, glm::vec3(0.92f, 0.94f, 0.96f), glm::vec3(0.20f, 0.22f, 0.28f))),
      turbineTowerMesh(Mesh::CreateWindTurbineTower(26.0f, glm::vec3(0.92f, 0.94f, 0.96f), glm::vec3(0.20f, 0.22f, 0.28f))),
      turbineBladesMesh(Mesh::CreateWindTurbineBlades(10.5f, glm::vec3(0.98f, 0.98f, 1.0f), glm::vec3(0.92f, 0.18f, 0.22f))),
      treeMesh(Mesh::CreateLowPolyTree(4.2f, 0.75f, 11.0f, 4.4f, glm::vec3(0.45f, 0.28f, 0.16f), glm::vec3(0.16f, 0.62f, 0.24f))),
      iceCrystalMesh(Mesh::CreateIceCrystal(14.0f, 2.5f, glm::vec3(0.70f, 0.90f, 1.0f), glm::vec3(0.85f, 0.96f, 1.0f))),
      cactusMesh(Mesh::CreateCactus(9.0f, 0.8f, 4.0f, glm::vec3(0.28f, 0.62f, 0.22f))),
      desertPyramidMesh(Mesh::CreateDesertPyramid(22.0f, 18.0f, glm::vec3(0.85f, 0.72f, 0.46f), glm::vec3(0.58f, 0.46f, 0.26f))),
      roadMesh(Mesh::CreateCityRoad(60.0f, 68.0f)),
      rubbleMesh(Mesh::CreateRubblePile(4.5f, glm::vec3(0.35f,0.33f,0.30f), glm::vec3(0.22f,0.20f,0.18f))),
      gantryMesh(Mesh::CreateCityGantry(54.0f, 18.0f)),
      spireMesh(Mesh::CreateCollapsingSpire(55.0f, 4.2f, glm::vec3(0.35f, 0.38f, 0.42f), glm::vec3(1.0f, 0.2f, 0.1f))),
      arenaPlazaMesh(Mesh::CreateArenaPlaza(300.0f, glm::vec3(0.12f, 0.13f, 0.16f), glm::vec3(0.18f, 0.19f, 0.22f), glm::vec3(0.95f, 0.22f, 0.15f))),
      arenaPillarMesh(Mesh::CreateArenaPillar(45.0f, 6.0f, glm::vec3(0.22f, 0.24f, 0.28f), glm::vec3(0.20f, 0.70f, 0.95f))),
      coastlineMesh(Mesh::CreateCoastlineTerrain(60.0f, 440.0f)),
      oceanIslandMesh(Mesh::CreateOceanIsland(28.0f, 22.0f, 8.0f)),
      highwayTransitionMesh(Mesh::CreateCoastalHighwayTransition(600.0f, 160.0f)),
      elevatedHighwayMesh(Mesh::CreateElevatedHighway(2200.0f, 160.0f)),
      navalFortressMesh(Mesh::CreateNavalFortress(22.0f, 14.0f)),
      cargoShipMesh(Mesh::CreateCargoShip(72.0f, 18.0f)),
      canalMesh(Mesh::CreateCityCanal(60.0f, 140.0f)),
      ruinedBuildingMesh(Mesh::CreateRuinedBuilding(32.0f, 110.0f, 26.0f, glm::vec3(0.20f, 0.22f, 0.26f), glm::vec3(0.15f, 0.65f, 0.90f))),
      skybridgeMesh(Mesh::CreateSkybridge(76.0f, 14.0f, 6.0f)),
      cityIntersectionMesh(Mesh::CreateCityIntersection(60.0f, 68.0f)),
      nextSpawnZ(0.0f),
      despawnDistBehind(60.0f),
      lastPlayerZ(0.0f),
      isAllRangeActive(false),
      arenaSpawned(false),
      arenaCenter(0.0f, -7.5f, -13180.0f) {
    buildingMeshes[0] = Mesh::CreateBuilding(18.0f, 35.0f, 14.0f, glm::vec3(0.22f,0.24f,0.28f), glm::vec3(0.15f,0.60f,0.85f), glm::vec3(0.18f,0.20f,0.24f), 5, 3);
    buildingMeshes[1] = Mesh::CreateBuilding(24.0f, 55.0f, 18.0f, glm::vec3(0.20f,0.22f,0.26f), glm::vec3(0.90f,0.25f,0.55f), glm::vec3(0.15f,0.17f,0.20f), 7, 4);
    buildingMeshes[2] = Mesh::CreateBuilding(14.0f, 28.0f, 12.0f, glm::vec3(0.25f,0.26f,0.30f), glm::vec3(0.20f,0.75f,0.50f), glm::vec3(0.20f,0.22f,0.25f), 4, 2);
    buildingMeshes[3] = Mesh::CreateBuilding(30.0f, 75.0f, 22.0f, glm::vec3(0.18f,0.20f,0.24f), glm::vec3(0.85f,0.62f,0.15f), glm::vec3(0.14f,0.16f,0.20f), 9, 5);
    buildingMeshes[4] = Mesh::CreateBuilding(10.0f, 18.0f, 10.0f, glm::vec3(0.28f,0.28f,0.32f), glm::vec3(0.40f,0.80f,0.95f), glm::vec3(0.22f,0.22f,0.25f), 3, 2);
    // Sky-high mega skyscrapers (125m - 165m) for Zone C Concrete Jungle
    buildingMeshes[5] = Mesh::CreateBuilding(36.0f, 125.0f, 28.0f, glm::vec3(0.16f, 0.18f, 0.22f), glm::vec3(0.20f, 0.85f, 0.95f), glm::vec3(0.14f, 0.15f, 0.18f), 14, 5);
    buildingMeshes[6] = Mesh::CreateBuilding(42.0f, 150.0f, 32.0f, glm::vec3(0.18f, 0.19f, 0.24f), glm::vec3(0.95f, 0.35f, 0.60f), glm::vec3(0.15f, 0.16f, 0.20f), 16, 6);
    buildingMeshes[7] = Mesh::CreateBuilding(48.0f, 165.0f, 36.0f, glm::vec3(0.15f, 0.16f, 0.20f), glm::vec3(0.90f, 0.70f, 0.20f), glm::vec3(0.12f, 0.13f, 0.16f), 18, 6);

    // Neon Cyberpunk Billboard Skyscrapers (145m)
    billboardBuildingMesh[0] = Mesh::CreateCyberBillboardBuilding(36.0f, 145.0f, 28.0f, glm::vec3(0.17f, 0.18f, 0.22f), glm::vec3(0.20f, 0.70f, 0.90f), glm::vec3(0.95f, 0.20f, 0.65f), 0);
    billboardBuildingMesh[1] = Mesh::CreateCyberBillboardBuilding(38.0f, 145.0f, 30.0f, glm::vec3(0.18f, 0.19f, 0.23f), glm::vec3(0.85f, 0.55f, 0.20f), glm::vec3(0.15f, 0.90f, 0.95f), 1);
    billboardBuildingMesh[2] = Mesh::CreateCyberBillboardBuilding(40.0f, 145.0f, 32.0f, glm::vec3(0.16f, 0.17f, 0.21f), glm::vec3(0.25f, 0.80f, 0.50f), glm::vec3(0.98f, 0.80f, 0.20f), 2);
    Clear();
}

void WorldEnvironment::SetSector(SectorStage sector) {
    currentSector = sector;
    Clear();
}

void WorldEnvironment::SpawnCollapsingSpire(float z, float x) {
    CollapsingSpireHazard sp;
    sp.position = glm::vec3(x, -7.5f, z);
    collapsingSpires.push_back(sp);
}

void WorldEnvironment::SetTerrainTheme(const std::string& theme) {
    terrainTheme = theme;
    if (theme == "railway_canyon") {
        glm::vec3 cA(0.65f, 0.28f, 0.16f), cB(0.55f, 0.22f, 0.12f);
        canyonMesh    = Mesh::CreateOpenFieldTerrain(60.0f, 340.0f, cA, cB, 0);
        canyonMeshOdd = Mesh::CreateOpenFieldTerrain(60.0f, 340.0f, cA, cB, 1);
        horizonMesh = Mesh::CreateBiomeHorizon("canyon", 700.0f, 260.0f,
                                               glm::vec3(0.85f, 0.42f, 0.18f),
                                               glm::vec3(0.98f, 0.65f, 0.35f));
    } else if (theme == "iron_fortress") {
        glm::vec3 cA(0.18f, 0.20f, 0.24f), cB(0.25f, 0.27f, 0.32f);
        canyonMesh    = Mesh::CreateOpenFieldTerrain(60.0f, 340.0f, cA, cB, 0);
        canyonMeshOdd = Mesh::CreateOpenFieldTerrain(60.0f, 340.0f, cA, cB, 1);
        horizonMesh = Mesh::CreateBiomeHorizon("iron_fortress", 700.0f, 260.0f,
                                               glm::vec3(0.12f, 0.10f, 0.16f),
                                               glm::vec3(0.22f, 0.12f, 0.10f));
    } else if (theme == "dune_pass") {
        glm::vec3 cA(0.84f, 0.65f, 0.32f), cB(0.76f, 0.58f, 0.26f);
        canyonMesh    = Mesh::CreateOpenFieldTerrain(60.0f, 340.0f, cA, cB, 0);
        canyonMeshOdd = Mesh::CreateOpenFieldTerrain(60.0f, 340.0f, cA, cB, 1);
        horizonMesh = Mesh::CreateBiomeHorizon("dune_pass", 700.0f, 260.0f,
                                               glm::vec3(0.70f, 0.52f, 0.28f),
                                               glm::vec3(0.98f, 0.82f, 0.50f));
    } else if (theme == "glacial") {
        glm::vec3 cA(0.75f, 0.88f, 0.96f), cB(0.85f, 0.93f, 1.0f);
        canyonMesh    = Mesh::CreateOpenFieldTerrain(60.0f, 340.0f, cA, cB, 0);
        canyonMeshOdd = Mesh::CreateOpenFieldTerrain(60.0f, 340.0f, cA, cB, 1);
        horizonMesh = Mesh::CreateBiomeHorizon("glacial", 700.0f, 260.0f,
                                               glm::vec3(0.22f, 0.45f, 0.72f),
                                               glm::vec3(0.82f, 0.92f, 1.0f));
    } else if (theme == "city") {
        canyonMesh    = Mesh::CreateCityRoad(60.0f, 68.0f);
        canyonMeshOdd = Mesh::CreateCityRoad(60.0f, 68.0f);
        horizonMesh = Mesh::CreateBiomeHorizon("city", 700.0f, 260.0f,
                                               glm::vec3(0.06f, 0.06f, 0.10f),
                                               glm::vec3(0.12f, 0.10f, 0.18f));
    } else {
        glm::vec3 cA(0.18f, 0.58f, 0.28f), cB(0.24f, 0.68f, 0.35f);
        canyonMesh    = Mesh::CreateOpenFieldTerrain(60.0f, 340.0f, cA, cB, 0);
        canyonMeshOdd = Mesh::CreateOpenFieldTerrain(60.0f, 340.0f, cA, cB, 1);
        horizonMesh = Mesh::CreateBiomeHorizon("canyon", 700.0f, 260.0f,
                                               glm::vec3(0.16f, 0.46f, 0.92f),
                                               glm::vec3(0.75f, 0.90f, 0.98f));
    }
    Clear();
}

int WorldEnvironment::GetDestroyedRelayCount() const {
    int count = 0;
    for (const auto& r : secretRelays) {
        if (r.destroyed) count++;
    }
    return count;
}

void WorldEnvironment::GenerateChunk(float startZ, float endZ) {
    if (currentSector == SectorStage::Sector1_Canyon) {
        float sliceLen = 60.0f;

        if (terrainTheme == "city") {
            // 5-ZONE CORNERIA TOPOGRAPHY: Spawn modular terrain slices based on Z coordinates
            for (float z = startZ; z >= endZ; z -= sliceLen) {
                if (z > -12800.0f) {
                    TerrainSliceType st = TerrainSliceType::CityRoad;
                    if (z > -2200.0f) {
                        st = TerrainSliceType::Coastline;      // Zone A: Emerald Coastline Open Ocean
                    } else if (z > -2800.0f) {
                        continue; // Dedicated 600m Winding Coastal Highway Transition Set-Piece
                    } else if (z > -5000.0f) {
                        continue; // Dedicated 2200m Winding Elevated Highway & Suspension Bridge Set-Piece
                    } else if (z > -8500.0f) {
                        // Zone C: Downtown Concrete Jungle (4-way cross-avenue intersections every ~540m-600m)
                        if (std::abs(z - (-5400.0f)) < 1.0f ||
                            std::abs(z - (-5940.0f)) < 1.0f ||
                            std::abs(z - (-6480.0f)) < 1.0f ||
                            std::abs(z - (-7020.0f)) < 1.0f ||
                            std::abs(z - (-7560.0f)) < 1.0f ||
                            std::abs(z - (-8160.0f)) < 1.0f) {
                            st = TerrainSliceType::CityIntersection;
                        } else {
                            st = TerrainSliceType::CityRoad;
                        }
                    } else if (z > -11000.0f) {
                        st = TerrainSliceType::SunkenCanal;     // Zone D: Sunken Canal & Aqueduct
                    } else {
                        st = TerrainSliceType::CityRoad;        // Zone E approach
                    }
                    canyonSlices.push_back({glm::vec3(0.0f, 0.0f, z), st});
                }
            }

            // GRAND ARENA PLAZA: Spawn when approaching the boss area
            if (endZ <= -12700.0f && !arenaSpawned) {
                arenaSpawned = true;
                arenaCenter = glm::vec3(0.0f, -7.5f, -13180.0f);
            }

            // ZONE A: SCATTERED ARCHIPELAGO ISLANDS & REWARD RINGS (Z = 0 to -2150)
            for (float z = startZ - 40.0f; z >= endZ; z -= 220.0f) {
                if (z > -2150.0f && z < -40.0f) {
                    // Left flank island (tropical palm trees and beach)
                    float lx = -45.0f - ((rand() % 100) / 100.0f) * 45.0f;
                    float lSc = 0.85f + ((rand() % 100) / 100.0f) * 0.55f;
                    float lRot = (float)(rand() % 360);
                    oceanIslands.push_back({glm::vec3(lx, -7.5f, z), lSc, lRot});

                    // Right flank island
                    float rx = 45.0f + ((rand() % 100) / 100.0f) * 45.0f;
                    float rSc = 0.85f + ((rand() % 100) / 100.0f) * 0.55f;
                    float rRot = (float)(rand() % 360);
                    oceanIslands.push_back({glm::vec3(rx, -7.5f, z - 70.0f), rSc, rRot});

                    // Corridor island: Occasional scenic island right in or near flight path
                    if (static_cast<int>(std::abs(z)) % 440 < 220) {
                        float cx = ((rand() % 2 == 0) ? -1.0f : 1.0f) * (8.0f + ((rand() % 100) / 100.0f) * 12.0f);
                        float cSc = 0.70f + ((rand() % 100) / 100.0f) * 0.35f;
                        float cRot = (float)(rand() % 360);
                        oceanIslands.push_back({glm::vec3(cx, -7.5f, z - 120.0f), cSc, cRot});

                        // Reward energy ring hovering directly above island palm canopy!
                        rings.push_back({glm::vec3(cx, 2.5f, z - 120.0f), 3.2f, 0.0f, false, false});
                    }
                }
            }

            // ZONE B MARINE PROPS: Naval Fortresses & Anchored Cargo Ships in Bay Waters (Z = -2800 to -5000)
            for (float z = startZ - 60.0f; z >= endZ; z -= 380.0f) {
                if (z <= -2800.0f && z > -5000.0f) {
                    if (static_cast<int>(std::abs(z)) % 760 < 380) {
                        float fx = ((rand() % 2 == 0) ? -1.0f : 1.0f) * (68.0f + ((rand() % 100) / 100.0f) * 22.0f);
                        navalFortresses.push_back({glm::vec3(fx, -18.0f, z), 1.0f, (float)(rand() % 360)});
                    } else {
                        float sx = ((rand() % 2 == 0) ? -1.0f : 1.0f) * (72.0f + ((rand() % 100) / 100.0f) * 20.0f);
                        float sRot = (rand() % 2 == 0) ? 12.0f : -15.0f;
                        cargoShips.push_back({glm::vec3(sx, -18.0f, z), 1.0f, sRot});
                    }
                }
            }

            // ZONE C GANTRIES: Highway overhead road gantries (mainland highway only)
            for (float z = startZ - 120.0f; z >= endZ; z -= 450.0f) {
                if (z <= -5000.0f && z > -8500.0f) {
                    cityGantries.push_back({glm::vec3(0.0f, 0.0f, z), 54.0f});
                    if ((static_cast<int>(std::abs(z)) % 900) < 450) {
                        rings.push_back({glm::vec3(0.0f, 1.5f, z), 3.2f, 0.0f, false, false});
                    }
                }
            }

            // ZONE C SKYBRIDGES: Multi-level illuminated skybridges spanning across the corridor
            for (float sbZ : {-5640.0f, -6240.0f, -7260.0f, -7860.0f}) {
                if (startZ >= sbZ && endZ < sbZ) {
                    skybridges.push_back({glm::vec3(0.0f, -7.5f, sbZ), 1.0f});
                    rings.push_back({glm::vec3(0.0f, 1.0f, sbZ), 3.2f, 0.0f, false, false});
                }
            }

            // ZONE D PROPS (Z = -8500 to -11000): Energy Ring Gates along the canal channel
            for (float z = startZ - 60.0f; z >= endZ; z -= 320.0f) {
                if (z <= -8500.0f && z > -11000.0f) {
                    rings.push_back({glm::vec3(0.0f, -1.5f, z), 3.4f, 0.0f, false, false});
                }
            }

            // COLLAPSING COMMUNICATIONS SPIRES (Cinematic Hazards)
            if (startZ >= -6800.0f && endZ < -6800.0f) {
                SpawnCollapsingSpire(-6800.0f, -32.0f); // Zone C Downtown spire
            }
            if (startZ >= -10800.0f && endZ < -10800.0f) {
                SpawnCollapsingSpire(-10800.0f, 32.0f);  // Zone D Canal exit spire
            }

            // RARE GOLDEN CHALLENGE RINGS (Only ~5 in entire 10-minute level)
            for (float z = startZ - 200.0f; z >= endZ; z -= 2200.0f) {
                if (z > -12500.0f && z < -1000.0f) {
                    float sideX = ((rand() % 2 == 0) ? -1.0f : 1.0f) * 16.0f;
                    rings.push_back({glm::vec3(sideX, 4.0f, z), 3.2f, 0.0f, true, false});
                }
            }

            // ZONE-SPECIFIC BRUTALIST ARCHITECTURE (Buildings ONLY in Zone C Downtown and Zone D Canal!)
            // Zone A (Ocean), Zone B (Bay Causeway), and Zone E (Arena) are 100% free of land buildings
            for (float z = startZ - 15.0f; z >= endZ; z -= 40.0f) {
                if (z > -5000.0f || z <= -11000.0f) {
                    continue;
                }

                // ZONE C: DOWNTOWN CONCRETE JUNGLE (Z = -5000 to -8500) - SKY-HIGH MEGA SKYSCRAPERS & CROSS-STREET AVENUES!
                if (z > -8500.0f) {
                    // Check if near an intersection (within 36m)
                    bool nearIntersection = false;
                    for (float iz : {-5400.0f, -5940.0f, -6480.0f, -7020.0f, -7560.0f, -8160.0f}) {
                        if (std::abs(z - iz) < 36.0f) {
                            nearIntersection = true;
                            break;
                        }
                    }

                    if (nearIntersection) {
                        // At intersections: Keep central 40m cross avenue open!
                        // Flank the cross-avenue wings (X = ±80m to ±150m) with towering skyscrapers rotated 90 deg
                        float lx = -85.0f - ((rand() % 100) / 100.0f) * 45.0f;
                        float rx =  85.0f + ((rand() % 100) / 100.0f) * 45.0f;
                        int vL = (rand() % 3 == 0) ? 9 : (5 + rand() % 3);
                        int vR = (rand() % 3 == 0) ? 10 : (5 + rand() % 3);
                        buildings.push_back({glm::vec3(lx, -7.5f, z - 28.0f), 38.0f, 145.0f, 30.0f, 90.0f, vL, 1.0f});
                        buildings.push_back({glm::vec3(rx, -7.5f, z - 28.0f), 38.0f, 145.0f, 30.0f, -90.0f, vR, 1.0f});
                        buildings.push_back({glm::vec3(lx, -7.5f, z + 28.0f), 38.0f, 145.0f, 30.0f, 90.0f, vL, 1.0f});
                        buildings.push_back({glm::vec3(rx, -7.5f, z + 28.0f), 38.0f, 145.0f, 30.0f, -90.0f, vR, 1.0f});
                    } else {
                        // Varied sky-high variants (75m to 165m), ruined buildings, and neon billboard towers
                        auto pickVariant = [](int r) -> int {
                            int roll = r % 100;
                            if (roll < 18) return 8;               // Sheared Ruined Skyscraper (18%)
                            if (roll < 28) return 9;               // Cyber Arrow Nova Billboard Tower (10%)
                            if (roll < 38) return 10;              // Titan Heavy Dynamics Billboard Tower (10%)
                            if (roll < 46) return 11;              // Hyperion Starburst Billboard Tower (8%)
                            if (roll < 65) return 5;               // 125m Mega Skyscraper (19%)
                            if (roll < 82) return 6;               // 150m Mega Skyscraper (17%)
                            if (roll < 94) return 7;               // 165m Colossal Skyscraper (12%)
                            return 3;                              // 75m Skyscraper (6%)
                        };

                        int vL = pickVariant(rand());
                        int vR = pickVariant(rand());

                        float lx = -38.0f - ((rand() % 100) / 100.0f) * 6.0f;
                        float rx =  38.0f + ((rand() % 100) / 100.0f) * 6.0f;

                        // Ruined buildings face their sheared breach toward the flight corridor
                        float rotL = (vL == 8) ? 0.0f : ((vL >= 9 && vL <= 11) ? 0.0f : (float)(rand() % 2) * 180.0f);
                        float rotR = (vR == 8) ? 180.0f : ((vR >= 9 && vR <= 11) ? 0.0f : (float)(rand() % 2) * 180.0f);

                        auto getDims = [](int v, float& w, float& h, float& d) {
                            if (v == 8) { w = 32.0f; h = 110.0f; d = 26.0f; }
                            else if (v >= 9 && v <= 11) { w = 38.0f; h = 145.0f; d = 30.0f; }
                            else if (v == 7) { w = 48.0f; h = 165.0f; d = 36.0f; }
                            else if (v == 6) { w = 42.0f; h = 150.0f; d = 32.0f; }
                            else if (v == 5) { w = 36.0f; h = 125.0f; d = 28.0f; }
                            else { w = 30.0f; h = 75.0f; d = 22.0f; }
                        };

                        float wL, hL, dL, wR, hR, dR;
                        getDims(vL, wL, hL, dL);
                        getDims(vR, wR, hR, dR);

                        // Front-row towers right alongside highway
                        buildings.push_back({glm::vec3(lx, -7.5f, z), wL, hL, dL, rotL, vL, 1.0f});
                        buildings.push_back({glm::vec3(rx, -7.5f, z), wR, hR, dR, rotR, vR, 1.0f});

                        // Mid-row background skyline (X = ±72m)
                        int vMidL = (rand() % 2 == 0) ? 5 : 6;
                        int vMidR = (rand() % 2 == 0) ? 5 : 6;
                        buildings.push_back({glm::vec3(lx - 34.0f, -7.5f, z - 15.0f), 36.0f, 125.0f, 28.0f, 0.0f, vMidL, 1.0f});
                        buildings.push_back({glm::vec3(rx + 34.0f, -7.5f, z - 15.0f), 36.0f, 125.0f, 28.0f, 0.0f, vMidR, 1.0f});

                        // Deep background colossal skyline (X = ±115m)
                        if (static_cast<int>(std::abs(z)) % 80 == 0) {
                            buildings.push_back({glm::vec3(lx - 75.0f, -7.5f, z - 20.0f), 48.0f, 165.0f, 36.0f, 0.0f, 7, 1.0f});
                            buildings.push_back({glm::vec3(rx + 75.0f, -7.5f, z - 20.0f), 48.0f, 165.0f, 36.0f, 0.0f, 7, 1.0f});
                        }
                    }
                }
                // ZONE D: SUNKEN CANAL & AQUEDUCT DISTRICT (Z = -8500 to -11000) - Industrial fortress walls
                else {
                    float lx = -46.0f - ((rand() % 100) / 100.0f) * 24.0f;
                    float rx =  46.0f + ((rand() % 100) / 100.0f) * 24.0f;
                    int vL = rand() % 5;
                    int vR = rand() % 5;
                    buildings.push_back({glm::vec3(lx, -7.5f, z), 24.0f, 40.0f, 20.0f, (float)(rand() % 4) * 90.0f, vL, 1.0f});
                    buildings.push_back({glm::vec3(rx, -7.5f, z), 24.0f, 40.0f, 20.0f, (float)(rand() % 4) * 90.0f, vR, 1.0f});
                }
            }
        } else {
            // NON-CITY THEMES: Natural canyon plains, glacial spires, or desert dunes
            for (float z = startZ; z >= endZ; z -= sliceLen) {
                canyonSlices.push_back({glm::vec3(0.0f, 0.0f, z)});
            }

            // Rock Archways
            for (float z = startZ - 80.0f; z >= endZ; z -= 180.0f) {
                rockArches.push_back({glm::vec3(0.0f, 0.0f, z), 54.0f, 22.0f});
                rings.push_back({glm::vec3(0.0f, -0.5f, z), 3.2f, 0.0f, true, false});
            }

            // Trees
            for (float z = startZ - 12.0f; z >= endZ; z -= 22.0f) {
                float lx = -22.0f - ((float)rand() / RAND_MAX) * 85.0f;
                float lScale = 0.85f + ((float)rand() / RAND_MAX) * 0.45f;
                float lRot = ((float)rand() / RAND_MAX) * 360.0f;
                trees.push_back({glm::vec3(lx, -7.5f, z + ((float)rand() / RAND_MAX * 10.0f - 5.0f)), lScale, lRot});

                float rx = 22.0f + ((float)rand() / RAND_MAX) * 85.0f;
                float rScale = 0.85f + ((float)rand() / RAND_MAX) * 0.45f;
                float rRot = ((float)rand() / RAND_MAX) * 360.0f;
                trees.push_back({glm::vec3(rx, -7.5f, z + ((float)rand() / RAND_MAX * 10.0f - 5.0f)), rScale, rRot});
            }

            // Wind Turbines
            for (float z = startZ - 40.0f; z >= endZ; z -= 70.0f) {
                float side = ((rand() % 2) == 0) ? -1.0f : 1.0f;
                float tx = side * (32.0f + ((float)rand() / RAND_MAX) * 65.0f);
                windTurbines.push_back({
                    glm::vec3(tx, -7.5f, z),
                    ((float)rand() / RAND_MAX) * 360.0f,
                    55.0f + ((float)rand() / RAND_MAX) * 35.0f
                });
            }

            // Monolith Pillars
            for (float z = startZ - 40.0f; z >= endZ; z -= 80.0f) {
                float side = ((rand() % 2) == 0) ? -1.0f : 1.0f;
                float rx = side * (6.0f + ((float)rand() / RAND_MAX) * 35.0f);
                pillars.push_back({
                    glm::vec3(rx, 0.0f, z),
                    1.8f, 16.0f, 50.0f, false
                });
            }

            // Recovery Rings
            for (float z = startZ - 45.0f; z >= endZ; z -= 85.0f) {
                float rx = ((float)rand() / RAND_MAX * 2.0f - 1.0f) * 11.0f;
                float ry = -3.5f + ((float)rand() / RAND_MAX) * 7.5f;
                bool isGold = ((float)rand() / RAND_MAX) > 0.55f;
                rings.push_back({glm::vec3(rx, ry, z), 3.2f, 0.0f, isGold, false});
            }

            // Asteroids
            for (float z = startZ - 30.0f; z >= endZ; z -= 95.0f) {
                float rx = ((float)rand() / RAND_MAX * 2.0f - 1.0f) * 15.0f;
                float ry = 4.5f + ((float)rand() / RAND_MAX) * 4.5f;
                float radius = 1.8f + ((float)rand() / RAND_MAX) * 1.5f;
                glm::vec3 rotSpeed(
                    ((float)rand() / RAND_MAX * 2.0f - 1.0f) * 40.0f,
                    ((float)rand() / RAND_MAX * 2.0f - 1.0f) * 50.0f,
                    ((float)rand() / RAND_MAX * 2.0f - 1.0f) * 30.0f
                );
                asteroids.push_back({glm::vec3(rx, ry, z), glm::vec3(0.0f), rotSpeed, radius, false});
            }

            // Floating Domes
            for (float z = startZ - 60.0f; z >= endZ; z -= 140.0f) {
                float side = ((rand() % 2) == 0) ? -1.0f : 1.0f;
                floatingDomes.push_back({
                    glm::vec3(side * (36.0f + ((float)rand() / RAND_MAX) * 25.0f), 8.5f, z), 6.5f
                });
            }

            // Biome Props
            if (terrainTheme == "glacial") {
                for (float z = startZ - 15.0f; z >= endZ; z -= 30.0f) {
                    float lx = ((float)rand()/RAND_MAX * 2.0f - 1.0f) * 55.0f;
                    float sc = 0.7f + (float)rand()/RAND_MAX * 0.6f;
                    float rot = (float)rand()/RAND_MAX * 360.0f;
                    iceCrystals.push_back({glm::vec3(lx, 0.0f, z), sc, rot});
                    float rx = -lx * 0.7f + ((float)rand()/RAND_MAX - 0.5f) * 20.0f;
                    iceCrystals.push_back({glm::vec3(rx, 0.0f, z - 8.0f), sc*0.8f, rot+60.0f});
                }
            } else if (terrainTheme == "dune_pass") {
                for (float z = startZ - 20.0f; z >= endZ; z -= 38.0f) {
                    float lx = -20.0f - (float)rand()/RAND_MAX * 60.0f;
                    float sc = 0.8f + (float)rand()/RAND_MAX * 0.4f;
                    cacti.push_back({glm::vec3(lx, 0.0f, z), sc, (float)rand()/RAND_MAX*360.0f});
                    float rx = 20.0f + (float)rand()/RAND_MAX * 60.0f;
                    cacti.push_back({glm::vec3(rx, 0.0f, z - 10.0f), sc*0.9f, (float)rand()/RAND_MAX*360.0f});
                }
                for (float z = startZ - 60.0f; z >= endZ; z -= 120.0f) {
                    float px = ((float)rand()/RAND_MAX * 2.0f - 1.0f) * 80.0f;
                    float sc = 1.0f + (float)rand()/RAND_MAX * 0.8f;
                    desertPyramids.push_back({glm::vec3(px, 0.0f, z), sc});
                }
            }
        }
    } else {
        // Sector 2: Deep Space Debris Field
        // No canyon walls - vast open space with dense asteroid storm & tumbling space station wreckage!
        for (float z = startZ - 20.0f; z >= endZ; z -= 45.0f) {
            float rx = ((float)rand() / RAND_MAX * 2.0f - 1.0f) * 22.0f;
            float ry = ((float)rand() / RAND_MAX * 2.0f - 1.0f) * 16.0f;
            float radius = 2.0f + ((float)rand() / RAND_MAX) * 3.5f;
            glm::vec3 rotSpeed(
                ((float)rand() / RAND_MAX * 2.0f - 1.0f) * 50.0f,
                ((float)rand() / RAND_MAX * 2.0f - 1.0f) * 60.0f,
                ((float)rand() / RAND_MAX * 2.0f - 1.0f) * 40.0f
            );
            asteroids.push_back({
                glm::vec3(rx, ry, z),
                glm::vec3(0.0f),
                rotSpeed,
                radius,
                false
            });
        }

        // Industrial Space Station Wreckage Trusses
        for (float z = startZ - 60.0f; z >= endZ; z -= 90.0f) {
            float rx = ((float)rand() / RAND_MAX * 2.0f - 1.0f) * 18.0f;
            float ry = ((float)rand() / RAND_MAX * 2.0f - 1.0f) * 12.0f;
            glm::vec3 rotSpeed(
                ((float)rand() / RAND_MAX * 2.0f - 1.0f) * 25.0f,
                ((float)rand() / RAND_MAX * 2.0f - 1.0f) * 35.0f,
                ((float)rand() / RAND_MAX * 2.0f - 1.0f) * 20.0f
            );
            spaceDebris.push_back({
                glm::vec3(rx, ry, z),
                glm::vec3(0.0f),
                rotSpeed,
                7.0f
            });
        }

        // Sector 2 Energy Rings (Floating in open space)
        for (float z = startZ - 40.0f; z >= endZ; z -= 70.0f) {
            float rx = ((float)rand() / RAND_MAX * 2.0f - 1.0f) * 12.0f;
            float ry = ((float)rand() / RAND_MAX * 2.0f - 1.0f) * 9.0f;
            bool isGold = ((float)rand() / RAND_MAX) > 0.45f;
            rings.push_back({glm::vec3(rx, ry, z), 3.4f, 0.0f, isGold, false});
        }
    }

    nextSpawnZ = endZ;
}

void WorldEnvironment::Update(float playerZ, float dt, bool allRange) {
    lastPlayerZ = playerZ;
    isAllRangeActive = allRange;

    // Generate new chunks ahead as player advances (suppress when fighting in arena)
    if (!allRange && playerZ - 350.0f < nextSpawnZ) {
        GenerateChunk(nextSpawnZ, nextSpawnZ - 420.0f);
    }

    // Spin rings
    for (auto& r : rings) {
        r.rotation += 90.0f * dt;
        if (r.rotation > 360.0f) r.rotation -= 360.0f;
    }

    // Rotate asteroids
    for (auto& a : asteroids) {
        a.rotation += a.rotSpeed * dt;
    }

    // Rotate space debris
    for (auto& d : spaceDebris) {
        d.rotation += d.rotSpeed * dt;
    }

    // Rotate wind turbines
    for (auto& wt : windTurbines) {
        wt.rotation += wt.rotSpeed * dt;
        if (wt.rotation > 360.0f) wt.rotation -= 360.0f;
    }

    // Pulse secret relays
    for (auto& rel : secretRelays) {
        rel.pulseTimer += dt * 3.0f;
    }

    // Slowly cruise Cargo Ships in Zone B Bay Waters (~3 units/s forward heading)
    for (auto& cs : cargoShips) {
        float rad = glm::radians(cs.rotation);
        cs.position.z += std::cos(rad) * cs.speed * dt;
        cs.position.x += std::sin(rad) * cs.speed * dt;
    }

    // Cull objects behind player, but FREEZE culling in arena when in All-Range mode!
    float cullZ = playerZ + despawnDistBehind;

    canyonSlices.erase(
        std::remove_if(canyonSlices.begin(), canyonSlices.end(),
                       [cullZ, allRange](const CanyonSlice& c) {
                           if (allRange || c.position.z <= -12700.0f) return false;
                           return c.position.z > cullZ + 30.0f;
                       }),
        canyonSlices.end()
    );

    rockArches.erase(
        std::remove_if(rockArches.begin(), rockArches.end(),
                       [cullZ](const RockArchway& a) { return a.position.z > cullZ; }),
        rockArches.end()
    );

    pillars.erase(
        std::remove_if(pillars.begin(), pillars.end(),
                       [cullZ](const HazardPillar& p) { return p.position.z > cullZ || p.destroyed; }),
        pillars.end()
    );

    rings.erase(
        std::remove_if(rings.begin(), rings.end(),
                       [cullZ, allRange](const RingGate& r) {
                           if (allRange && r.position.z <= -12700.0f) return r.collected;
                           return r.position.z > cullZ || r.collected;
                       }),
        rings.end()
    );

    asteroids.erase(
        std::remove_if(asteroids.begin(), asteroids.end(),
                       [cullZ](const AsteroidObstacle& a) { return a.position.z > cullZ || a.destroyed; }),
        asteroids.end()
    );

    spaceDebris.erase(
        std::remove_if(spaceDebris.begin(), spaceDebris.end(),
                       [cullZ](const SpaceDebris& d) { return d.position.z > cullZ; }),
        spaceDebris.end()
    );

    floatingDomes.erase(
        std::remove_if(floatingDomes.begin(), floatingDomes.end(),
                       [cullZ](const FloatingDomeObstacle& d) { return d.position.z > cullZ; }),
        floatingDomes.end()
    );

    windTurbines.erase(
        std::remove_if(windTurbines.begin(), windTurbines.end(),
                       [cullZ](const WindTurbineObstacle& wt) { return wt.position.z > cullZ; }),
        windTurbines.end()
    );

    trees.erase(
        std::remove_if(trees.begin(), trees.end(),
                       [cullZ](const TreeObstacle& t) { return t.position.z > cullZ; }),
        trees.end()
    );

    iceCrystals.erase(
        std::remove_if(iceCrystals.begin(), iceCrystals.end(),
                       [cullZ](const IceCrystalSpire& c) { return c.position.z > cullZ; }),
        iceCrystals.end()
    );

    cacti.erase(
        std::remove_if(cacti.begin(), cacti.end(),
                       [cullZ](const CactusObstacle& c) { return c.position.z > cullZ; }),
        cacti.end()
    );

    desertPyramids.erase(
        std::remove_if(desertPyramids.begin(), desertPyramids.end(),
                       [cullZ](const DesertPyramidObstacle& d) { return d.position.z > cullZ; }),
        desertPyramids.end()
    );

    oceanIslands.erase(
        std::remove_if(oceanIslands.begin(), oceanIslands.end(),
                       [cullZ](const OceanIslandObstacle& oi) { return oi.position.z > cullZ; }),
        oceanIslands.end()
    );

    navalFortresses.erase(
        std::remove_if(navalFortresses.begin(), navalFortresses.end(),
                       [cullZ](const NavalFortressObstacle& nf) { return nf.position.z > cullZ; }),
        navalFortresses.end()
    );

    cargoShips.erase(
        std::remove_if(cargoShips.begin(), cargoShips.end(),
                       [cullZ](const CargoShipObstacle& cs) { return cs.position.z > cullZ; }),
        cargoShips.end()
    );

    buildings.erase(
        std::remove_if(buildings.begin(), buildings.end(),
                       [cullZ, allRange](const BuildingObstacle& b) {
                           if (allRange || b.position.z <= -12700.0f) return false;
                           return b.position.z > cullZ;
                       }),
        buildings.end()
    );

    cityGantries.erase(
        std::remove_if(cityGantries.begin(), cityGantries.end(),
                       [cullZ, allRange](const CityGantryObstacle& g) {
                           if (allRange || g.position.z <= -12700.0f) return false;
                           return g.position.z > cullZ;
                       }),
        cityGantries.end()
    );

    skybridges.erase(
        std::remove_if(skybridges.begin(), skybridges.end(),
                       [cullZ](const SkybridgeObstacle& sb) { return sb.position.z > cullZ; }),
        skybridges.end()
    );

    // Update and animate Collapsing Antenna Spires
    for (auto& sp : collapsingSpires) {
        float dz = sp.position.z - playerZ;
        if (!sp.isTriggered && dz > -sp.triggerDist && dz < 40.0f) {
            sp.isTriggered = true;
        }
        if (sp.isTriggered && !sp.destroyed) {
            sp.currentAngle = std::min(sp.targetAngle, sp.currentAngle + sp.collapseSpeed * dt);
        }
    }
    collapsingSpires.erase(
        std::remove_if(collapsingSpires.begin(), collapsingSpires.end(),
                       [cullZ, allRange](const CollapsingSpireHazard& s) {
                           if (allRange || s.position.z <= -12700.0f) return false;
                           return s.position.z > cullZ;
                       }),
        collapsingSpires.end()
    );
}

void WorldEnvironment::Draw(const Shader& shader) const {
    if (currentSector == SectorStage::Sector1_Canyon) {
        // 0. Draw Distant Arcade Horizon (Vibrant Multi-Band Sky Dome & Low-Poly Sawtooth Peaks)
        shader.SetInt("uUseLighting", 0);
        shader.SetInt("uUseFog", 0);
        shader.SetFloat("uAlpha", 1.0f);
        glm::mat4 horizonModel = glm::mat4(1.0f);
        horizonModel = glm::translate(horizonModel, glm::vec3(0.0f, 0.0f, lastPlayerZ));
        shader.SetMat4("uModel", horizonModel);
        horizonMesh.Draw(shader);

        shader.SetInt("uUseLighting", 1);
        shader.SetInt("uUseFog", 1);

        // 1. Draw Open Field Ground Slices — alternate even/odd mesh for seamless checkerboard
        const float sliceLen = 60.0f;
        for (const auto& cs : canyonSlices) {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, cs.position);
            shader.SetMat4("uModel", model);
            if (cs.sliceType == TerrainSliceType::Coastline) {
                coastlineMesh.Draw(shader);
            } else if (cs.sliceType == TerrainSliceType::ElevatedHighway) {
                elevatedHighwayMesh.Draw(shader);
            } else if (cs.sliceType == TerrainSliceType::CityRoad) {
                roadMesh.Draw(shader);
            } else if (cs.sliceType == TerrainSliceType::CityIntersection) {
                cityIntersectionMesh.Draw(shader);
            } else if (cs.sliceType == TerrainSliceType::SunkenCanal) {
                canalMesh.Draw(shader);
            } else if (cs.sliceType == TerrainSliceType::HighwayTransition) {
                highwayTransitionMesh.Draw(shader);
            } else {
                // Each slab covers 3 tile rows (tileSize=20, sliceLen=60). 3 is odd,
                // so parity flips at every slab boundary. Pick even/odd mesh accordingly.
                int slabIdx = static_cast<int>(std::round(-cs.position.z / sliceLen));
                if (std::abs(slabIdx) % 2 == 0) {
                    canyonMesh.Draw(shader);
                } else {
                    canyonMeshOdd.Draw(shader);
                }
            }
        }

        // 1b. Draw Winding Coastal Highway Transition Set-Piece (Z = -2200 to -2800)
        if (terrainTheme == "city" && lastPlayerZ >= -3600.0f && lastPlayerZ <= -1600.0f) {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(0.0f, 0.0f, -2500.0f));
            shader.SetMat4("uModel", model);
            highwayTransitionMesh.Draw(shader);
        }

        // 1c. Draw Winding Elevated Highway & Suspension Bridge Set-Piece (Z = -2800 to -5000)
        if (terrainTheme == "city" && lastPlayerZ >= -5800.0f && lastPlayerZ <= -2200.0f) {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(0.0f, 0.0f, -3900.0f));
            shader.SetMat4("uModel", model);
            elevatedHighwayMesh.Draw(shader);
        }

        // 1d. Draw Tropical Archipelago Islands in Zone A Ocean
        for (const auto& oi : oceanIslands) {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, oi.position);
            model = glm::rotate(model, glm::radians(oi.rotation), glm::vec3(0.0f, 1.0f, 0.0f));
            model = glm::scale(model, glm::vec3(oi.scale));
            shader.SetMat4("uModel", model);
            oceanIslandMesh.Draw(shader);
        }

        // 1e. Draw Naval Fortresses in Zone B Bay Waters
        for (const auto& nf : navalFortresses) {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, nf.position);
            model = glm::rotate(model, glm::radians(nf.rotation), glm::vec3(0.0f, 1.0f, 0.0f));
            model = glm::scale(model, glm::vec3(nf.scale));
            shader.SetMat4("uModel", model);
            navalFortressMesh.Draw(shader);
        }

        // 1f. Draw Anchored Cargo Ships in Zone B Bay Waters
        for (const auto& cs : cargoShips) {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, cs.position);
            model = glm::rotate(model, glm::radians(cs.rotation), glm::vec3(0.0f, 1.0f, 0.0f));
            model = glm::scale(model, glm::vec3(cs.scale));
            shader.SetMat4("uModel", model);
            cargoShipMesh.Draw(shader);
        }

        // 1g. Draw Elevated Skybridges across Downtown Canyon (Zone C)
        for (const auto& sb : skybridges) {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, sb.position);
            model = glm::scale(model, glm::vec3(sb.scale));
            shader.SetMat4("uModel", model);
            skybridgeMesh.Draw(shader);
        }

        // 2. Draw Rock Arches
        for (const auto& arch : rockArches) {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, arch.position);
            shader.SetMat4("uModel", model);
            rockArchMesh.Draw(shader);
        }

        // 3. Draw Hazard Monolith Pillars
        for (const auto& pil : pillars) {
            if (pil.destroyed) continue;
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, pil.position);
            shader.SetMat4("uModel", model);
            pillarMesh.Draw(shader);
        }

        // 4. Draw Floating High-Tech Dome Pavilions (Ex-Zodiac Image 1)
        for (const auto& dome : floatingDomes) {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, dome.position);
            shader.SetMat4("uModel", model);
            domeMesh.Draw(shader);
        }

        // 5. Draw Giant Wind Turbines along plains: Stationary Tower + Rotating Blades (Ex-Zodiac Image 3)
        for (const auto& wt : windTurbines) {
            // Stationary tower standing firmly upright
            glm::mat4 towerModel = glm::mat4(1.0f);
            towerModel = glm::translate(towerModel, wt.position);
            shader.SetMat4("uModel", towerModel);
            turbineTowerMesh.Draw(shader);

            // Rotating 3-blade propeller at the nacelle hub
            glm::mat4 bladeModel = glm::mat4(1.0f);
            bladeModel = glm::translate(bladeModel, wt.position + glm::vec3(0.0f, 26.0f, 2.0f));
            bladeModel = glm::rotate(bladeModel, glm::radians(wt.rotation), glm::vec3(0.0f, 0.0f, 1.0f));
            shader.SetMat4("uModel", bladeModel);
            turbineBladesMesh.Draw(shader);
        }

        // 5b. Draw Low-Poly Trees scattered across the plains (Ex-Zodiac Image 3)
        for (const auto& tr : trees) {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, tr.position);
            model = glm::rotate(model, glm::radians(tr.rotation), glm::vec3(0.0f, 1.0f, 0.0f));
            model = glm::scale(model, glm::vec3(tr.scale));
            shader.SetMat4("uModel", model);
            treeMesh.Draw(shader);
        }

        // 9b. Draw Glacial Ice Crystals
        for (const auto& ic : iceCrystals) {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, ic.position);
            model = glm::rotate(model, glm::radians(ic.rotation), glm::vec3(0.0f, 1.0f, 0.0f));
            model = glm::scale(model, glm::vec3(ic.scale));
            shader.SetMat4("uModel", model);
            iceCrystalMesh.Draw(shader);
        }

        // 9c. Draw Desert Cacti
        for (const auto& ca : cacti) {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, ca.position);
            model = glm::rotate(model, glm::radians(ca.rotation), glm::vec3(0.0f, 1.0f, 0.0f));
            model = glm::scale(model, glm::vec3(ca.scale));
            shader.SetMat4("uModel", model);
            cactusMesh.Draw(shader);
        }

        // 9d. Draw Desert Pyramids
        for (const auto& dp : desertPyramids) {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, dp.position);
            model = glm::scale(model, glm::vec3(dp.scale));
            shader.SetMat4("uModel", model);
            desertPyramidMesh.Draw(shader);
        }

        // 9e. Draw Buildings and Rubble (Fixed persistent mesh variant per building!)
        for (const auto& b : buildings) {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, b.position);
            model = glm::rotate(model, glm::radians(b.rotation), glm::vec3(0.0f, 1.0f, 0.0f));
            shader.SetMat4("uModel", model);

            if (b.meshVariant == 8) {
                ruinedBuildingMesh.Draw(shader);
                // Draw collapsed rubble mounds at the breach base
                for (float rxOffset : {-8.0f, 12.0f}) {
                    glm::mat4 rm = glm::mat4(1.0f);
                    rm = glm::translate(rm, b.position + glm::vec3(rxOffset, 0.0f, 6.0f));
                    shader.SetMat4("uModel", rm);
                    rubbleMesh.Draw(shader);
                }
            } else if (b.meshVariant >= 9 && b.meshVariant <= 11) {
                billboardBuildingMesh[b.meshVariant - 9].Draw(shader);
            } else {
                buildingMeshes[std::clamp(b.meshVariant, 0, 7)].Draw(shader);
            }
        }

        // 9f. Draw Highway Overhead Gantries
        for (const auto& g : cityGantries) {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, g.position);
            shader.SetMat4("uModel", model);
            gantryMesh.Draw(shader);
        }

        // 9f2. Draw Collapsing Communication Antenna Spires
        for (const auto& sp : collapsingSpires) {
            if (sp.destroyed) continue;
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, sp.position);
            model = glm::rotate(model, glm::radians(sp.currentAngle), glm::vec3(0.0f, 0.0f, 1.0f));
            shader.SetMat4("uModel", model);
            spireMesh.Draw(shader);
        }

        // 9g. Draw Grand Boss Arena Plaza & Stadium Perimeter Columns
        if (arenaSpawned) {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, arenaCenter);
            shader.SetMat4("uModel", model);
            arenaPlazaMesh.Draw(shader);

            // 16 colossal perimeter stadium floodlight pillars surrounding the arena boundary
            for (int p = 0; p < 16; ++p) {
                float a = p * (glm::two_pi<float>() / 16.0f);
                glm::vec3 pPos = arenaCenter + glm::vec3(std::cos(a) * 285.0f, 0.0f, std::sin(a) * 285.0f);
                glm::mat4 pModel = glm::mat4(1.0f);
                pModel = glm::translate(pModel, pPos);
                shader.SetMat4("uModel", pModel);
                arenaPillarMesh.Draw(shader);
            }
        }

        // 6. Draw Secret Planetary Radar Relays
        for (const auto& rel : secretRelays) {
            if (rel.destroyed) continue;
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, rel.position);
            shader.SetMat4("uModel", model);
            relayMesh.Draw(shader);
        }
    } else {
        // Sector 2: Draw Space Debris Girders
        shader.SetInt("uUseLighting", 1);
        shader.SetInt("uUseFog", 1);
        shader.SetFloat("uAlpha", 1.0f);
        for (const auto& d : spaceDebris) {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, d.position);
            model = glm::rotate(model, glm::radians(d.rotation.x), glm::vec3(1, 0, 0));
            model = glm::rotate(model, glm::radians(d.rotation.y), glm::vec3(0, 1, 0));
            model = glm::rotate(model, glm::radians(d.rotation.z), glm::vec3(0, 0, 1));
            shader.SetMat4("uModel", model);
            debrisMesh.Draw(shader);
        }
    }

    // 7. Draw Asteroids (in both sectors)
    for (const auto& a : asteroids) {
        if (a.destroyed) continue;
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, a.position);
        model = glm::rotate(model, glm::radians(a.rotation.x), glm::vec3(1, 0, 0));
        model = glm::rotate(model, glm::radians(a.rotation.y), glm::vec3(0, 1, 0));
        model = glm::rotate(model, glm::radians(a.rotation.z), glm::vec3(0, 0, 1));
        model = glm::scale(model, glm::vec3(a.radius / 2.4f));
        shader.SetMat4("uModel", model);
        asteroidMesh.Draw(shader);
    }

    // 8. Draw Rings (unlit glowing)
    shader.SetInt("uUseLighting", 0);
    shader.SetFloat("uAlpha", 0.95f);
    for (const auto& r : rings) {
        if (r.collected) continue;
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, r.position);
        model = glm::rotate(model, glm::radians(r.rotation), glm::vec3(0, 0, 1));
        shader.SetMat4("uModel", model);
        if (r.isGold) {
            goldRingMesh.Draw(shader);
        } else {
            silverRingMesh.Draw(shader);
        }
    }

    // Restore standard lighting and fog state for subsequent passes
    shader.SetInt("uUseLighting", 1);
    shader.SetInt("uUseFog", 1);
    shader.SetFloat("uAlpha", 1.0f);
}

void WorldEnvironment::Clear() {
    canyonSlices.clear();
    rockArches.clear();
    pillars.clear();
    rings.clear();
    asteroids.clear();
    secretRelays.clear();
    spaceDebris.clear();
    floatingDomes.clear();
    windTurbines.clear();
    trees.clear();
    iceCrystals.clear();
    cacti.clear();
    desertPyramids.clear();
    oceanIslands.clear();
    navalFortresses.clear();
    cargoShips.clear();
    buildings.clear();
    cityGantries.clear();
    collapsingSpires.clear();
    skybridges.clear();
    arenaSpawned = false;
    isAllRangeActive = false;
    nextSpawnZ = 0.0f;
    lastPlayerZ = 0.0f;

    if (currentSector == SectorStage::Sector1_Canyon) {
        if (terrainTheme != "city") {
            // Spawn 3 Secret Planetary Radar Relays at archway coordinates (canyon theme only)
            secretRelays.push_back({glm::vec3(0.0f, 1.8f, -260.0f), 2.5f, 60.0f, 60.0f, false, 0.0f});
            secretRelays.push_back({glm::vec3(-6.5f, 3.2f, -480.0f), 2.5f, 60.0f, 60.0f, false, 0.0f});
            secretRelays.push_back({glm::vec3(5.5f, 2.4f, -740.0f), 2.5f, 60.0f, 60.0f, false, 0.0f});
        }
        GenerateChunk(30.0f, -420.0f);
    } else {
        // Sector 2: Deep Space chunk
        GenerateChunk(30.0f, -420.0f);
    }
}
