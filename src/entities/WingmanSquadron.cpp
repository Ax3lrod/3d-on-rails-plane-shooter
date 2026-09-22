#include "WingmanSquadron.h"
#include <cmath>
#include <algorithm>
#include <iostream>

static const float PI = 3.14159265358979323846f;

Wingman::Wingman(WingmanID id, const std::string& callsign, const std::string& pilot,
                 const glm::vec3& color, float pitch, const glm::vec3& offset)
    : id(id),
      callsign(callsign),
      pilotName(pilot),
      themeColor(color),
      voicePitch(pitch),
      transform(offset),
      mesh(Mesh::CreateStarfighter(
          glm::vec3(0.85f, 0.87f, 0.90f),
          color,
          (id == WingmanID::Striker) ? glm::vec3(1.0f, 0.85f, 0.2f) : glm::vec3(0.2f, 0.95f, 0.95f)
      )),
      shield(100.0f),
      maxShield(100.0f),
      state(WingmanState::InFormation),
      formationOffset(offset),
      fireTimer(0.0f),
      fireRate(0.48f),
      distressTimer(0.0f),
      tailingEnemyIndex(-1),
      bobOffset((id == WingmanID::Striker) ? 0.0f : 2.1f),
      evasiveTimer(0.0f),
      currentBank(0.0f),
      currentPitch(0.0f),
      currentYaw(0.0f) {}

WingmanSquadron::WingmanSquadron()
    : hasActiveMessage(false),
      introTriggered(false),
      wave1Triggered(false),
      asteroidTriggered(false),
      bossTriggered(false),
      bossTurretTriggered(false),
      bossCoreTriggered(false),
      victoryTriggered(false),
      wingDamageAlertTriggered(false),
      nextDistressCheckTime(14.0f) {
    Reset();
}

void WingmanSquadron::Reset() {
    wingmen.clear();
    messageQueue.clear();
    hasActiveMessage = false;

    // Echo-1: Striker (Echelon Left, aggressive scout)
    wingmen.emplace_back(WingmanID::Striker, "ECHO-1 // STRIKER", "Striker",
                         glm::vec3(1.0f, 0.52f, 0.12f), 0.86f,
                         glm::vec3(-7.2f, 1.2f, 3.2f));

    // Echo-2: Aegis (Echelon Right, defensive escort)
    wingmen.emplace_back(WingmanID::Aegis, "ECHO-2 // AEGIS", "Aegis",
                         glm::vec3(0.15f, 0.88f, 0.85f), 1.28f,
                         glm::vec3(7.2f, 1.2f, 3.2f));

    introTriggered = false;
    wave1Triggered = false;
    asteroidTriggered = false;
    bossTriggered = false;
    bossTurretTriggered = false;
    bossCoreTriggered = false;
    victoryTriggered = false;
    wingDamageAlertTriggered = false;
    nextDistressCheckTime = 14.0f;
    pendingSupplyDrop = false;
    supplyDropPosition = glm::vec3(0.0f);
}

void WingmanSquadron::TriggerTransmission(WingmanID speaker, const std::string& line1,
                                         const std::string& line2, float duration,
                                         SoundManager* audio) {
    CommsMessage msg;
    msg.speaker = speaker;
    msg.callsign = (speaker == WingmanID::Striker) ? "ECHO-1 // STRIKER" : "ECHO-2 // AEGIS";
    msg.line1 = line1;
    msg.line2 = line2;
    msg.color = (speaker == WingmanID::Striker) ? glm::vec3(1.0f, 0.55f, 0.15f) : glm::vec3(0.2f, 0.9f, 0.85f);
    msg.voicePitch = (speaker == WingmanID::Striker) ? 0.86f : 1.28f;
    msg.duration = duration;
    msg.timer = duration;
    msg.chatterTimer = 0.05f;

    if (!hasActiveMessage) {
        activeMessage = msg;
        hasActiveMessage = true;
        if (audio) {
            audio->Play(SoundID::RadioChatter, 0.35f, msg.voicePitch);
        }
    } else {
        messageQueue.push_back(msg);
    }
}

void WingmanSquadron::TriggerScriptedRescue(WingmanID target, int threatCount, float timeout,
                                           const std::string& line1, const std::string& line2,
                                           EnemyManager& enemies, SoundManager* audio) {
    for (auto& wm : wingmen) {
        if (wm.id == target) {
            wm.state = WingmanState::Distressed;
            wm.distressTimer = timeout;
            wm.evasiveTimer = 0.0f;
            wm.pursuerIndices.clear();

            // Spawn the threat interceptors behind the distressed wingman
            size_t startIdx = enemies.enemies.size();
            float spawnZ = wm.transform.position.z + 24.0f;
            enemies.SpawnCustomWave(EnemyType::EliteInterceptor, "v_formation", threatCount,
                                   wm.transform.position.x, wm.transform.position.y + 1.0f, spawnZ,
                                   7.0f, 12.0f);
            for (size_t i = startIdx; i < enemies.enemies.size(); ++i) {
                wm.pursuerIndices.push_back(static_cast<int>(i));
            }
            wm.tailingEnemyIndex = (!wm.pursuerIndices.empty()) ? wm.pursuerIndices[0] : -1;

            TriggerTransmission(target, line1, line2, 4.2f, audio);
            nextDistressCheckTime = 9999.0f; // Don't trigger random distress during scripted rescue
            break;
        }
    }
}

void WingmanSquadron::Update(float dt, const glm::vec3& playerPos, float playerHeadingYaw,
                            float playerSpeed, bool isAllRange, bool playerLeftWingLost,
                            bool playerRightWingLost, ProjectileManager& projectiles,
                            ParticleSystem& particles, EnemyManager& enemies,
                            SoundManager* audio, int& outScoreGained) {
    // 1. Update Radio Transmission Queue (plays gentle chime once on pop)
    if (hasActiveMessage) {
        activeMessage.timer -= dt;

        if (activeMessage.timer <= 0.0f) {
            hasActiveMessage = false;
            if (!messageQueue.empty()) {
                activeMessage = messageQueue.front();
                messageQueue.pop_front();
                hasActiveMessage = true;
                if (audio) {
                    audio->Play(SoundID::RadioChatter, 0.35f, activeMessage.voicePitch);
                }
            }
        }
    } else if (!messageQueue.empty()) {
        activeMessage = messageQueue.front();
        messageQueue.pop_front();
        hasActiveMessage = true;
        if (audio) {
            audio->Play(SoundID::RadioChatter, 0.35f, activeMessage.voicePitch);
        }
    }

    // 2. Story / Progress Radio Checkpoints
    if (!introTriggered && playerPos.z <= -25.0f) {
        introTriggered = true;
        TriggerTransmission(WingmanID::Striker, "Phew... I'm glad to be out of there...",
                            "Entering coastline sector. All systems green!", 3.6f, audio);
        TriggerTransmission(WingmanID::Aegis, "Echo-2 standing by. Watch the wind turbines.",
                            "Stay alert for enemy ambush formations, Lead.", 3.4f, audio);
    }

    if (!wave1Triggered && playerPos.z <= -240.0f) {
        wave1Triggered = true;
        TriggerTransmission(WingmanID::Striker, "Hostiles inbound! Multiple drone bogeys!",
                            "Engaging with supporting fire!", 3.2f, audio);
    }

    if (!asteroidTriggered && playerPos.z <= -500.0f) {
        asteroidTriggered = true;
        TriggerTransmission(WingmanID::Aegis, "Dense asteroid cluster ahead!",
                            "Tighten formation! Don't clip the cliff face!", 3.4f, audio);
    }

    if (!bossTriggered && playerPos.z <= -930.0f) {
        bossTriggered = true;
        TriggerTransmission(WingmanID::Striker, "Warning! Colossal dreadnought dead ahead!",
                            "Watch out! It's armed with spinning spiked maces!", 3.6f, audio);
        TriggerTransmission(WingmanID::Aegis, "Target the wing turret batteries first!",
                            "Concentrate laser fire on the exposed cooling vents!", 3.6f, audio);
    }

    // Player Wing Severed Radio Reaction
    if (!wingDamageAlertTriggered && (playerLeftWingLost || playerRightWingLost)) {
        wingDamageAlertTriggered = true;
        TriggerTransmission(WingmanID::Aegis, "Commander, your wing is sheared off!",
                            "Fly through a silver ring for emergency field repairs!", 3.8f, audio);
    }

    // 3. Periodic Distress Event Trigger
    nextDistressCheckTime -= dt;
    if (nextDistressCheckTime <= 0.0f && !isAllRange) {
        // Pick an intact wingman to get tailed if enemies exist ahead
        for (auto& wm : wingmen) {
            if (wm.state == WingmanState::InFormation && wm.shield > 25.0f) {
                int targetEnemy = -1;
                for (size_t i = 0; i < enemies.enemies.size(); ++i) {
                    if (enemies.enemies[i].active &&
                        enemies.enemies[i].transform.position.z < (playerPos.z - 30.0f) &&
                        enemies.enemies[i].transform.position.z > (playerPos.z - 160.0f)) {
                        targetEnemy = static_cast<int>(i);
                        break;
                    }
                }

                if (targetEnemy >= 0) {
                    wm.state = WingmanState::Distressed;
                    wm.tailingEnemyIndex = targetEnemy;
                    wm.distressTimer = 11.0f;
                    wm.evasiveTimer = 0.0f;

                    if (wm.id == WingmanID::Striker) {
                        TriggerTransmission(WingmanID::Striker, "Bogey on my tail! Can't shake him!",
                                            "Get him off my six, Commander!", 3.8f, audio);
                    } else {
                        TriggerTransmission(WingmanID::Aegis, "Hostile pursuit! Shields taking fire!",
                                            "Commander, request immediate assistance!", 3.8f, audio);
                    }
                    nextDistressCheckTime = 26.0f;
                    break;
                }
            }
        }
        if (nextDistressCheckTime <= 0.0f) {
            nextDistressCheckTime = 12.0f;
        }
    }

    // 4. Update Each Wingman's AI Flight & Combat
    for (auto& wm : wingmen) {
        // Calculate desired formation anchor position
        glm::vec3 targetPos;
        if (isAllRange) {
            // In 360 All-Range mode, rotate formation offset around player's heading
            glm::mat4 rot = glm::rotate(glm::mat4(1.0f), glm::radians(playerHeadingYaw), glm::vec3(0.0f, 1.0f, 0.0f));
            glm::vec3 rotatedOffset = glm::vec3(rot * glm::vec4(wm.formationOffset, 1.0f));
            targetPos = playerPos + rotatedOffset;
        } else {
            // Rail corridor V-formation flight with subtle hovering bob
            targetPos = playerPos + wm.formationOffset;
            targetPos.y += std::sin(playerPos.z * 0.04f + wm.bobOffset) * 0.65f;
        }

        // State Machine
        switch (wm.state) {
            case WingmanState::InFormation: {
                // Smoothly fly towards formation position
                float t = 1.0f - std::exp(-5.5f * dt);
                wm.transform.position = glm::mix(wm.transform.position, targetPos, t);

                // Match player heading with slight banking
                wm.currentYaw = glm::mix(wm.currentYaw, playerHeadingYaw, 1.0f - std::exp(-6.0f * dt));
                wm.currentPitch = glm::mix(wm.currentPitch, 0.0f, 1.0f - std::exp(-8.0f * dt));
                wm.currentBank = glm::mix(wm.currentBank, (wm.id == WingmanID::Striker ? -6.0f : 6.0f), 1.0f - std::exp(-6.0f * dt));

                // Supporting Fire: Scan for nearest enemy drone ahead to shoot
                wm.fireTimer -= dt;
                if (wm.fireTimer <= 0.0f) {
                    for (auto& e : enemies.enemies) {
                        if (!e.active) continue;

                        float dz = e.transform.position.z - wm.transform.position.z;
                        if (dz < -15.0f && dz > -110.0f && std::abs(e.transform.position.x - wm.transform.position.x) < 22.0f) {
                            // Fire supporting plasma laser!
                            glm::vec3 muzzle = wm.transform.position + glm::vec3(0.0f, -0.1f, -2.2f);
                            projectiles.SpawnLaser(muzzle, e.transform.position, true, 190.0f);
                            particles.SpawnExplosion(muzzle, 2, wm.themeColor);
                            wm.fireTimer = wm.fireRate + ((rand() % 10) * 0.02f);
                            if (audio) {
                                audio->Play(SoundID::LaserFire, 0.45f, wm.voicePitch * 1.15f);
                            }
                            break;
                        }
                    }
                }
                break;
            }

            case WingmanState::Distressed: {
                wm.distressTimer -= dt;
                wm.evasiveTimer += dt;

                // Evasive zig-zag flight pattern
                float weaveX = std::sin(wm.evasiveTimer * 6.0f) * 7.5f;
                float weaveY = std::cos(wm.evasiveTimer * 4.0f) * 3.5f;
                glm::vec3 evasiveTarget = targetPos + glm::vec3(weaveX, weaveY, -4.0f);

                wm.transform.position = glm::mix(wm.transform.position, evasiveTarget, 1.0f - std::exp(-8.0f * dt));
                wm.currentBank = std::sin(wm.evasiveTimer * 6.0f) * 45.0f;
                wm.currentPitch = std::cos(wm.evasiveTimer * 4.0f) * 15.0f;

                // Check if pursuing enemies are destroyed -> RESCUE!
                bool pursuerAlive = false;
                if (!wm.pursuerIndices.empty()) {
                    for (int idx : wm.pursuerIndices) {
                        if (idx >= 0 && idx < static_cast<int>(enemies.enemies.size())) {
                            if (enemies.enemies[idx].active && enemies.enemies[idx].health > 0.0f) {
                                pursuerAlive = true;
                                break;
                            }
                        }
                    }
                } else if (wm.tailingEnemyIndex >= 0 && wm.tailingEnemyIndex < static_cast<int>(enemies.enemies.size())) {
                    const auto& pursuer = enemies.enemies[wm.tailingEnemyIndex];
                    if (pursuer.active && pursuer.health > 0.0f) {
                        pursuerAlive = true;
                    }
                }

                if (!pursuerAlive) {
                    // RESCUE EVENT!
                    wm.state = WingmanState::Rescued;
                    outScoreGained += 5000; // Big Corneria-style rescue score bonus!
                    wm.shield = wm.maxShield;
                    pendingSupplyDrop = true;
                    supplyDropPosition = wm.transform.position + glm::vec3(0.0f, -0.5f, -25.0f);

                    if (wm.id == WingmanID::Striker) {
                        TriggerTransmission(WingmanID::Striker, "Bogey squad splashed! Outstanding shooting, Lead!",
                                            "Airdropping field supplies right in your lane! Grab the cargo!", 4.2f, audio);
                    } else {
                        TriggerTransmission(WingmanID::Aegis, "Hostiles neutralized! Outstanding shot, Commander!",
                                            "Releasing emergency supply capsule right on your trajectory!", 4.2f, audio);
                    }
                    if (audio) {
                        audio->Play(SoundID::RingCollect, 1.0f, 1.4f);
                    }
                } else if (wm.distressTimer <= 0.0f) {
                    // Timer expired without rescue - wingman takes heavy damage
                    wm.shield -= 50.0f;
                    particles.SpawnExplosion(wm.transform.position, 22, glm::vec3(1.0f, 0.4f, 0.2f));

                    if (wm.shield <= 20.0f) {
                        wm.state = WingmanState::Retreating;
                        if (wm.id == WingmanID::Striker) {
                            TriggerTransmission(WingmanID::Striker, "Shields failing! Systems critical!",
                                                "Disengaging for emergency field repair!", 3.8f, audio);
                        } else {
                            TriggerTransmission(WingmanID::Aegis, "Hull compromised! Withdrawing to high orbit!",
                                                "Cover your six, Commander!", 3.8f, audio);
                        }
                    } else {
                        wm.state = WingmanState::InFormation;
                    }
                }
                break;
            }

            case WingmanState::Rescued: {
                // Brief victory roll and rejoin formation
                wm.evasiveTimer += dt;
                float t = 1.0f - std::exp(-6.0f * dt);
                wm.transform.position = glm::mix(wm.transform.position, targetPos, t);
                wm.currentBank += 720.0f * dt;

                if (wm.evasiveTimer > 1.2f) {
                    wm.state = WingmanState::InFormation;
                    wm.currentBank = 0.0f;
                }
                break;
            }

            case WingmanState::Retreating: {
                // Ascend steeply into the sky/clouds
                wm.transform.position.y += 24.0f * dt;
                wm.transform.position.z -= playerSpeed * 0.7f * dt;
                wm.currentPitch = glm::mix(wm.currentPitch, 35.0f, 1.0f - std::exp(-5.0f * dt));
                break;
            }

            default:
                break;
        }

        // Apply rotation to transform
        wm.transform.rotation.x = wm.currentPitch;
        wm.transform.rotation.y = wm.currentYaw;
        wm.transform.rotation.z = wm.currentBank;

        // Thruster sparks from wingman twin engines
        if (wm.transform.position.y < 50.0f) {
            glm::mat4 m = wm.transform.GetModelMatrix();
            glm::vec3 leftExh = glm::vec3(m * glm::vec4(-0.25f, 0.0f, 1.35f, 1.0f));
            glm::vec3 rightExh = glm::vec3(m * glm::vec4(0.25f, 0.0f, 1.35f, 1.0f));
            glm::vec3 shipVel(0.0f, 0.0f, playerSpeed * 0.9f);
            particles.SpawnThrusterSparks(leftExh, shipVel);
            particles.SpawnThrusterSparks(rightExh, shipVel);
        }
    }
}

void WingmanSquadron::Draw(const Shader& shader) const {
    for (const auto& wm : wingmen) {
        if (wm.transform.position.y > 60.0f) continue; // Don't draw if retreated high into clouds

        glm::mat4 model = wm.transform.GetModelMatrix();
        shader.SetMat4("uModel", model);
        shader.SetInt("uUseLighting", 1);
        shader.SetFloat("uAlpha", 1.0f);
        wm.mesh.Draw(shader);
    }
}
