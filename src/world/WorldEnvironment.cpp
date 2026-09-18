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
      nextSpawnZ(0.0f),
      despawnDistBehind(60.0f),
      lastPlayerZ(0.0f) {
    buildingMeshes[0] = Mesh::CreateBuilding(18.0f, 35.0f, 14.0f, glm::vec3(0.22f,0.24f,0.28f), glm::vec3(0.15f,0.60f,0.85f), glm::vec3(0.18f,0.20f,0.24f), 5, 3);
    buildingMeshes[1] = Mesh::CreateBuilding(24.0f, 55.0f, 18.0f, glm::vec3(0.20f,0.22f,0.26f), glm::vec3(0.90f,0.25f,0.55f), glm::vec3(0.15f,0.17f,0.20f), 7, 4);
    buildingMeshes[2] = Mesh::CreateBuilding(14.0f, 28.0f, 12.0f, glm::vec3(0.25f,0.26f,0.30f), glm::vec3(0.20f,0.75f,0.50f), glm::vec3(0.20f,0.22f,0.25f), 4, 2);
    buildingMeshes[3] = Mesh::CreateBuilding(30.0f, 75.0f, 22.0f, glm::vec3(0.18f,0.20f,0.24f), glm::vec3(0.85f,0.62f,0.15f), glm::vec3(0.14f,0.16f,0.20f), 9, 5);
    buildingMeshes[4] = Mesh::CreateBuilding(10.0f, 18.0f, 10.0f, glm::vec3(0.28f,0.28f,0.32f), glm::vec3(0.40f,0.80f,0.95f), glm::vec3(0.22f,0.22f,0.25f), 3, 2);
    Clear();
}

void WorldEnvironment::SetSector(SectorStage sector) {
    currentSector = sector;
    Clear();
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

        // 1. Generate seamless open field slices (wide expanse, no side walls)
        for (float z = startZ; z >= endZ; z -= sliceLen) {
            canyonSlices.push_back({glm::vec3(0.0f, 0.0f, z)});
        }

        // 2. Generate massive Low-Poly Rock Archways across the open plains
        for (float z = startZ - 80.0f; z >= endZ; z -= 180.0f) {
            rockArches.push_back({glm::vec3(0.0f, 0.0f, z), 54.0f, 22.0f});
            // Golden Ring nested under arch
            rings.push_back({glm::vec3(0.0f, -0.5f, z), 3.2f, 0.0f, true, false});
        }

        // 3. Generate Low-Poly Trees scattered across the wide open plains (Ex-Zodiac Image 3)
        for (float z = startZ - 12.0f; z >= endZ; z -= 22.0f) {
            // Left flank grove
            float lx = -22.0f - ((float)rand() / RAND_MAX) * 85.0f;
            float lScale = 0.85f + ((float)rand() / RAND_MAX) * 0.45f;
            float lRot = ((float)rand() / RAND_MAX) * 360.0f;
            trees.push_back({glm::vec3(lx, -7.5f, z + ((float)rand() / RAND_MAX * 10.0f - 5.0f)), lScale, lRot});

            // Right flank grove
            float rx = 22.0f + ((float)rand() / RAND_MAX) * 85.0f;
            float rScale = 0.85f + ((float)rand() / RAND_MAX) * 0.45f;
            float rRot = ((float)rand() / RAND_MAX) * 360.0f;
            trees.push_back({glm::vec3(rx, -7.5f, z + ((float)rand() / RAND_MAX * 10.0f - 5.0f)), rScale, rRot});
        }

        // 4. Giant Wind Turbines along the open plains (Ex-Zodiac Image 3)
        for (float z = startZ - 40.0f; z >= endZ; z -= 70.0f) {
            float side = ((rand() % 2) == 0) ? -1.0f : 1.0f;
            float tx = side * (32.0f + ((float)rand() / RAND_MAX) * 65.0f);
            windTurbines.push_back({
                glm::vec3(tx, -7.5f, z),
                ((float)rand() / RAND_MAX) * 360.0f,
                55.0f + ((float)rand() / RAND_MAX) * 35.0f
            });
        }

        // 5. Generate Hazard Monolith Pillars on the plains
        for (float z = startZ - 40.0f; z >= endZ; z -= 80.0f) {
            float side = ((rand() % 2) == 0) ? -1.0f : 1.0f;
            float rx = side * (6.0f + ((float)rand() / RAND_MAX) * 35.0f);
            pillars.push_back({
                glm::vec3(rx, 0.0f, z),
                1.8f,   // Collision radius
                16.0f,  // Height
                50.0f,  // Health
                false
            });
        }

        // 6. Generate Energy Recovery Rings
        for (float z = startZ - 45.0f; z >= endZ; z -= 85.0f) {
            float rx = ((float)rand() / RAND_MAX * 2.0f - 1.0f) * 11.0f;
            float ry = -3.5f + ((float)rand() / RAND_MAX) * 7.5f;
            bool isGold = ((float)rand() / RAND_MAX) > 0.55f;
            rings.push_back({glm::vec3(rx, ry, z), 3.2f, 0.0f, isGold, false});
        }

        // 7. High-altitude floating asteroid debris
        for (float z = startZ - 30.0f; z >= endZ; z -= 95.0f) {
            float rx = ((float)rand() / RAND_MAX * 2.0f - 1.0f) * 15.0f;
            float ry = 4.5f + ((float)rand() / RAND_MAX) * 4.5f;
            float radius = 1.8f + ((float)rand() / RAND_MAX) * 1.5f;
            glm::vec3 rotSpeed(
                ((float)rand() / RAND_MAX * 2.0f - 1.0f) * 40.0f,
                ((float)rand() / RAND_MAX * 2.0f - 1.0f) * 50.0f,
                ((float)rand() / RAND_MAX * 2.0f - 1.0f) * 30.0f
            );
            asteroids.push_back({
                glm::vec3(rx, ry, z),
                glm::vec3(0.0f),
                rotSpeed,
                radius,
                false
            });
        }

        // 8. Floating High-Tech Dome Pavilions (Ex-Zodiac Image 1)
        for (float z = startZ - 60.0f; z >= endZ; z -= 140.0f) {
            float side = ((rand() % 2) == 0) ? -1.0f : 1.0f;
            floatingDomes.push_back({
                glm::vec3(side * (36.0f + ((float)rand() / RAND_MAX) * 25.0f), 8.5f, z),
                6.5f
            });
        }

        // 9. Biome-specific props
        if (terrainTheme == "glacial") {
            // Scatter ice crystal clusters on both flanks
            for (float z = startZ - 15.0f; z >= endZ; z -= 30.0f) {
                float lx = ((float)rand()/RAND_MAX * 2.0f - 1.0f) * 55.0f;
                float sc = 0.7f + (float)rand()/RAND_MAX * 0.6f;
                float rot = (float)rand()/RAND_MAX * 360.0f;
                iceCrystals.push_back({glm::vec3(lx, 0.0f, z), sc, rot});
                float rx = -lx * 0.7f + ((float)rand()/RAND_MAX - 0.5f) * 20.0f;
                iceCrystals.push_back({glm::vec3(rx, 0.0f, z - 8.0f), sc*0.8f, rot+60.0f});
            }
        } else if (terrainTheme == "dune_pass") {
            // Scatter cacti on both flanks
            for (float z = startZ - 20.0f; z >= endZ; z -= 38.0f) {
                float lx = -20.0f - (float)rand()/RAND_MAX * 60.0f;
                float sc = 0.8f + (float)rand()/RAND_MAX * 0.4f;
                cacti.push_back({glm::vec3(lx, 0.0f, z), sc, (float)rand()/RAND_MAX*360.0f});
                float rx = 20.0f + (float)rand()/RAND_MAX * 60.0f;
                cacti.push_back({glm::vec3(rx, 0.0f, z - 10.0f), sc*0.9f, (float)rand()/RAND_MAX*360.0f});
            }
            // Scatter distant pyramids at sparser interval
            for (float z = startZ - 60.0f; z >= endZ; z -= 120.0f) {
                float px = ((float)rand()/RAND_MAX * 2.0f - 1.0f) * 80.0f;
                float sc = 1.0f + (float)rand()/RAND_MAX * 0.8f;
                desertPyramids.push_back({glm::vec3(px, 0.0f, z), sc});
            }
        } else if (terrainTheme == "city") {
            for (float z = startZ - 10.0f; z >= endZ; z -= 45.0f) {
                // Left buildings
                float lx = -45.0f - (float)rand()/RAND_MAX * 75.0f;
                buildings.push_back({glm::vec3(lx, -7.5f, z + ((float)rand()/RAND_MAX*10.0f - 5.0f)),
                                     0.0f, 0.0f, 0.0f, (float)rand()/RAND_MAX * 360.0f});
                // Right buildings
                float rx = 45.0f + (float)rand()/RAND_MAX * 75.0f;
                buildings.push_back({glm::vec3(rx, -7.5f, z + ((float)rand()/RAND_MAX*10.0f - 5.0f)),
                                     0.0f, 0.0f, 0.0f, (float)rand()/RAND_MAX * 360.0f});
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

void WorldEnvironment::Update(float playerZ, float dt) {
    lastPlayerZ = playerZ;

    // Generate new chunks ahead as player advances
    if (playerZ - 350.0f < nextSpawnZ) {
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

    // Cull objects behind player
    float cullZ = playerZ + despawnDistBehind;

    canyonSlices.erase(
        std::remove_if(canyonSlices.begin(), canyonSlices.end(),
                       [cullZ](const CanyonSlice& c) { return c.position.z > cullZ + 30.0f; }),
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
                       [cullZ](const RingGate& r) { return r.position.z > cullZ || r.collected; }),
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

    buildings.erase(
        std::remove_if(buildings.begin(), buildings.end(),
                       [cullZ](const BuildingObstacle& b) { return b.position.z > cullZ; }),
        buildings.end()
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
            // Each slab covers 3 tile rows (tileSize=20, sliceLen=60). 3 is odd,
            // so parity flips at every slab boundary. Pick even/odd mesh accordingly.
            int slabIdx = static_cast<int>(std::round(-cs.position.z / sliceLen));
            if (std::abs(slabIdx) % 2 == 0) {
                canyonMesh.Draw(shader);
            } else {
                canyonMeshOdd.Draw(shader);
            }
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

        // 9e. Draw Buildings and Rubble
        int bIdx = 0;
        for (const auto& b : buildings) {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, b.position);
            model = glm::rotate(model, glm::radians(b.rotation), glm::vec3(0.0f, 1.0f, 0.0f));
            shader.SetMat4("uModel", model);
            buildingMeshes[bIdx % 5].Draw(shader);
            
            // Draw rubble near the building
            glm::mat4 rubbleModel = glm::mat4(1.0f);
            rubbleModel = glm::translate(rubbleModel, b.position + glm::vec3(15.0f, 0.0f, 10.0f));
            shader.SetMat4("uModel", rubbleModel);
            rubbleMesh.Draw(shader);
            
            bIdx++;
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
    buildings.clear();
    nextSpawnZ = 0.0f;
    lastPlayerZ = 0.0f;

    if (currentSector == SectorStage::Sector1_Canyon) {
        // Spawn 3 Secret Planetary Radar Relays at archway coordinates
        // Relay 1: Archway 1 at Z = -260
        secretRelays.push_back({glm::vec3(0.0f, 1.8f, -260.0f), 2.5f, 60.0f, 60.0f, false, 0.0f});
        // Relay 2: Archway 2 at Z = -480 (elevated on left shelf)
        secretRelays.push_back({glm::vec3(-6.5f, 3.2f, -480.0f), 2.5f, 60.0f, 60.0f, false, 0.0f});
        // Relay 3: Archway 3 at Z = -740 (tucked on right ledge)
        secretRelays.push_back({glm::vec3(5.5f, 2.4f, -740.0f), 2.5f, 60.0f, 60.0f, false, 0.0f});

        GenerateChunk(30.0f, -420.0f);
    } else {
        // Sector 2: Deep Space chunk
        GenerateChunk(30.0f, -420.0f);
    }
}
