#include "Engine.h"
#include "Input.h"
#include <iostream>
#include <fstream>
#include <cstring>
#include <cmath>
#include <algorithm>

Engine::Engine(int width, int height, const std::string& title)
    : window(nullptr),
      windowWidth(width),
      windowHeight(height),
      windowTitle(title),
      state(GameState::TitleHangar),
      hangarRotAngle(0.0f),
      selectedTitleMenu(0),
      selectedSettingsIndex(0),
      invertPitchY(false),
      defaultCockpitMode(false),
      warpTransitionTimer(0.0f),
      hardRouteWon(false),
      comboHits(0),
      comboTimer(0.0f),
      comboMaxDuration(2.6f),
      comboAnimScale(1.0f),
      bossSpawned(false),
      victoryTimer(0.0f),
      wasChargingAudio(false),
      wasBoostingAudio(false),
      wasBrakingAudio(false),
      hadLockOnLastFrame(false),
      wasSomersaultingAudio(false),
      victoryFanfarePlayed(false),
      bossDeathCamTriggered(false),
      lastFrameTime(0.0f) {}

Engine::~Engine() {
    // Explicitly delete OpenGL resources before destroying GLFW window context
    player.reset();
    projectiles.reset();
    ordnance.reset();
    particles.reset();
    reticle.reset();
    environment.reset();
    enemies.reset();
    boss.reset();
    hud.reset();
    audio.reset();
    wingmen.reset();
    postProcessor.reset();
    hangarFloorMesh.reset();
    turntableMesh.reset();
    shader.Delete();

    if (window) {
        glfwDestroyWindow(window);
        glfwTerminate();
    }
}

void Engine::FramebufferSizeCallback(GLFWwindow* window, int width, int height) {
    Engine* engine = static_cast<Engine*>(glfwGetWindowUserPointer(window));
    if (engine) {
        engine->windowWidth = width;
        engine->windowHeight = height;
        glViewport(0, 0, width, height);
        engine->camera.SetAspectRatio(static_cast<float>(width) / static_cast<float>(height));
        if (engine->postProcessor) {
            engine->postProcessor->Resize(width, height);
        }
    }
}

void Engine::LoadHighScores() {
    highScores.clear();
    std::ifstream file("scores.dat", std::ios::binary);
    if (file.is_open()) {
        HighScoreEntry entry;
        while (file.read(reinterpret_cast<char*>(&entry), sizeof(HighScoreEntry))) {
            highScores.push_back(entry);
        }
        file.close();
    }
    if (highScores.empty()) {
        highScores = {
            {"FOX", 45000, 2, true},
            {"FALCO", 38500, 2, true},
            {"SLIPPY", 26200, 1, false},
            {"PEPPY", 22800, 1, false},
            {"ROB-64", 18000, 1, false}
        };
        SaveHighScores();
    }
}

void Engine::SaveHighScores() {
    std::ofstream file("scores.dat", std::ios::binary);
    if (file.is_open()) {
        for (const auto& entry : highScores) {
            file.write(reinterpret_cast<const char*>(&entry), sizeof(HighScoreEntry));
        }
        file.close();
    }
}

void Engine::CheckNewHighScore(int score, int stage, bool complete) {
    HighScoreEntry newEntry;
    std::memset(&newEntry, 0, sizeof(HighScoreEntry));
    std::snprintf(newEntry.name, sizeof(newEntry.name), "ACE");
    newEntry.score = score;
    newEntry.stageReached = stage;
    newEntry.missionComplete = complete;

    highScores.push_back(newEntry);
    std::sort(highScores.begin(), highScores.end(), [](const HighScoreEntry& a, const HighScoreEntry& b) {
        return a.score > b.score;
    });
    if (highScores.size() > 5) {
        highScores.resize(5);
    }
    SaveHighScores();
}

bool Engine::Init() {
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW!" << std::endl;
        return false;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(windowWidth, windowHeight, windowTitle.c_str(), nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window!" << std::endl;
        glfwTerminate();
        return false;
    }

    glfwMakeContextCurrent(window);
    glfwSetWindowUserPointer(window, this);
    glfwSetFramebufferSizeCallback(window, FramebufferSizeCallback);

    if (!gladLoadGL()) {
        std::cerr << "Failed to initialize GLAD!" << std::endl;
        return false;
    }

    glViewport(0, 0, windowWidth, windowHeight);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    // Initialize subsystems
    Input::Init(window);
    camera.SetAspectRatio(static_cast<float>(windowWidth) / static_cast<float>(windowHeight));

    // Load shaders
    shader = Shader("resource/shaders/default.vert", "resource/shaders/default.frag");
    if (shader.ID == 0) {
        shader = Shader("../resource/shaders/default.vert", "../resource/shaders/default.frag");
    }

    player = std::make_unique<PlayerStarfighter>();
    projectiles = std::make_unique<ProjectileManager>();
    ordnance = std::make_unique<OrdnanceManager>();
    particles = std::make_unique<ParticleSystem>();
    reticle = std::make_unique<TargetingReticle>();
    environment = std::make_unique<WorldEnvironment>();
    enemies = std::make_unique<EnemyManager>();
    boss = std::make_unique<BossDreadnought>();
    hud = std::make_unique<HUD>();
    audio = std::make_unique<SoundManager>();
    audio->Init();
    wingmen = std::make_unique<WingmanSquadron>();
    postProcessor = std::make_unique<PostProcessor>();
    postProcessor->Init(windowWidth, windowHeight);

    hangarFloorMesh = std::make_unique<Mesh>(Mesh::CreateCube(glm::vec3(40.0f, 0.4f, 40.0f), glm::vec3(0.08f, 0.11f, 0.16f)));
    turntableMesh = std::make_unique<Mesh>(Mesh::CreateRing(7.2f, 8.2f, 24, glm::vec3(0.2f, 0.85f, 1.0f)));

    LoadHighScores();

    // Start in 3D Hangar with Title BGM
    state = GameState::TitleHangar;
    audio->PlayBGM(BGMTrack::Title, 0.65f);

    std::cout << "========================================================\n"
              << " 3D Rail-Shooter Engine initialized successfully!\n"
              << " Controls:\n"
              << "   - [Enter] / [Space]: Select / Fire Blasters\n"
              << "   - [A] / [D] or [Left] / [Right]: Move & Bank\n"
              << "   - [W] / [S] or [Up] / [Down]: Pitch & Altitude\n"
              << "   - [Shift]: Boost (Warp FOV)\n"
              << "   - [Ctrl]: Airbrake\n"
              << "   - Double-tap [A] / [D] or [Q] / [E]: Tactical Barrel Roll\n"
              << "   - [Space] or [J] (Hold & Release): Charged Lock-on Shot!\n"
              << "   - [B] or [K]: Launch / Detonate Screen-Clearing Smart Bomb!\n"
              << "   - [S + Shift]: Evasive Somersault Loop-de-loop!\n"
              << "   - [S + Ctrl]: Evasive U-Turn 180 Flip!\n"
              << "   - [V]: Toggle Cockpit First-Person / Chase Camera\n"
              << "   - [F1]: Toggle Retro Arcade CRT Scanlines\n"
              << "   - [S]: Flight Settings  |  [L]: Hall of Fame\n"
              << "========================================================" << std::endl;

    return true;
}

void Engine::StartMission() {
    state = GameState::Playing;
    environment->SetSector(SectorStage::Sector1_Canyon);
    player = std::make_unique<PlayerStarfighter>();
    player->invertPitch = invertPitchY;
    if (defaultCockpitMode) {
        camera.SetViewMode(CameraViewMode::CockpitFirstPerson);
    } else {
        camera.SetViewMode(CameraViewMode::ThirdPerson);
    }

    projectiles->Clear();
    ordnance->Clear();
    particles->Clear();
    enemies->Clear();
    if (wingmen) wingmen->Reset();
    if (boss) boss->Reset();
    bossSpawned = false;
    victoryTimer = 0.0f;
    wasChargingAudio = false;
    wasBoostingAudio = false;
    wasBrakingAudio = false;
    hadLockOnLastFrame = false;
    victoryFanfarePlayed = false;
    bossDeathCamTriggered = false;
    if (postProcessor) postProcessor->SetWarpIntensity(0.0f);
    warpTransitionTimer = 0.0f;
    hardRouteWon = false;
    comboHits = 0;
    comboTimer = 0.0f;
    comboAnimScale = 1.0f;

    if (audio) {
        audio->PlayBGM(BGMTrack::Stage1, 0.65f);
    }
}

void Engine::RegisterHitCombo(const glm::vec3&, int baseScore) {
    comboHits++;
    comboTimer = comboMaxDuration;
    comboAnimScale = 1.45f;

    int multiplier = std::clamp(comboHits, 1, 8);
    int earnedScore = baseScore * multiplier;
    player->score += earnedScore;

    if (audio) {
        float pitch = 1.0f + std::min(0.55f, (comboHits - 1) * 0.06f);
        audio->Play(SoundID::LockOnPing, 0.70f, pitch);
    }
}

void Engine::ReturnToTitle() {
    state = GameState::TitleHangar;
    if (audio) {
        audio->StopLoop(SoundID::BoostRoar);
        audio->StopLoop(SoundID::BrakeHiss);
        audio->StopLoop(SoundID::ChargeHum);
        audio->StopLoop(SoundID::WarningSiren);
        audio->PlayBGM(BGMTrack::Title, 0.65f);
    }
    camera.StopCinematic();
    camera.SetViewMode(CameraViewMode::ThirdPerson);
    if (postProcessor) postProcessor->SetWarpIntensity(0.0f);
}

void Engine::RestartGame() {
    StartMission();
}

void Engine::TriggerHyperspaceWarp() {
    state = GameState::StageClearWarp;
    warpTransitionTimer = 0.0f;
    hardRouteWon = true;
    player->invulnerableTimer = 10.0f;
    if (audio) {
        audio->StopBGM();
        audio->Play(SoundID::BoostRoar, 1.0f);
    }
    if (wingmen) {
        wingmen->TriggerTransmission(WingmanID::Aegis, "Route confirmed! Engaging hyperdrive!",
                                     "Next stop: Sector 2 Debris Field!", 4.0f, audio.get());
    }
}

void Engine::CompleteHyperspaceWarp() {
    environment->SetSector(SectorStage::Sector2_DeepSpace);
    player->transform.position = glm::vec3(0.0f, 0.0f, 0.0f);
    player->currentSpeed = player->baseSpeed;
    camera.position = player->transform.position + camera.followOffset;
    camera.target = player->transform.position;

    projectiles->Clear();
    ordnance->Clear();
    enemies->Clear();
    bossSpawned = false;
    if (boss) boss->Reset();

    if (wingmen) {
        wingmen->TriggerTransmission(WingmanID::Striker, "Hyperspace jump successful!",
                                     "Entering Sector 2 cosmic debris field!", 4.2f, audio.get());
    }
    if (audio) {
        audio->PlayBGM(BGMTrack::Sector2, 0.70f);
    }
    state = GameState::Playing;
}

void Engine::ProcessInput(float) {
    if (state == GameState::TitleHangar) {
        if (Input::IsKeyPressed(GLFW_KEY_UP) || Input::IsKeyPressed(GLFW_KEY_W)) {
            selectedTitleMenu = (selectedTitleMenu + 3) % 4;
            if (audio) audio->Play(SoundID::LockOnPing, 0.6f, 1.2f);
        }
        if (Input::IsKeyPressed(GLFW_KEY_DOWN) || Input::IsKeyPressed(GLFW_KEY_S)) {
            selectedTitleMenu = (selectedTitleMenu + 1) % 4;
            if (audio) audio->Play(SoundID::LockOnPing, 0.6f, 1.2f);
        }
        if (Input::IsKeyPressed(GLFW_KEY_ENTER) || Input::IsKeyPressed(GLFW_KEY_SPACE)) {
            if (selectedTitleMenu == 0) {
                state = GameState::MissionBriefing;
                if (audio) audio->Play(SoundID::RingCollect, 0.85f);
            } else if (selectedTitleMenu == 1) {
                state = GameState::SettingsMenu;
                if (audio) audio->Play(SoundID::RingCollect, 0.85f);
            } else if (selectedTitleMenu == 2) {
                state = GameState::Leaderboard;
                if (audio) audio->Play(SoundID::RingCollect, 0.85f);
            } else if (selectedTitleMenu == 3) {
                if (postProcessor) postProcessor->ToggleCRT();
                if (audio) audio->Play(SoundID::LockOnPing, 0.8f);
            }
        }
        if (Input::IsKeyPressed(GLFW_KEY_S)) {
            state = GameState::SettingsMenu;
        }
        if (Input::IsKeyPressed(GLFW_KEY_L)) {
            state = GameState::Leaderboard;
        }
        if (Input::IsKeyPressed(GLFW_KEY_F1)) {
            if (postProcessor) postProcessor->ToggleCRT();
        }
        return;
    }

    if (state == GameState::SettingsMenu) {
        if (Input::IsKeyPressed(GLFW_KEY_UP) || Input::IsKeyPressed(GLFW_KEY_W)) {
            selectedSettingsIndex = (selectedSettingsIndex + 6) % 7;
            if (audio) audio->Play(SoundID::LockOnPing, 0.6f, 1.2f);
        }
        if (Input::IsKeyPressed(GLFW_KEY_DOWN) || Input::IsKeyPressed(GLFW_KEY_S)) {
            selectedSettingsIndex = (selectedSettingsIndex + 1) % 7;
            if (audio) audio->Play(SoundID::LockOnPing, 0.6f, 1.2f);
        }
        if (Input::IsKeyPressed(GLFW_KEY_LEFT) || Input::IsKeyPressed(GLFW_KEY_A)) {
            if (selectedSettingsIndex == 0 && audio) {
                audio->SetMasterVolume(audio->GetMasterVolume() - 0.1f);
            } else if (selectedSettingsIndex == 1 && audio) {
                audio->SetMusicVolume(audio->GetMusicVolume() - 0.1f);
            } else if (selectedSettingsIndex == 2 && audio) {
                audio->SetSFXVolume(audio->GetSFXVolume() - 0.1f);
            } else if (selectedSettingsIndex == 3 && hud) {
                float scales[] = {0.85f, 1.0f, 1.25f, 1.50f};
                float cur = hud->GetScale();
                int idx = 1;
                for (int i = 0; i < 4; ++i) {
                    if (std::abs(cur - scales[i]) < 0.05f) { idx = i; break; }
                }
                idx = (idx + 3) % 4;
                hud->SetScale(scales[idx]);
            } else if (selectedSettingsIndex == 4) {
                invertPitchY = !invertPitchY;
                if (player) player->invertPitch = invertPitchY;
            } else if (selectedSettingsIndex == 5) {
                if (postProcessor) postProcessor->ToggleCRT();
            } else if (selectedSettingsIndex == 6) {
                defaultCockpitMode = !defaultCockpitMode;
            }
            if (audio) audio->Play(SoundID::LockOnPing, 0.6f, 1.4f);
        }
        if (Input::IsKeyPressed(GLFW_KEY_RIGHT) || Input::IsKeyPressed(GLFW_KEY_D)) {
            if (selectedSettingsIndex == 0 && audio) {
                audio->SetMasterVolume(audio->GetMasterVolume() + 0.1f);
            } else if (selectedSettingsIndex == 1 && audio) {
                audio->SetMusicVolume(audio->GetMusicVolume() + 0.1f);
            } else if (selectedSettingsIndex == 2 && audio) {
                audio->SetSFXVolume(audio->GetSFXVolume() + 0.1f);
            } else if (selectedSettingsIndex == 3 && hud) {
                float scales[] = {0.85f, 1.0f, 1.25f, 1.50f};
                float cur = hud->GetScale();
                int idx = 1;
                for (int i = 0; i < 4; ++i) {
                    if (std::abs(cur - scales[i]) < 0.05f) { idx = i; break; }
                }
                idx = (idx + 1) % 4;
                hud->SetScale(scales[idx]);
            } else if (selectedSettingsIndex == 4) {
                invertPitchY = !invertPitchY;
                if (player) player->invertPitch = invertPitchY;
            } else if (selectedSettingsIndex == 5) {
                if (postProcessor) postProcessor->ToggleCRT();
            } else if (selectedSettingsIndex == 6) {
                defaultCockpitMode = !defaultCockpitMode;
            }
            if (audio) audio->Play(SoundID::LockOnPing, 0.6f, 1.4f);
        }
        if (Input::IsKeyPressed(GLFW_KEY_ESCAPE) || Input::IsKeyPressed(GLFW_KEY_ENTER)) {
            state = GameState::TitleHangar;
        }
        return;
    }

    if (state == GameState::Leaderboard) {
        if (Input::IsKeyPressed(GLFW_KEY_ESCAPE) || Input::IsKeyPressed(GLFW_KEY_SPACE) || Input::IsKeyPressed(GLFW_KEY_ENTER)) {
            state = GameState::TitleHangar;
        }
        return;
    }

    if (state == GameState::MissionBriefing) {
        if (Input::IsKeyPressed(GLFW_KEY_ENTER) || Input::IsKeyPressed(GLFW_KEY_SPACE)) {
            StartMission();
        }
        if (Input::IsKeyPressed(GLFW_KEY_ESCAPE)) {
            state = GameState::TitleHangar;
        }
        return;
    }

    if (state == GameState::GameOver || state == GameState::Victory) {
        if (Input::IsKeyPressed(GLFW_KEY_R) || Input::IsKeyPressed(GLFW_KEY_SPACE) || Input::IsKeyPressed(GLFW_KEY_ENTER)) {
            if (player) {
                int stageNum = (environment && environment->currentSector == SectorStage::Sector2_DeepSpace) ? 2 : 1;
                CheckNewHighScore(player->score, stageNum, hardRouteWon);
            }
            ReturnToTitle();
        }
        return;
    }

    // Toggle Cockpit First-Person / Third-Person Chase Cam
    if (Input::IsKeyPressed(GLFW_KEY_V)) {
        camera.ToggleViewMode();
    }

    // Toggle Retro Arcade CRT Scanlines / Phosphor Filter
    if (Input::IsKeyPressed(GLFW_KEY_F1)) {
        if (postProcessor) {
            postProcessor->ToggleCRT();
        }
    }

    // Skip Boss Intro Cinematic or hold controls during sweep
    if (camera.IsInCinematic() && camera.cinematicMode == CinematicMode::BossIntro) {
        if (Input::IsKeyPressed(GLFW_KEY_SPACE) || Input::IsKeyPressed(GLFW_KEY_ENTER)) {
            camera.StopCinematic();
        } else {
            return;
        }
    }

    // Weapons Input:
    // 1. Dual Plasma Lasers (Single tap)
    bool isFireKey = Input::IsKeyPressed(GLFW_KEY_SPACE) || Input::IsKeyPressed(GLFW_KEY_J);
    if (isFireKey && player->CanFire()) {
        glm::vec3 aimTarget = player->GetFarTargetPos();
        if (player->leftWingLost) {
            projectiles->SpawnLaser(player->GetRightMuzzlePos(), aimTarget, true);
        } else if (player->rightWingLost) {
            projectiles->SpawnLaser(player->GetLeftMuzzlePos(), aimTarget, true);
        } else {
            projectiles->SpawnLaser(player->GetLeftMuzzlePos(), aimTarget, true);
            projectiles->SpawnLaser(player->GetRightMuzzlePos(), aimTarget, true);
        }

        player->ResetFireTimer();
        if (!player->leftWingLost) {
            particles->SpawnExplosion(player->GetLeftMuzzlePos(), 3, glm::vec3(0.2f, 1.0f, 0.4f));
        }
        if (!player->rightWingLost) {
            particles->SpawnExplosion(player->GetRightMuzzlePos(), 3, glm::vec3(0.2f, 1.0f, 0.4f));
        }
        if (audio) {
            audio->Play(SoundID::LaserFire, 0.85f);
        }
    }

    // 2. Charge Shot System (Holding Space or J builds charge)
    bool isHoldingFire = Input::IsKeyDown(GLFW_KEY_SPACE) || Input::IsKeyDown(GLFW_KEY_J);
    if (isHoldingFire) {
        player->StartCharging();
    }

    // 3. Release Charged Shot (Key released after charging)
    bool justReleasedFire = Input::IsKeyReleased(GLFW_KEY_SPACE) || Input::IsKeyReleased(GLFW_KEY_J);
    if (justReleasedFire) {
        if (player->ReleaseChargedShot()) {
            glm::vec3 nosePos = player->GetNosePos();
            glm::vec3 fwd = player->GetForwardVector();
            glm::vec3 targetPos = player->hasLockOn ? player->lockTargetPos : player->GetFarTargetPos();
            ordnance->SpawnChargedShot(nosePos, fwd, player->hasLockOn, targetPos);
            particles->SpawnExplosion(nosePos, 18, glm::vec3(0.2f, 1.0f, 0.8f));
            camera.TriggerShake(0.35f, 0.15f);
            if (audio) {
                audio->Play(SoundID::ChargedShotFire, 1.0f);
            }
        }
    }

    // 4. Smart Bomb (Launch or Detonate Early)
    if (Input::IsKeyPressed(GLFW_KEY_B) || Input::IsKeyPressed(GLFW_KEY_K)) {
        if (!ordnance->smartBombs.empty()) {
            ordnance->DetonateBomb(0);
            camera.TriggerShake(1.2f, 0.6f);
            if (audio) {
                audio->Play(SoundID::BombExplosion, 1.0f);
            }
        } else if (player->LaunchBomb()) {
            glm::vec3 nosePos = player->GetNosePos();
            ordnance->SpawnSmartBomb(nosePos, player->GetForwardVector());
            particles->SpawnExplosion(nosePos, 12, glm::vec3(1.0f, 0.85f, 0.2f));
            camera.TriggerShake(0.35f, 0.18f);
            if (audio) {
                audio->Play(SoundID::BombLaunch, 0.95f);
            }
        }
    }
}

void Engine::HandleCollisions() {
    if (state != GameState::Playing) return;

    auto CheckRelayHit = [&](const glm::vec3& hitPos, float radius, float damage) -> bool {
        for (auto& rel : environment->secretRelays) {
            if (rel.destroyed) continue;
            float dist = glm::distance(hitPos, rel.position);
            if (dist < (radius + rel.radius)) {
                rel.health -= damage;
                particles->SpawnExplosion(hitPos, 14, glm::vec3(1.0f, 0.85f, 0.2f));
                if (rel.health <= 0.0f) {
                    rel.destroyed = true;
                    player->score += 1500;
                    particles->SpawnExplosion(rel.position, 50, glm::vec3(1.0f, 0.45f, 0.1f));
                    camera.TriggerShake(0.85f, 0.45f);
                    if (audio) audio->Play(SoundID::ExplosionLarge, 1.0f);

                    int count = environment->GetDestroyedRelayCount();
                    if (wingmen) {
                        if (count == 1) {
                            wingmen->TriggerTransmission(WingmanID::Striker, "Target confirmed! Direct hit!",
                                                         "Planetary radar array #1 offline!", 3.8f, audio.get());
                        } else if (count == 2) {
                            wingmen->TriggerTransmission(WingmanID::Aegis, "Hostile telemetry jammed!",
                                                         "Only one relay remains!", 3.8f, audio.get());
                        } else if (count >= 3) {
                            wingmen->TriggerTransmission(WingmanID::Striker, "All relays neutralized!",
                                                         "Deep space vector unlocked, Fox!", 4.2f, audio.get());
                        }
                    }
                }
                return true;
            }
        }
        return false;
    };

    auto HandlePlayerFatalHit = [&]() {
        if (player->lives > 1) {
            player->lives--;
            player->shield = player->maxShield;
            player->invulnerableTimer = 2.5f;
            particles->SpawnExplosion(player->transform.position, 40, glm::vec3(1.0f, 0.6f, 0.2f));
            camera.TriggerShake(1.0f, 0.5f);
            if (audio) audio->Play(SoundID::ExplosionLarge, 1.0f);
        } else {
            player->lives = 0;
            particles->SpawnExplosion(player->transform.position, 60, glm::vec3(1.0f, 0.5f, 0.1f));
            camera.TriggerShake(1.4f, 0.8f);
            if (audio) audio->Play(SoundID::ExplosionLarge, 1.0f);
            state = GameState::GameOver;
        }
    };

    // 1. Player Regular Lasers vs Enemies, Boss, & Secret Relays
    for (auto& p : projectiles->projectiles) {
        if (!p.active || !p.isPlayer) continue;

        // Check vs Secret Relays
        if (CheckRelayHit(p.position, p.radius, 25.0f)) {
            p.active = false;
            continue;
        }

        // Check vs Boss
        if (boss && boss->IsActive()) {
            int scoreGained = 0;
            if (boss->CheckLaserHit(p.position, p.radius, 25.0f, *particles, camera, scoreGained)) {
                p.active = false;
                if (scoreGained > 0) {
                    RegisterHitCombo(p.position, scoreGained);
                }
                continue;
            }
        }

        for (auto& e : enemies->enemies) {
            if (!e.active) continue;

            float dist = glm::distance(p.position, e.transform.position);
            if (dist < (p.radius + e.radius)) {
                p.active = false;
                e.health -= 25.0f;
                particles->SpawnExplosion(p.position, 8, glm::vec3(1.0f, 0.8f, 0.2f));

                if (e.health <= 0.0f) {
                    e.active = false;
                    RegisterHitCombo(e.transform.position, e.scoreValue);
                    particles->SpawnExplosion(e.transform.position, 28, glm::vec3(1.0f, 0.4f, 0.1f));
                    camera.TriggerShake(0.45f, 0.25f);
                    if (audio) {
                        audio->Play(SoundID::ExplosionSmall, 0.85f, 0.95f + ((rand() % 10) * 0.01f));
                    }
                }
                break;
            }
        }
    }

    // 2. Charged Plasma Shots vs Enemies, Boss & Secret Relays
    for (auto& cs : ordnance->chargedShots) {
        if (!cs.active) continue;

        if (CheckRelayHit(cs.position, cs.radius, 110.0f)) {
            cs.active = false;
            ordnance->TriggerShockwave(cs.position, cs.aoeRadius, 110.0f);
            continue;
        }

        if (boss && boss->IsActive()) {
            int scoreGained = 0;
            if (boss->CheckLaserHit(cs.position, cs.radius, 110.0f, *particles, camera, scoreGained)) {
                cs.active = false;
                RegisterHitCombo(cs.position, scoreGained);
                ordnance->TriggerShockwave(cs.position, cs.aoeRadius, 110.0f);
                particles->SpawnExplosion(cs.position, 40, glm::vec3(0.2f, 1.0f, 0.8f));
                camera.TriggerShake(0.85f, 0.4f);
                if (audio) {
                    audio->Play(SoundID::BombExplosion, 0.85f);
                }
                continue;
            }
        }

        for (auto& e : enemies->enemies) {
            if (!e.active) continue;

            float dist = glm::distance(cs.position, e.transform.position);
            if (dist < (cs.radius + e.radius)) {
                cs.active = false;
                ordnance->TriggerShockwave(cs.position, cs.aoeRadius, 90.0f);
                particles->SpawnExplosion(cs.position, 40, glm::vec3(0.2f, 1.0f, 0.8f));
                camera.TriggerShake(0.75f, 0.35f);
                if (audio) {
                    audio->Play(SoundID::BombExplosion, 0.85f);
                }
                break;
            }
        }
    }

    // 3. Smart Bombs vs Enemies, Boss & Secret Relays
    for (size_t i = 0; i < ordnance->smartBombs.size(); ++i) {
        auto& b = ordnance->smartBombs[i];
        if (!b.active) continue;

        if (CheckRelayHit(b.position, b.radius, 160.0f)) {
            ordnance->DetonateBomb(i);
            camera.TriggerShake(1.3f, 0.6f);
            if (audio) audio->Play(SoundID::BombExplosion, 1.0f);
            break;
        }

        if (boss && boss->IsActive()) {
            int scoreGained = 0;
            if (boss->CheckLaserHit(b.position, b.radius, 160.0f, *particles, camera, scoreGained)) {
                ordnance->DetonateBomb(i);
                RegisterHitCombo(b.position, scoreGained);
                camera.TriggerShake(1.3f, 0.6f);
                if (audio) {
                    audio->Play(SoundID::BombExplosion, 1.0f);
                }
                break;
            }
        }

        for (auto& e : enemies->enemies) {
            if (!e.active) continue;

            float dist = glm::distance(b.position, e.transform.position);
            if (dist < (b.radius + e.radius)) {
                ordnance->DetonateBomb(i);
                camera.TriggerShake(1.2f, 0.6f);
                if (audio) {
                    audio->Play(SoundID::BombExplosion, 1.0f);
                }
                break;
            }
        }
    }

    // 4. Expanding Shockwaves vs Everything
    for (const auto& sw : ordnance->shockwaves) {
        if (!sw.active) continue;

        // Shockwave vs Relays
        for (auto& rel : environment->secretRelays) {
            if (rel.destroyed) continue;
            float dist = glm::distance(sw.position, rel.position);
            if (dist <= sw.currentRadius) {
                rel.health -= sw.damage * 0.8f;
                particles->SpawnExplosion(rel.position, 12, glm::vec3(1.0f, 0.8f, 0.2f));
                if (rel.health <= 0.0f) {
                    rel.destroyed = true;
                    RegisterHitCombo(rel.position, 1500);
                    particles->SpawnExplosion(rel.position, 48, glm::vec3(1.0f, 0.4f, 0.1f));
                    camera.TriggerShake(0.85f, 0.4f);
                    if (audio) audio->Play(SoundID::ExplosionLarge, 1.0f);
                }
            }
        }

        // Wipe enemy projectiles caught in shockwave
        for (auto& p : projectiles->projectiles) {
            if (!p.active || p.isPlayer) continue;

            float dist = glm::distance(sw.position, p.position);
            if (dist <= sw.currentRadius) {
                p.active = false;
                particles->SpawnExplosion(p.position, 6, glm::vec3(0.3f, 0.85f, 1.0f));
            }
        }

        // Damage Boss weakpoints
        if (boss && boss->IsActive()) {
            int scoreGained = 0;
            boss->ApplyShockwaveDamage(sw.position, sw.currentRadius, sw.damage * 0.7f, *particles, camera, scoreGained);
            if (scoreGained > 0) {
                RegisterHitCombo(sw.position, scoreGained);
            }
        }

        // Destroy regular enemies caught in shockwave
        for (auto& e : enemies->enemies) {
            if (!e.active) continue;

            float dist = glm::distance(sw.position, e.transform.position);
            if (dist <= sw.currentRadius) {
                e.health -= sw.damage;
                if (e.health <= 0.0f) {
                    e.active = false;
                    RegisterHitCombo(e.transform.position, e.scoreValue);
                    particles->SpawnExplosion(e.transform.position, 24, glm::vec3(1.0f, 0.6f, 0.2f));
                    if (audio) {
                        audio->Play(SoundID::ExplosionSmall, 0.85f);
                    }
                }
            }
        }
    }

    // 5. Hazard Monolith Pillars
    for (auto& pil : environment->pillars) {
        if (pil.destroyed) continue;

        // Blaster vs Pillar
        for (auto& p : projectiles->projectiles) {
            if (!p.active || !p.isPlayer) continue;
            float dx = p.position.x - pil.position.x;
            float dz = p.position.z - pil.position.z;
            float dist = std::sqrt(dx * dx + dz * dz);
            if (dist < (p.radius + pil.radius) && p.position.y > -7.0f && p.position.y < pil.height) {
                p.active = false;
                pil.health -= 25.0f;
                particles->SpawnExplosion(p.position, 8, glm::vec3(0.7f, 0.5f, 0.35f));
                if (pil.health <= 0.0f) {
                    pil.destroyed = true;
                    RegisterHitCombo(pil.position, 300);
                    particles->SpawnExplosion(pil.position + glm::vec3(0.0f, 4.0f, 0.0f), 35, glm::vec3(0.6f, 0.45f, 0.3f));
                    camera.TriggerShake(0.55f, 0.25f);
                    if (audio) audio->Play(SoundID::ExplosionLarge, 0.9f);
                }
                break;
            }
        }

        // Central fuselage collision
        float dx = player->transform.position.x - pil.position.x;
        float dz = player->transform.position.z - pil.position.z;
        float dist = std::sqrt(dx * dx + dz * dz);
        if (dist < (pil.radius + 1.2f) && player->transform.position.y < 7.0f) {
            pil.destroyed = true;
            if (player->IsDeflecting()) {
                particles->SpawnExplosion(pil.position + glm::vec3(0.0f, 3.0f, 0.0f), 30, glm::vec3(0.2f, 0.9f, 1.0f));
                camera.TriggerShake(0.35f, 0.2f);
                if (audio) audio->Play(SoundID::ExplosionLarge, 0.9f);
            } else {
                player->TakeDamage(30.0f);
                particles->SpawnExplosion(player->transform.position, 35, glm::vec3(0.7f, 0.45f, 0.3f));
                camera.TriggerShake(1.0f, 0.5f);
                if (audio) audio->Play(SoundID::ExplosionLarge, 1.0f, 0.65f);
                if (player->shield <= 0.0f) {
                    HandlePlayerFatalHit();
                }
            }
        }
    }

    // 6. Enemy Lasers vs Player
    for (auto& p : projectiles->projectiles) {
        if (!p.active || p.isPlayer) continue;

        float dist = glm::distance(p.position, player->transform.position);
        if (dist < (p.radius + 1.8f)) {
            p.active = false;
            if (player->IsDeflecting()) {
                particles->SpawnExplosion(p.position, 14, glm::vec3(0.2f, 0.9f, 1.0f));
                camera.TriggerShake(0.2f, 0.15f);
                if (audio) audio->Play(SoundID::LockOnPing, 0.85f, 1.6f);
            } else {
                player->TakeDamage(15.0f);
                particles->SpawnExplosion(p.position, 12, glm::vec3(1.0f, 0.2f, 0.2f));
                camera.TriggerShake(0.6f, 0.35f);
                if (audio) audio->Play(SoundID::ExplosionSmall, 0.95f, 0.65f);
                if (player->shield <= 0.0f) {
                    HandlePlayerFatalHit();
                }
            }
        }
    }

    // 7. Energy Rings
    for (auto& r : environment->rings) {
        if (r.collected) continue;

        float dist = glm::distance(player->transform.position, r.position);
        if (dist < r.radius * 1.15f) {
            r.collected = true;
            player->ringsCollected++;
            player->score += r.isGold ? 1000 : 500;
            player->AddShield(r.isGold ? 35.0f : 25.0f);

            if (!r.isGold) {
                bool neededRepair = (player->leftWingLost || player->rightWingLost ||
                                     player->leftWingHealth < 100.0f || player->rightWingHealth < 100.0f);
                player->RepairWings();
                if (neededRepair && audio) audio->Play(SoundID::WingRepair, 1.0f);
            }
            if (r.isGold && player->ringsCollected % 3 == 0) {
                player->AddBombs(1);
            }
            particles->SpawnExplosion(r.position, 22, r.isGold ? glm::vec3(1.0f, 0.9f, 0.3f) : glm::vec3(0.3f, 0.9f, 1.0f));
            if (audio) audio->Play(SoundID::RingCollect, 0.9f);
        }
    }

    // 8. Asteroids
    for (auto& a : environment->asteroids) {
        if (a.destroyed) continue;

        float dist = glm::distance(player->transform.position, a.position);
        if (dist < (a.radius + 1.6f)) {
            a.destroyed = true;
            particles->SpawnExplosion(a.position, 35, glm::vec3(0.7f, 0.6f, 0.5f));
            camera.TriggerShake(0.9f, 0.45f);
            if (audio) audio->Play(SoundID::ExplosionLarge, 0.85f);
            if (!player->IsDeflecting()) {
                player->TakeDamage(25.0f);
                if (player->shield <= 0.0f) {
                    HandlePlayerFatalHit();
                }
            }
        }
    }
}

void Engine::Update(float dt) {
    Input::Update(dt);
    ProcessInput(dt);

    if (state == GameState::TitleHangar || state == GameState::SettingsMenu ||
        state == GameState::Leaderboard || state == GameState::MissionBriefing) {
        hangarRotAngle += 35.0f * dt;
        if (hangarRotAngle > 360.0f) hangarRotAngle -= 360.0f;
        return;
    }

    if (state == GameState::StageClearWarp) {
        warpTransitionTimer += dt;
        float warpInt = std::clamp(warpTransitionTimer / 2.2f, 0.0f, 1.0f);
        if (postProcessor) {
            postProcessor->SetWarpIntensity(warpInt);
        }

        player->currentSpeed = 130.0f;
        player->transform.position.z -= player->currentSpeed * dt;
        camera.Follow(player->transform.position, 0.0f, 0.0f, 0.0f, dt);

        if (warpTransitionTimer >= 3.2f) {
            CompleteHyperspaceWarp();
        }
        return;
    }

    if (state == GameState::Playing) {
        bool inCinematic = camera.IsInCinematic() && camera.cinematicMode == CinematicMode::BossIntro;
        player->Update(dt, !inCinematic);
        if (inCinematic) {
            player->invulnerableTimer = 0.5f;
        }

        // Spawn Boss when threshold reached in Sector 1
        if (environment->currentSector == SectorStage::Sector1_Canyon) {
            if (!bossSpawned && player->transform.position.z <= -950.0f) {
                bossSpawned = true;
                boss->Spawn(player->transform.position.z);
                camera.StartBossIntro(boss->transform.position, player->transform.position);
                if (audio) {
                    audio->PlayBGM(BGMTrack::Boss, 0.70f);
                }
            }
        } else {
            // Sector 2: Deep Space finish line at Z <= -920
            if (player->transform.position.z <= -920.0f) {
                state = GameState::Victory;
                hardRouteWon = true;
                if (audio) {
                    audio->StopBGM();
                    audio->Play(SoundID::VictoryFanfare, 1.0f);
                }
            }
        }

        // Lock-on targeting during charge shot (prioritize boss weakpoints, then relays, then bogeys)
        if (player->isCharging) {
            particles->SpawnChargeInwardSparks(player->GetNosePos(), player->GetChargeProgress());
            glm::vec3 bossLockPos(0.0f);
            int bossTarget = (boss && boss->IsActive())
                             ? boss->FindLockTarget(player->transform.position, player->GetFarTargetPos(), bossLockPos)
                             : -1;

            if (bossTarget >= 0) {
                player->hasLockOn = true;
                player->lockTargetPos = bossLockPos;
            } else {
                bool relayLocked = false;
                for (const auto& rel : environment->secretRelays) {
                    if (rel.destroyed) continue;
                    float dist = glm::distance(player->transform.position, rel.position);
                    if (dist < 180.0f && rel.position.z < player->transform.position.z) {
                        player->hasLockOn = true;
                        player->lockTargetPos = rel.position;
                        relayLocked = true;
                        break;
                    }
                }
                if (!relayLocked) {
                    int lockIdx = enemies->FindLockTarget(player->transform.position, player->GetFarTargetPos());
                    if (lockIdx >= 0) {
                        player->hasLockOn = true;
                        player->lockTargetPos = enemies->enemies[lockIdx].transform.position;
                    } else {
                        player->hasLockOn = false;
                    }
                }
            }
        } else {
            player->hasLockOn = false;
        }

        if (player->hasLockOn && !hadLockOnLastFrame && audio) {
            audio->Play(SoundID::LockOnPing, 0.95f);
        }
        hadLockOnLastFrame = player->hasLockOn;

        static bool wasSpinning = false;
        if (player->isSpinning && !wasSpinning && audio) {
            audio->Play(SoundID::BarrelRoll, 0.95f);
        }
        wasSpinning = player->isSpinning;

        if (player->isBoosting) {
            camera.SetTargetFOV(70.0f);
            if (!wasBoostingAudio && audio) {
                audio->PlayLoop(SoundID::BoostRoar, 0.85f);
                wasBoostingAudio = true;
            }
        } else if (wasBoostingAudio && audio) {
            audio->StopLoop(SoundID::BoostRoar);
            wasBoostingAudio = false;
        }

        if (player->isBraking) {
            camera.SetTargetFOV(52.0f);
            if (!wasBrakingAudio && audio) {
                audio->PlayLoop(SoundID::BrakeHiss, 0.80f);
                wasBrakingAudio = true;
            }
        } else if (wasBrakingAudio && audio) {
            audio->StopLoop(SoundID::BrakeHiss);
            wasBrakingAudio = false;
        }

        if (!player->isBoosting && !player->isBraking) {
            camera.SetTargetFOV(60.0f);
        }

        if (player->isCharging) {
            if (!wasChargingAudio && audio) {
                audio->PlayLoop(SoundID::ChargeHum, 0.75f);
                wasChargingAudio = true;
            }
        } else if (wasChargingAudio && audio) {
            audio->StopLoop(SoundID::ChargeHum);
            wasChargingAudio = false;
        }

        if (player->isSomersaulting) {
            if (!wasSomersaultingAudio && audio) {
                audio->Play(SoundID::BarrelRoll, 1.0f);
                wasSomersaultingAudio = true;
            }
        } else {
            wasSomersaultingAudio = false;
        }

        // Keep camera steady during somersault (smooth follow, do not jerk camera into 360 loop pitch)
        float passPitch = player->isSomersaulting ? 0.0f : player->transform.rotation.x;
        camera.Follow(player->transform.position, passPitch, player->transform.rotation.y, player->transform.rotation.z,
                      dt, player->isAllRangeMode, player->isBoosting, player->isBraking);

        // Thruster sparks
        glm::vec3 shipVel = -player->GetForwardVector() * player->currentSpeed;
        glm::mat4 pModel = player->transform.GetModelMatrix();
        glm::vec3 leftExhaust = glm::vec3(pModel * glm::vec4(-0.28f, 0.0f, 1.45f, 1.0f));
        glm::vec3 rightExhaust = glm::vec3(pModel * glm::vec4(0.28f, 0.0f, 1.45f, 1.0f));
        particles->SpawnThrusterSparks(leftExhaust, shipVel);
        particles->SpawnThrusterSparks(rightExhaust, shipVel);

        // Low-altitude surface spray plume (Ex-Zodiac ground/water skimming FX)
        if (environment && environment->currentSector == SectorStage::Sector1_Canyon && !player->isAllRangeMode) {
            float floorY = -7.5f;
            float altitude = player->transform.position.y - floorY;
            if (altitude < 3.4f) {
                particles->SpawnSurfacePlume(player->transform.position, altitude, shipVel, player->transform.rotation.z);
            }
        }

        // Update Hit Combo chain countdown & scale bounce
        if (comboTimer > 0.0f) {
            comboTimer -= dt;
            if (comboAnimScale > 1.0f) {
                comboAnimScale = std::max(1.0f, comboAnimScale - dt * 2.8f);
            }
            if (comboTimer <= 0.0f) {
                if (comboHits >= 3) {
                    int chainBonus = comboHits * 150;
                    player->score += chainBonus;
                    if (audio) {
                        audio->Play(SoundID::RingCollect, 0.9f, 1.25f);
                    }
                }
                comboHits = 0;
                comboTimer = 0.0f;
            }
        }

        projectiles->Update(dt);
        ordnance->Update(dt);
        particles->Update(dt);
        environment->Update(player->transform.position.z, dt);

        if (boss && boss->IsActive()) {
            enemies->spawnTimer = 0.0f;
        }
        enemies->Update(player->transform.position.z, player->transform.position, *projectiles, dt, audio.get());

        if (wingmen) {
            int rescueBonus = 0;
            wingmen->Update(dt, player->transform.position, player->headingYaw,
                            player->currentSpeed, player->isAllRangeMode,
                            player->leftWingLost, player->rightWingLost,
                            *projectiles, *particles, *enemies, audio.get(), rescueBonus);
            player->score += rescueBonus;
        }

        if (boss) {
            boss->Update(dt, player->transform.position.z, player->transform.position,
                         *projectiles, *particles, camera, audio.get());

            if (boss->IsWarning()) {
                if (audio && !audio->IsLoopPlaying(SoundID::WarningSiren)) {
                    audio->PlayLoop(SoundID::WarningSiren, 0.85f);
                }
            } else if (audio && audio->IsLoopPlaying(SoundID::WarningSiren)) {
                audio->StopLoop(SoundID::WarningSiren);
            }

            if (boss->IsDefeated()) {
                if (!bossDeathCamTriggered) {
                    bossDeathCamTriggered = true;
                    camera.StartBossDeathSlowMo(boss->transform.position);
                    if (audio) {
                        audio->StopBGM();
                    }
                }
                victoryTimer += dt;
                if (victoryTimer >= 3.4f) {
                    bool allRelaysDestroyed = (environment->GetDestroyedRelayCount() >= 3);
                    bool wingmenSurviving = true;
                    if (wingmen) {
                        for (const auto& w : wingmen->wingmen) {
                            if (w.state == WingmanState::Retreating) wingmenSurviving = false;
                        }
                    }
                    bool secretRouteUnlocked = allRelaysDestroyed && wingmenSurviving;

                    if (secretRouteUnlocked && environment->currentSector == SectorStage::Sector1_Canyon) {
                        TriggerHyperspaceWarp();
                    } else {
                        state = GameState::Victory;
                        hardRouteWon = false;
                        if (!victoryFanfarePlayed && audio) {
                            audio->Play(SoundID::VictoryFanfare, 1.0f);
                            victoryFanfarePlayed = true;
                        }
                    }
                }
            }
        }

        HandleCollisions();
    }

    if (postProcessor && postProcessor->GetWarpIntensity() > 0.0f) {
        postProcessor->SetWarpIntensity(std::max(0.0f, postProcessor->GetWarpIntensity() - dt * 0.7f));
    }

    camera.Update(dt);
}

void Engine::Render() {
    float currentTime = static_cast<float>(glfwGetTime());

    // 1. RENDER 3D HANGAR BAY & MENUS
    if (state == GameState::TitleHangar || state == GameState::SettingsMenu ||
        state == GameState::Leaderboard || state == GameState::MissionBriefing) {
        if (postProcessor) {
            postProcessor->BeginRender();
        }

        glClearColor(0.03f, 0.05f, 0.09f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shader.Activate();

        camera.position = glm::vec3(0.0f, 3.2f, 4.0f);
        camera.target = glm::vec3(0.0f, 1.0f, -5.0f);
        camera.up = glm::vec3(0.0f, 1.0f, 0.0f);
        camera.fov = 55.0f;

        glm::mat4 view = camera.GetViewMatrix();
        glm::mat4 proj = camera.GetProjectionMatrix();
        shader.SetMat4("uView", view);
        shader.SetMat4("uProjection", proj);

        shader.SetVec3("uLightDir", glm::normalize(glm::vec3(0.4f, 1.0f, 0.5f)));
        shader.SetVec3("uLightColor", glm::vec3(0.9f, 0.95f, 1.0f));
        shader.SetVec3("uAmbientColor", glm::vec3(0.25f, 0.30f, 0.38f));
        shader.SetVec3("uFogColor", glm::vec3(0.03f, 0.05f, 0.09f));
        shader.SetFloat("uFogStart", 40.0f);
        shader.SetFloat("uFogEnd", 180.0f);
        shader.SetVec3("uCameraPos", camera.position);
        shader.SetInt("uUseLighting", 1);
        shader.SetInt("uUseFog", 1);
        shader.SetInt("uUseColorOverride", 0);
        shader.SetFloat("uAlpha", 1.0f);

        // Hangar Floor
        glm::mat4 floorModel = glm::mat4(1.0f);
        floorModel = glm::translate(floorModel, glm::vec3(0.0f, -0.2f, -5.0f));
        shader.SetMat4("uModel", floorModel);
        if (hangarFloorMesh) hangarFloorMesh->Draw(shader);

        // Turntable Pad
        glm::mat4 turnModel = glm::mat4(1.0f);
        turnModel = glm::translate(turnModel, glm::vec3(0.0f, 0.05f, -5.0f));
        shader.SetInt("uUseLighting", 0);
        shader.SetMat4("uModel", turnModel);
        if (turntableMesh) turntableMesh->Draw(shader);

        // Rotating Starfighter
        shader.SetInt("uUseLighting", 1);
        glm::mat4 shipModel = glm::mat4(1.0f);
        shipModel = glm::translate(shipModel, glm::vec3(0.0f, 1.2f, -5.0f));
        shipModel = glm::rotate(shipModel, glm::radians(hangarRotAngle), glm::vec3(0.0f, 1.0f, 0.0f));
        shipModel = glm::scale(shipModel, glm::vec3(1.3f));
        shader.SetMat4("uModel", shipModel);
        player->mesh.Draw(shader);

        // Render specific menu HUD overlay
        if (state == GameState::TitleHangar) {
            hud->DrawTitleScreen(shader, windowWidth, windowHeight, currentTime, selectedTitleMenu);
        } else if (state == GameState::SettingsMenu) {
            hud->DrawSettingsMenu(shader, windowWidth, windowHeight, selectedSettingsIndex,
                                  audio ? audio->GetMasterVolume() : 1.0f,
                                  audio ? audio->GetMusicVolume() : 0.7f,
                                  audio ? audio->GetSFXVolume() : 1.0f,
                                  hud ? hud->GetScale() : 1.0f,
                                  invertPitchY,
                                  postProcessor ? postProcessor->IsCRTEnabled() : false,
                                  defaultCockpitMode);
        } else if (state == GameState::Leaderboard) {
            hud->DrawLeaderboard(shader, windowWidth, windowHeight, highScores);
        } else if (state == GameState::MissionBriefing) {
            hud->DrawMissionBriefing(shader, windowWidth, windowHeight, currentTime);
        }

        if (postProcessor) {
            postProcessor->EndRender();
            postProcessor->Render(currentTime);
        }

        glfwSwapBuffers(window);
        return;
    }

    // 2. RENDER GAMEPLAY (PLAYING, WARP, VICTORY, GAME OVER)
    if (postProcessor) {
        postProcessor->BeginRender();
    }

    if (environment && environment->currentSector == SectorStage::Sector2_DeepSpace) {
        glClearColor(0.02f, 0.02f, 0.06f, 1.0f);
    } else {
        glClearColor(0.25f, 0.55f, 0.85f, 1.0f);
    }
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    shader.Activate();

    glm::mat4 view = camera.GetViewMatrix();
    glm::mat4 proj = camera.GetProjectionMatrix();
    shader.SetMat4("uView", view);
    shader.SetMat4("uProjection", proj);

    if (environment && environment->currentSector == SectorStage::Sector2_DeepSpace) {
        shader.SetVec3("uLightDir", glm::normalize(glm::vec3(0.3f, 0.9f, 0.4f)));
        shader.SetVec3("uLightColor", glm::vec3(0.9f, 0.95f, 1.1f));
        shader.SetVec3("uAmbientColor", glm::vec3(0.22f, 0.20f, 0.32f));
        shader.SetVec3("uFogColor", glm::vec3(0.02f, 0.02f, 0.06f));
        shader.SetFloat("uFogStart", 120.0f);
        shader.SetFloat("uFogEnd", 480.0f);
    } else {
        shader.SetVec3("uLightDir", glm::normalize(glm::vec3(0.45f, 0.88f, 0.35f)));
        shader.SetVec3("uLightColor", glm::vec3(1.0f, 0.98f, 0.90f));
        shader.SetVec3("uAmbientColor", glm::vec3(0.42f, 0.45f, 0.55f));
        shader.SetVec3("uFogColor", glm::vec3(0.35f, 0.68f, 0.92f));
        shader.SetFloat("uFogStart", 120.0f);
        shader.SetFloat("uFogEnd", 360.0f);
    }
    shader.SetVec3("uCameraPos", camera.position);
    shader.SetInt("uUseLighting", 1);
    shader.SetInt("uUseFog", 1);
    shader.SetInt("uUseColorOverride", 0);
    shader.SetFloat("uAlpha", 1.0f);

    environment->Draw(shader);
    enemies->Draw(shader);
    if (boss && environment->currentSector == SectorStage::Sector1_Canyon) {
        boss->Draw(shader);
    }
    projectiles->Draw(shader);
    ordnance->Draw(shader);
    particles->Draw(shader);

    if (state == GameState::Playing || state == GameState::Victory || state == GameState::StageClearWarp) {
        player->Draw(shader);
        if (wingmen) {
            wingmen->Draw(shader);
        }
        if (state == GameState::Playing) {
            reticle->Draw(shader, player->GetNearTargetPos(), player->GetFarTargetPos(),
                          player->hasLockOn, player->lockTargetPos, player->lockRotation);
        }
    }

    if (state == GameState::StageClearWarp) {
        hud->DrawWarpHUD(shader, windowWidth, windowHeight, warpTransitionTimer, true);
    } else {
        bool bActive = boss && boss->IsActive();
        bool bWarn = boss && boss->IsWarning();
        float bHealthRatio = boss ? boss->GetHealthRatio() : 0.0f;
        bool bLDown = boss ? boss->leftTurret.destroyed : false;
        bool bRDown = boss ? boss->rightTurret.destroyed : false;
        bool bSDown = boss ? boss->shieldGen.destroyed : false;
        bool bCoreExp = boss ? (boss->state == BossState::Phase2_ExposedCore) : false;

        std::vector<glm::vec3> enemyPositions;
        for (const auto& e : enemies->enemies) {
            if (e.active) {
                enemyPositions.push_back(e.transform.position);
            }
        }

        bool hasComms = wingmen ? wingmen->hasActiveMessage : false;
        int commsSpeaker = (wingmen && hasComms) ? static_cast<int>(wingmen->activeMessage.speaker) : 0;
        std::string commsCallsign = (wingmen && hasComms) ? wingmen->activeMessage.callsign : "";
        std::string commsLine1 = (wingmen && hasComms) ? wingmen->activeMessage.line1 : "";
        std::string commsLine2 = (wingmen && hasComms) ? wingmen->activeMessage.line2 : "";
        glm::vec3 commsColor = (wingmen && hasComms) ? wingmen->activeMessage.color : glm::vec3(1.0f);
        float commsTimer = (wingmen && hasComms) ? wingmen->activeMessage.timer : 0.0f;

        hud->Render(shader, windowWidth, windowHeight,
                    player->shield, player->maxShield,
                    player->boostMeter, player->maxBoost, player->isOverheated,
                    player->IsDeflecting(), player->score, player->ringsCollected,
                    player->bombCount, player->GetChargeProgress(),
                    bActive, bWarn, bHealthRatio,
                    bLDown, bRDown, bSDown, bCoreExp,
                    state == GameState::Victory, state == GameState::GameOver,
                    player->leftWingHealth, player->leftWingLost,
                    player->rightWingHealth, player->rightWingLost,
                    player->wingAlertTimer, player->wingAlertMessage,
                    camera.IsFirstPerson(), player->currentPitch, player->currentBank,
                    player->transform.position, player->headingYaw,
                    boss ? boss->transform.position : glm::vec3(0.0f),
                    enemyPositions,
                    hasComms, commsSpeaker, commsCallsign, commsLine1, commsLine2, commsColor, commsTimer,
                    environment ? environment->GetDestroyedRelayCount() : 0, 3,
                    comboHits, comboTimer, comboMaxDuration, comboAnimScale,
                    player->lives);
    }

    if (postProcessor) {
        postProcessor->EndRender();
        postProcessor->Render(currentTime);
    }

    glfwSwapBuffers(window);
}

void Engine::Run() {
    lastFrameTime = static_cast<float>(glfwGetTime());

    while (!glfwWindowShouldClose(window)) {
        float currentTime = static_cast<float>(glfwGetTime());
        float dt = currentTime - lastFrameTime;
        lastFrameTime = currentTime;

        if (dt > 0.08f) dt = 0.08f;

        glfwPollEvents();
        Update(dt);
        Render();
        Input::EndFrame();
    }
}
