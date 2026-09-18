#include "TrainConvoy.h"
#include "SoundManager.h"
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>
#include <cstdlib>

TrainConvoy::TrainConvoy(float xPosition)
    : active(false),
      locomotivePos(xPosition, -7.5f, 0.0f),
      speed(26.0f),
      trackX(xPosition),
      locoMesh(Mesh::CreateTrainLocomotive(
          glm::vec3(0.18f, 0.20f, 0.25f), // Midnight industrial steel
          glm::vec3(0.85f, 0.22f, 0.18f)  // Hazard crimson trim
      )),
      cargoMesh(Mesh::CreateTrainCargoCar(
          glm::vec3(0.22f, 0.25f, 0.30f), // Flatbed chassis
          glm::vec3(0.75f, 0.46f, 0.15f)  // Industrial amber freight container
      )),
      tracksMesh(Mesh::CreateRailTracks(
          320.0f, 2.4f,
          glm::vec3(0.55f, 0.58f, 0.62f), // Polished steel rails
          glm::vec3(0.30f, 0.22f, 0.16f)  // Timber railway ties
      )),
      turretMesh(Mesh::CreateFlakTurret(
          glm::vec3(0.35f, 0.38f, 0.42f),
          glm::vec3(0.20f, 0.22f, 0.24f)
      )),
      smokeTimer(0.0f),
      hornTimer(0.0f) {
    Reset();
}

void TrainConvoy::Reset() {
    active = false;
    smokeTimer = 0.0f;
    hornTimer = 0.0f;
    cars.clear();
}

void TrainConvoy::Spawn(float playerZ, float trackXPos) {
    active = true;
    trackX = trackXPos;
    locomotivePos = glm::vec3(trackX, -7.5f, playerZ - 240.0f);

    cars.clear();
    for (int i = 0; i < 4; ++i) {
        TrainCar car;
        car.position = locomotivePos - glm::vec3(0.0f, 0.0f, (i + 1) * 9.5f);
        car.health = 80.0f;
        car.destroyed = false;
        car.hasTurret = (i == 1); // Second railcar carries an anti-air flak turret!
        car.fireTimer = 0.8f;
        cars.push_back(car);
    }
}

void TrainConvoy::Update(float dt, float playerZ, const glm::vec3& playerPos,
                         ProjectileManager& projectiles, ParticleSystem& particles,
                         SoundManager* audio) {
    if (!active) return;

    // Train speeds along the track towards positive Z (meeting player)
    locomotivePos.z += speed * dt;
    locomotivePos.x = trackX;
    locomotivePos.y = -7.5f;

    // Chugging steam & black smoke from smokestack
    smokeTimer -= dt;
    if (smokeTimer <= 0.0f) {
        smokeTimer = 0.08f;
        glm::vec3 stackPos = locomotivePos + glm::vec3(0.0f, 3.2f, 2.8f);
        particles.SpawnExplosion(stackPos, 4, glm::vec3(0.35f, 0.35f, 0.38f));
    }

    // Railcars follow behind locomotive
    for (size_t i = 0; i < cars.size(); ++i) {
        cars[i].position = locomotivePos - glm::vec3(0.0f, 0.0f, (i + 1) * 9.5f);

        if (!cars[i].destroyed && cars[i].hasTurret) {
            cars[i].fireTimer -= dt;
            float distZ = cars[i].position.z - playerPos.z;
            if (distZ < -20.0f && distZ > -160.0f && cars[i].fireTimer <= 0.0f) {
                cars[i].fireTimer = 1.6f;
                glm::vec3 flakMuzzle = cars[i].position + glm::vec3(0.0f, 3.2f, 0.0f);
                projectiles.SpawnLaser(flakMuzzle, playerPos, false, 85.0f);
                if (audio) audio->Play(SoundID::EnemyLaser, 0.45f, 1.1f);
            }
        }
    }

    // Deactivate if entire train has moved past player
    if (!cars.empty() && cars.back().position.z > playerPos.z + 80.0f) {
        active = false;
    }
}

void TrainConvoy::Draw(const Shader& shader, float playerZ) const {
    if (!active) return;

    shader.SetInt("uUseLighting", 1);
    shader.SetFloat("uAlpha", 1.0f);

    // 1. Render Continuous Rail Tracks on Ground along trackX
    glm::mat4 mTracks = glm::mat4(1.0f);
    mTracks = glm::translate(mTracks, glm::vec3(trackX, -7.5f, playerZ - 60.0f));
    shader.SetMat4("uModel", mTracks);
    tracksMesh.Draw(shader);

    glm::mat4 mTracks2 = glm::mat4(1.0f);
    mTracks2 = glm::translate(mTracks2, glm::vec3(trackX, -7.5f, playerZ + 200.0f));
    shader.SetMat4("uModel", mTracks2);
    tracksMesh.Draw(shader);

    // 2. Render Locomotive
    glm::mat4 mLoco = glm::mat4(1.0f);
    mLoco = glm::translate(mLoco, locomotivePos);
    shader.SetMat4("uModel", mLoco);
    locoMesh.Draw(shader);

    // 3. Render Cargo Cars
    for (const auto& car : cars) {
        if (car.destroyed) continue;

        glm::mat4 mCar = glm::mat4(1.0f);
        mCar = glm::translate(mCar, car.position);
        shader.SetMat4("uModel", mCar);
        cargoMesh.Draw(shader);

        if (car.hasTurret) {
            glm::mat4 mTurret = mCar;
            mTurret = glm::translate(mTurret, glm::vec3(0.0f, 2.2f, 0.0f));
            shader.SetMat4("uModel", mTurret);
            turretMesh.Draw(shader);
        }
    }
}

bool TrainConvoy::CheckLaserHit(const glm::vec3& laserPos, float laserRadius, float damage,
                               ParticleSystem& particles, int& outScoreGained) {
    if (!active) return false;

    // Locomotive hit
    if (glm::distance(laserPos, locomotivePos) <= (3.2f + laserRadius)) {
        particles.SpawnExplosion(laserPos, 6, glm::vec3(1.0f, 0.8f, 0.2f));
        outScoreGained += 25;
        return true;
    }

    // Cargo cars hit
    for (auto& car : cars) {
        if (car.destroyed) continue;
        if (glm::distance(laserPos, car.position + glm::vec3(0, 1.2f, 0)) <= (2.8f + laserRadius)) {
            car.health -= damage;
            particles.SpawnExplosion(laserPos, 6, glm::vec3(1.0f, 0.7f, 0.2f));
            outScoreGained += 35;

            if (car.health <= 0.0f) {
                car.destroyed = true;
                particles.SpawnExplosion(car.position + glm::vec3(0, 1.5f, 0), 30, glm::vec3(1.0f, 0.5f, 0.1f));
                outScoreGained += 600;
            }
            return true;
        }
    }

    return false;
}
