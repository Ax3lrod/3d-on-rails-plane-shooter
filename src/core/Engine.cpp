#include "Engine.h"
#include "Input.h"
#include <iostream>
#include <cmath>
#include <algorithm>

Engine::Engine(int width, int height, const std::string& title)
    : window(nullptr),
      windowWidth(width),
      windowHeight(height),
      windowTitle(title),
      state(GameState::Playing),
      bossSpawned(false),
      victoryTimer(0.0f),
      wasChargingAudio(false),
      wasBoostingAudio(false),
      wasBrakingAudio(false),
      hadLockOnLastFrame(false),
      victoryFanfarePlayed(false),
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
    shader.Delete();

    if (window) {
        glfwDestroyWindow(window);
        glfwTerminate();
    }
}

void Engine::FramebufferSizeCallback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
    Engine* engine = static_cast<Engine*>(glfwGetWindowUserPointer(window));
    if (engine && height > 0) {
        engine->windowWidth = width;
        engine->windowHeight = height;
        engine->camera.SetAspectRatio(static_cast<float>(width) / static_cast<float>(height));
    }
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

    // Initialize subsystems now that OpenGL context exists
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

    std::cout << "========================================================\n"
              << " 3D Rail-Shooter Engine initialized successfully!\n"
              << " Controls:\n"
              << "   - [A] / [D] or [Left] / [Right]: Move & Bank\n"
              << "   - [W] / [S] or [Up] / [Down]: Pitch & Altitude\n"
              << "   - [Shift]: Boost (Warp FOV)\n"
              << "   - [Ctrl]: Airbrake\n"
              << "   - Double-tap [A] / [D] or [Q] / [E]: Tactical Barrel Roll\n"
              << "   - [Space] or [J] (Tap): Fire Dual Plasma Lasers\n"
              << "   - [Space] or [J] (Hold & Release): Charged Lock-on Plasma Shot!\n"
              << "   - [B] or [K]: Launch / Detonate Screen-Clearing Smart Bomb!\n"
              << "   - [R]: Restart\n"
              << "========================================================" << std::endl;

    return true;
}

void Engine::RestartGame() {
    player = std::make_unique<PlayerStarfighter>();
    projectiles->Clear();
    ordnance->Clear();
    particles->Clear();
    environment->Clear();
    enemies->Clear();
    if (boss) {
        boss->Reset();
    }
    if (audio) {
        audio->StopLoop(SoundID::BoostRoar);
        audio->StopLoop(SoundID::BrakeHiss);
        audio->StopLoop(SoundID::ChargeHum);
        audio->StopLoop(SoundID::WarningSiren);
    }
    bossSpawned = false;
    victoryTimer = 0.0f;
    wasChargingAudio = false;
    wasBoostingAudio = false;
    wasBrakingAudio = false;
    hadLockOnLastFrame = false;
    victoryFanfarePlayed = false;
    state = GameState::Playing;
}

void Engine::ProcessInput(float) {
    if (state == GameState::GameOver || state == GameState::Victory) {
        if (Input::IsKeyPressed(GLFW_KEY_R) || Input::IsKeyPressed(GLFW_KEY_SPACE)) {
            RestartGame();
        }
        return;
    }

    // 1. Fire regular lasers on initial press
    if (Input::IsKeyPressed(GLFW_KEY_SPACE) || Input::IsKeyPressed(GLFW_KEY_J) ||
        Input::IsKeyPressed(GLFW_KEY_ENTER)) {
        if (player->CanFire()) {
            player->ResetFireTimer();

            glm::vec3 target = player->GetFarTargetPos();
            bool firedAny = false;

            if (player->HasLeftWing()) {
                glm::vec3 leftMuzzle = player->GetLeftMuzzlePos();
                projectiles->SpawnLaser(leftMuzzle, target, true, 220.0f);
                particles->SpawnExplosion(leftMuzzle, 3, glm::vec3(0.2f, 1.0f, 0.5f));
                firedAny = true;
            }

            if (player->HasRightWing()) {
                glm::vec3 rightMuzzle = player->GetRightMuzzlePos();
                projectiles->SpawnLaser(rightMuzzle, target, true, 220.0f);
                particles->SpawnExplosion(rightMuzzle, 3, glm::vec3(0.2f, 1.0f, 0.5f));
                firedAny = true;
            }

            // Emergency backup blaster if both wings are lost
            if (!firedAny) {
                glm::vec3 noseMuzzle = player->GetNosePos();
                projectiles->SpawnLaser(noseMuzzle, target, true, 220.0f);
                particles->SpawnExplosion(noseMuzzle, 4, glm::vec3(1.0f, 0.65f, 0.25f));
            }

            if (audio) {
                float pitch = firedAny ? (0.95f + ((rand() % 10) * 0.01f)) : 0.75f;
                audio->Play(SoundID::LaserFire, 0.85f, pitch);
            }
        }
    }

    // 2. Charge Plasma Shot while holding key
    if (Input::IsKeyDown(GLFW_KEY_SPACE) || Input::IsKeyDown(GLFW_KEY_J)) {
        player->StartCharging();
    }

    // 3. Release Charged Shot when key released
    if (Input::IsKeyReleased(GLFW_KEY_SPACE) || Input::IsKeyReleased(GLFW_KEY_J)) {
        if (player->ReleaseChargedShot()) {
            glm::vec3 nosePos = player->GetNosePos();
            ordnance->SpawnChargedShot(nosePos, glm::vec3(0.0f, 0.0f, -1.0f),
                                       player->hasLockOn, player->lockTargetPos);

            particles->SpawnExplosion(nosePos, 18, glm::vec3(0.2f, 1.0f, 0.8f));
            camera.TriggerShake(0.45f, 0.22f);

            if (audio) {
                audio->Play(SoundID::ChargedShotFire, 1.0f);
                audio->StopLoop(SoundID::ChargeHum);
                wasChargingAudio = false;
            }
        }
    }

    // 4. Smart Bomb (Launch or Detonate Early)
    if (Input::IsKeyPressed(GLFW_KEY_B) || Input::IsKeyPressed(GLFW_KEY_K)) {
        if (!ordnance->smartBombs.empty()) {
            // Early manual detonation of in-flight bomb
            ordnance->DetonateBomb(0);
            camera.TriggerShake(1.2f, 0.6f);
            if (audio) {
                audio->Play(SoundID::BombExplosion, 1.0f);
            }
        } else if (player->LaunchBomb()) {
            glm::vec3 nosePos = player->GetNosePos();
            ordnance->SpawnSmartBomb(nosePos, glm::vec3(0.0f, 0.0f, -1.0f));
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

    // 1. Player Regular Lasers vs Enemies & Boss
    for (auto& p : projectiles->projectiles) {
        if (!p.active || !p.isPlayer) continue;

        // Check vs Boss
        if (boss && boss->IsActive()) {
            int scoreGained = 0;
            if (boss->CheckLaserHit(p.position, p.radius, 25.0f, *particles, camera, scoreGained)) {
                p.active = false;
                player->score += scoreGained;
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
                    player->score += e.scoreValue;
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

    // 2. Charged Plasma Shots vs Enemies & Boss
    for (auto& cs : ordnance->chargedShots) {
        if (!cs.active) continue;

        // Check vs Boss
        if (boss && boss->IsActive()) {
            int scoreGained = 0;
            if (boss->CheckLaserHit(cs.position, cs.radius, 110.0f, *particles, camera, scoreGained)) {
                cs.active = false;
                player->score += scoreGained;
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
                // Trigger Area-of-Effect detonation!
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

    // 3. Smart Bombs vs Enemies & Boss (Direct contact detonates immediately)
    for (size_t i = 0; i < ordnance->smartBombs.size(); ++i) {
        auto& b = ordnance->smartBombs[i];
        if (!b.active) continue;

        // Check vs Boss
        if (boss && boss->IsActive()) {
            int scoreGained = 0;
            if (boss->CheckLaserHit(b.position, b.radius, 160.0f, *particles, camera, scoreGained)) {
                ordnance->DetonateBomb(i);
                player->score += scoreGained;
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

    // 4. Expanding Shockwaves vs Everything (Bullet Wipe + Enemy Wipe + Boss Damage)
    for (const auto& sw : ordnance->shockwaves) {
        if (!sw.active) continue;

        // Wipe enemy projectiles caught in the shockwave!
        for (auto& p : projectiles->projectiles) {
            if (!p.active || p.isPlayer) continue;

            float dist = glm::distance(sw.position, p.position);
            if (dist <= sw.currentRadius) {
                p.active = false;
                particles->SpawnExplosion(p.position, 6, glm::vec3(0.3f, 0.85f, 1.0f));
            }
        }

        // Damage Boss weakpoints in shockwave
        if (boss && boss->IsActive()) {
            int scoreGained = 0;
            boss->ApplyShockwaveDamage(sw.position, sw.currentRadius, sw.damage * 0.7f, *particles, camera, scoreGained);
            player->score += scoreGained;
        }

        // Damage enemies caught in shockwave
        for (auto& e : enemies->enemies) {
            if (!e.active) continue;

            float dist = glm::distance(sw.position, e.transform.position);
            if (dist <= sw.currentRadius) {
                e.health -= sw.damage;
                particles->SpawnExplosion(e.transform.position, 12, glm::vec3(0.3f, 0.85f, 1.0f));

                if (e.health <= 0.0f) {
                    e.active = false;
                    player->score += e.scoreValue;
                    particles->SpawnExplosion(e.transform.position, 32, glm::vec3(1.0f, 0.45f, 0.1f));
                    if (audio) {
                        audio->Play(SoundID::ExplosionSmall, 0.8f, 0.95f + ((rand() % 10) * 0.01f));
                    }
                }
            }
        }

        // Shatter asteroids in shockwave
        for (auto& a : environment->asteroids) {
            if (a.destroyed) continue;

            float dist = glm::distance(sw.position, a.position);
            if (dist <= sw.currentRadius) {
                a.destroyed = true;
                particles->SpawnExplosion(a.position, 25, glm::vec3(0.6f, 0.5f, 0.4f));
                if (audio) {
                    audio->Play(SoundID::ExplosionLarge, 0.8f);
                }
            }
        }

        // Shatter hazard pillars in shockwave
        for (auto& pil : environment->pillars) {
            if (pil.destroyed) continue;

            float dist = glm::distance(sw.position, pil.position);
            if (dist <= sw.currentRadius) {
                pil.destroyed = true;
                player->score += 150;
                particles->SpawnExplosion(pil.position + glm::vec3(0.0f, 4.0f, 0.0f), 28, glm::vec3(0.58f, 0.42f, 0.32f));
                if (audio) {
                    audio->Play(SoundID::ExplosionLarge, 0.9f);
                }
            }
        }
    }

    // 5. Lasers & Charged Shots vs Hazard Pillars
    for (auto& p : projectiles->projectiles) {
        if (!p.active || !p.isPlayer) continue;

        for (auto& pil : environment->pillars) {
            if (pil.destroyed) continue;

            float dx = p.position.x - pil.position.x;
            float dz = p.position.z - pil.position.z;
            float dist = std::sqrt(dx * dx + dz * dz);
            if (dist < (p.radius + pil.radius) && p.position.y > -7.5f && p.position.y < 7.0f) {
                p.active = false;
                pil.health -= 25.0f;
                particles->SpawnExplosion(p.position, 8, glm::vec3(0.55f, 0.45f, 0.35f));

                if (pil.health <= 0.0f) {
                    pil.destroyed = true;
                    player->score += 150;
                    particles->SpawnExplosion(pil.position + glm::vec3(0.0f, 4.0f, 0.0f), 30, glm::vec3(0.62f, 0.44f, 0.32f));
                    camera.TriggerShake(0.4f, 0.2f);
                    if (audio) {
                        audio->Play(SoundID::ExplosionLarge, 0.9f);
                    }
                }
                break;
            }
        }
    }

    for (auto& cs : ordnance->chargedShots) {
        if (!cs.active) continue;

        for (auto& pil : environment->pillars) {
            if (pil.destroyed) continue;

            float dx = cs.position.x - pil.position.x;
            float dz = cs.position.z - pil.position.z;
            float dist = std::sqrt(dx * dx + dz * dz);
            if (dist < (cs.radius + pil.radius) && cs.position.y > -7.5f && cs.position.y < 7.0f) {
                cs.active = false;
                pil.destroyed = true;
                player->score += 200;
                ordnance->TriggerShockwave(cs.position, cs.aoeRadius, 90.0f);
                particles->SpawnExplosion(pil.position + glm::vec3(0.0f, 4.0f, 0.0f), 36, glm::vec3(0.65f, 0.45f, 0.35f));
                camera.TriggerShake(0.6f, 0.28f);
                if (audio) {
                    audio->Play(SoundID::BombExplosion, 0.85f);
                }
                break;
            }
        }
    }

    // 6. Player vs Hazard Pillars (with wing-tip bounds)
    for (auto& pil : environment->pillars) {
        if (pil.destroyed) continue;

        // Check left wing collision
        if (!player->leftWingLost) {
            glm::vec3 lTip = player->GetLeftWingTipWorldPos();
            float dxL = lTip.x - pil.position.x;
            float dzL = lTip.z - pil.position.z;
            float distL = std::sqrt(dxL * dxL + dzL * dzL);
            if (distL < (pil.radius + 0.8f) && lTip.y < 7.0f) {
                pil.destroyed = true;
                if (player->IsDeflecting()) {
                    particles->SpawnExplosion(pil.position + glm::vec3(0.0f, 3.0f, 0.0f), 30, glm::vec3(0.2f, 0.9f, 1.0f));
                    camera.TriggerShake(0.35f, 0.2f);
                    if (audio) audio->Play(SoundID::ExplosionLarge, 0.9f);
                } else {
                    bool severed = player->DamageLeftWing(100.0f);
                    particles->SpawnExplosion(lTip, 32, glm::vec3(1.0f, 0.4f, 0.1f));
                    camera.TriggerShake(0.9f, 0.4f);
                    if (audio) {
                        audio->Play(SoundID::WingSnap, 1.0f);
                        audio->Play(SoundID::ExplosionLarge, 0.85f);
                    }
                    player->TakeDamage(15.0f);
                }
                continue;
            }
        }

        // Check right wing collision
        if (!player->rightWingLost) {
            glm::vec3 rTip = player->GetRightWingTipWorldPos();
            float dxR = rTip.x - pil.position.x;
            float dzR = rTip.z - pil.position.z;
            float distR = std::sqrt(dxR * dxR + dzR * dzR);
            if (distR < (pil.radius + 0.8f) && rTip.y < 7.0f) {
                pil.destroyed = true;
                if (player->IsDeflecting()) {
                    particles->SpawnExplosion(pil.position + glm::vec3(0.0f, 3.0f, 0.0f), 30, glm::vec3(0.2f, 0.9f, 1.0f));
                    camera.TriggerShake(0.35f, 0.2f);
                    if (audio) audio->Play(SoundID::ExplosionLarge, 0.9f);
                } else {
                    bool severed = player->DamageRightWing(100.0f);
                    particles->SpawnExplosion(rTip, 32, glm::vec3(1.0f, 0.4f, 0.1f));
                    camera.TriggerShake(0.9f, 0.4f);
                    if (audio) {
                        audio->Play(SoundID::WingSnap, 1.0f);
                        audio->Play(SoundID::ExplosionLarge, 0.85f);
                    }
                    player->TakeDamage(15.0f);
                }
                continue;
            }
        }

        // Check central fuselage collision
        float dx = player->transform.position.x - pil.position.x;
        float dz = player->transform.position.z - pil.position.z;
        float dist = std::sqrt(dx * dx + dz * dz);
        if (dist < (pil.radius + 1.2f) && player->transform.position.y < 7.0f) {
            pil.destroyed = true;

            if (player->IsDeflecting()) {
                // Deflection spin obliterates the pillar!
                particles->SpawnExplosion(pil.position + glm::vec3(0.0f, 3.0f, 0.0f), 30, glm::vec3(0.2f, 0.9f, 1.0f));
                camera.TriggerShake(0.35f, 0.2f);
                if (audio) {
                    audio->Play(SoundID::ExplosionLarge, 0.9f);
                }
            } else {
                player->TakeDamage(30.0f);
                player->DamageLeftWing(35.0f);
                player->DamageRightWing(35.0f);
                particles->SpawnExplosion(player->transform.position, 35, glm::vec3(0.7f, 0.45f, 0.3f));
                camera.TriggerShake(1.0f, 0.5f);
                if (audio) {
                    audio->Play(SoundID::ExplosionLarge, 1.0f, 0.65f);
                }

                if (player->shield <= 0.0f) {
                    state = GameState::GameOver;
                }
            }
        }
    }

    // 5. Enemy Lasers vs Player (with wing bounds)
    for (auto& p : projectiles->projectiles) {
        if (!p.active || p.isPlayer) continue;

        // Check Left Wing hit
        if (!player->leftWingLost) {
            float distL = glm::distance(p.position, player->GetLeftWingTipWorldPos());
            if (distL < (p.radius + 1.1f)) {
                p.active = false;
                if (player->IsDeflecting()) {
                    particles->SpawnExplosion(p.position, 14, glm::vec3(0.2f, 0.9f, 1.0f));
                    camera.TriggerShake(0.2f, 0.15f);
                    if (audio) audio->Play(SoundID::LockOnPing, 0.85f, 1.6f);
                } else {
                    bool severed = player->DamageLeftWing(35.0f);
                    particles->SpawnExplosion(p.position, 14, glm::vec3(1.0f, 0.4f, 0.2f));
                    camera.TriggerShake(0.6f, 0.3f);
                    if (audio) {
                        audio->Play(severed ? SoundID::WingSnap : SoundID::ExplosionSmall, 0.95f);
                    }
                    player->TakeDamage(10.0f);
                    if (player->shield <= 0.0f) {
                        state = GameState::GameOver;
                    }
                }
                continue;
            }
        }

        // Check Right Wing hit
        if (!player->rightWingLost) {
            float distR = glm::distance(p.position, player->GetRightWingTipWorldPos());
            if (distR < (p.radius + 1.1f)) {
                p.active = false;
                if (player->IsDeflecting()) {
                    particles->SpawnExplosion(p.position, 14, glm::vec3(0.2f, 0.9f, 1.0f));
                    camera.TriggerShake(0.2f, 0.15f);
                    if (audio) audio->Play(SoundID::LockOnPing, 0.85f, 1.6f);
                } else {
                    bool severed = player->DamageRightWing(35.0f);
                    particles->SpawnExplosion(p.position, 14, glm::vec3(1.0f, 0.4f, 0.2f));
                    camera.TriggerShake(0.6f, 0.3f);
                    if (audio) {
                        audio->Play(severed ? SoundID::WingSnap : SoundID::ExplosionSmall, 0.95f);
                    }
                    player->TakeDamage(10.0f);
                    if (player->shield <= 0.0f) {
                        state = GameState::GameOver;
                    }
                }
                continue;
            }
        }

        // Center fuselage hit
        float dist = glm::distance(p.position, player->transform.position);
        if (dist < (p.radius + 1.8f)) {
            p.active = false;

            if (player->IsDeflecting()) {
                // Deflect laser during barrel roll!
                particles->SpawnExplosion(p.position, 14, glm::vec3(0.2f, 0.9f, 1.0f));
                camera.TriggerShake(0.2f, 0.15f);
                if (audio) {
                    audio->Play(SoundID::LockOnPing, 0.85f, 1.6f);
                }
            } else {
                player->TakeDamage(15.0f);
                particles->SpawnExplosion(p.position, 12, glm::vec3(1.0f, 0.2f, 0.2f));
                camera.TriggerShake(0.6f, 0.35f);
                if (audio) {
                    audio->Play(SoundID::ExplosionSmall, 0.95f, 0.65f);
                }

                if (player->shield <= 0.0f) {
                    particles->SpawnExplosion(player->transform.position, 60, glm::vec3(1.0f, 0.5f, 0.1f));
                    camera.TriggerShake(1.2f, 0.7f);
                    state = GameState::GameOver;
                }
            }
        }
    }

    // 6. Player vs Rings (Gold awards bombs; Silver repairs wings & heals shield!)
    for (auto& r : environment->rings) {
        if (r.collected) continue;

        float dist = glm::distance(player->transform.position, r.position);
        if (dist < r.radius * 1.15f) {
            r.collected = true;
            player->ringsCollected++;
            player->score += r.isGold ? 1000 : 500;
            player->AddShield(r.isGold ? 35.0f : 25.0f);

            // Silver rings repair both wings!
            if (!r.isGold) {
                bool neededRepair = (player->leftWingLost || player->rightWingLost ||
                                     player->leftWingHealth < 100.0f || player->rightWingHealth < 100.0f);
                player->RepairWings();
                if (neededRepair && audio) {
                    audio->Play(SoundID::WingRepair, 1.0f);
                }
            }

            // Every 3 gold rings grants an extra Smart Bomb!
            if (r.isGold && player->ringsCollected % 3 == 0) {
                player->AddBombs(1);
            }

            particles->SpawnExplosion(r.position, 22,
                                      r.isGold ? glm::vec3(1.0f, 0.9f, 0.3f) : glm::vec3(0.3f, 0.9f, 1.0f));
            if (audio) {
                audio->Play(SoundID::RingCollect, 0.9f);
            }
        }
    }

    // 7. Player vs Asteroids (with wing bounds)
    for (auto& a : environment->asteroids) {
        if (a.destroyed) continue;

        // Check Left Wing
        if (!player->leftWingLost) {
            float distL = glm::distance(player->GetLeftWingTipWorldPos(), a.position);
            if (distL < (a.radius + 1.0f)) {
                a.destroyed = true;
                if (player->IsDeflecting()) {
                    particles->SpawnExplosion(a.position, 30, glm::vec3(0.2f, 0.9f, 1.0f));
                    camera.TriggerShake(0.3f, 0.2f);
                } else {
                    bool severed = player->DamageLeftWing(50.0f);
                    particles->SpawnExplosion(player->GetLeftWingTipWorldPos(), 28, glm::vec3(0.8f, 0.5f, 0.3f));
                    camera.TriggerShake(0.8f, 0.35f);
                    if (audio) {
                        audio->Play(severed ? SoundID::WingSnap : SoundID::ExplosionLarge, 0.95f);
                    }
                    player->TakeDamage(12.0f);
                }
                continue;
            }
        }

        // Check Right Wing
        if (!player->rightWingLost) {
            float distR = glm::distance(player->GetRightWingTipWorldPos(), a.position);
            if (distR < (a.radius + 1.0f)) {
                a.destroyed = true;
                if (player->IsDeflecting()) {
                    particles->SpawnExplosion(a.position, 30, glm::vec3(0.2f, 0.9f, 1.0f));
                    camera.TriggerShake(0.3f, 0.2f);
                } else {
                    bool severed = player->DamageRightWing(50.0f);
                    particles->SpawnExplosion(player->GetRightWingTipWorldPos(), 28, glm::vec3(0.8f, 0.5f, 0.3f));
                    camera.TriggerShake(0.8f, 0.35f);
                    if (audio) {
                        audio->Play(severed ? SoundID::WingSnap : SoundID::ExplosionLarge, 0.95f);
                    }
                    player->TakeDamage(12.0f);
                }
                continue;
            }
        }

        // Center collision
        float dist = glm::distance(player->transform.position, a.position);
        if (dist < (a.radius + 1.6f)) {
            a.destroyed = true;
            particles->SpawnExplosion(a.position, 35, glm::vec3(0.7f, 0.6f, 0.5f));
            camera.TriggerShake(0.9f, 0.45f);
            if (audio) {
                audio->Play(SoundID::ExplosionLarge, 0.85f);
            }

            if (!player->IsDeflecting()) {
                player->TakeDamage(25.0f);
                if (player->shield <= 0.0f) {
                    state = GameState::GameOver;
                }
            }
        }
    }
}

void Engine::Update(float dt) {
    Input::Update(dt);
    ProcessInput(dt);

    if (state == GameState::Playing) {
        player->Update(dt);

        // Smoke & electrical spark trails from severed wing roots
        if (player->leftWingLost) {
            glm::vec3 leftRoot = player->GetLeftWingRootWorldPos();
            particles->SpawnExplosion(leftRoot, 1, glm::vec3(0.35f, 0.35f, 0.38f)); // Smoke puff
            if (rand() % 3 == 0) {
                particles->SpawnExplosion(leftRoot, 1, glm::vec3(1.0f, 0.55f, 0.15f)); // Fire spark
            }
        }
        if (player->rightWingLost) {
            glm::vec3 rightRoot = player->GetRightWingRootWorldPos();
            particles->SpawnExplosion(rightRoot, 1, glm::vec3(0.35f, 0.35f, 0.38f)); // Smoke puff
            if (rand() % 3 == 0) {
                particles->SpawnExplosion(rightRoot, 1, glm::vec3(1.0f, 0.55f, 0.15f)); // Fire spark
            }
        }

        // Spawn Boss when corridor threshold is reached
        if (!bossSpawned && player->transform.position.z <= -950.0f) {
            bossSpawned = true;
            boss->Spawn(player->transform.position.z);
        }

        // Lock-on targeting during charge shot (prioritize boss weakpoints)
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
                int lockIdx = enemies->FindLockTarget(player->transform.position, player->GetFarTargetPos());
                if (lockIdx >= 0) {
                    player->hasLockOn = true;
                    player->lockTargetPos = enemies->enemies[lockIdx].transform.position;
                } else {
                    player->hasLockOn = false;
                }
            }
        } else {
            player->hasLockOn = false;
        }

        // Lock-on sound chime
        if (player->hasLockOn && !hadLockOnLastFrame && audio) {
            audio->Play(SoundID::LockOnPing, 0.95f);
        }
        hadLockOnLastFrame = player->hasLockOn;

        // Tactical Barrel Roll whoosh sound
        static bool wasSpinning = false;
        if (player->isSpinning && !wasSpinning && audio) {
            audio->Play(SoundID::BarrelRoll, 0.95f);
        }
        wasSpinning = player->isSpinning;

        // Boost thruster audio loop
        if (player->isBoosting) {
            camera.SetTargetFOV(72.0f);
            if (!wasBoostingAudio && audio) {
                audio->PlayLoop(SoundID::BoostRoar, 0.70f);
                wasBoostingAudio = true;
            }
        } else if (wasBoostingAudio && audio) {
            audio->StopLoop(SoundID::BoostRoar);
            wasBoostingAudio = false;
        }

        // Airbrake audio loop
        if (player->isBraking) {
            camera.SetTargetFOV(54.0f);
            if (!wasBrakingAudio && audio) {
                audio->PlayLoop(SoundID::BrakeHiss, 0.55f);
                wasBrakingAudio = true;
            }
        } else if (wasBrakingAudio && audio) {
            audio->StopLoop(SoundID::BrakeHiss);
            wasBrakingAudio = false;
        }

        if (!player->isBoosting && !player->isBraking) {
            camera.SetTargetFOV(60.0f);
        }

        // Charge hum audio loop
        if (player->isCharging) {
            if (!wasChargingAudio && audio) {
                audio->PlayLoop(SoundID::ChargeHum, 0.75f);
                wasChargingAudio = true;
            }
        } else if (wasChargingAudio && audio) {
            audio->StopLoop(SoundID::ChargeHum);
            wasChargingAudio = false;
        }

        camera.Follow(player->transform.position, player->currentBank, dt);

        // Emit engine thruster exhaust sparks
        glm::vec3 shipVel(0.0f, 0.0f, -player->currentSpeed);
        glm::mat4 pModel = player->transform.GetModelMatrix();
        glm::vec3 leftExhaust = glm::vec3(pModel * glm::vec4(-0.28f, 0.0f, 1.45f, 1.0f));
        glm::vec3 rightExhaust = glm::vec3(pModel * glm::vec4(0.28f, 0.0f, 1.45f, 1.0f));
        particles->SpawnThrusterSparks(leftExhaust, shipVel);
        particles->SpawnThrusterSparks(rightExhaust, shipVel);

        projectiles->Update(dt);
        ordnance->Update(dt);
        particles->Update(dt);
        environment->Update(player->transform.position.z, dt);

        if (boss && boss->IsActive()) {
            enemies->spawnTimer = 0.0f; // Pause new drone waves during boss fight
        }
        enemies->Update(player->transform.position.z, player->transform.position, *projectiles, dt, audio.get());

        if (boss) {
            boss->Update(dt, player->transform.position.z, player->transform.position,
                         *projectiles, *particles, camera, audio.get());

            // Warning Siren Audio Loop
            if (boss->IsWarning()) {
                if (audio && !audio->IsLoopPlaying(SoundID::WarningSiren)) {
                    audio->PlayLoop(SoundID::WarningSiren, 0.85f);
                }
            } else if (audio && audio->IsLoopPlaying(SoundID::WarningSiren)) {
                audio->StopLoop(SoundID::WarningSiren);
            }

            if (boss->IsDefeated()) {
                victoryTimer += dt;
                if (victoryTimer >= 1.2f) {
                    state = GameState::Victory;
                    if (!victoryFanfarePlayed && audio) {
                        audio->Play(SoundID::VictoryFanfare, 1.0f);
                        victoryFanfarePlayed = true;
                    }
                }
            }
        }

        HandleCollisions();
    }

    camera.Update(dt);
}

void Engine::Render() {
    // Warm atmospheric dusk canyon sky
    glClearColor(0.20f, 0.11f, 0.10f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    shader.Activate();

    // 1. Matrices
    glm::mat4 view = camera.GetViewMatrix();
    glm::mat4 proj = camera.GetProjectionMatrix();
    shader.SetMat4("uView", view);
    shader.SetMat4("uProjection", proj);

    // 2. Golden-hour canyon lighting and distance depth fog
    shader.SetVec3("uLightDir", glm::normalize(glm::vec3(0.5f, 0.85f, 0.45f)));
    shader.SetVec3("uLightColor", glm::vec3(1.0f, 0.94f, 0.82f));
    shader.SetVec3("uAmbientColor", glm::vec3(0.40f, 0.30f, 0.28f));
    shader.SetVec3("uFogColor", glm::vec3(0.20f, 0.11f, 0.10f));
    shader.SetFloat("uFogStart", 90.0f);
    shader.SetFloat("uFogEnd", 290.0f);
    shader.SetVec3("uCameraPos", camera.position);
    shader.SetInt("uUseLighting", 1);
    shader.SetInt("uUseFog", 1);
    shader.SetInt("uUseColorOverride", 0);
    shader.SetFloat("uAlpha", 1.0f);

    // 3. Draw 3D world entities
    environment->Draw(shader);
    enemies->Draw(shader);
    if (boss) {
        boss->Draw(shader);
    }
    projectiles->Draw(shader);
    ordnance->Draw(shader);
    particles->Draw(shader);

    if (state == GameState::Playing || state == GameState::Victory) {
        player->Draw(shader);
        if (state == GameState::Playing) {
            reticle->Draw(shader, player->GetNearTargetPos(), player->GetFarTargetPos(),
                          player->hasLockOn, player->lockTargetPos, player->lockRotation);
        }
    }

    // 4. Draw Cockpit HUD
    bool bActive = boss && boss->IsActive();
    bool bWarn = boss && boss->IsWarning();
    float bHealthRatio = boss ? boss->GetHealthRatio() : 0.0f;
    bool bLDown = boss ? boss->leftTurret.destroyed : false;
    bool bRDown = boss ? boss->rightTurret.destroyed : false;
    bool bSDown = boss ? boss->shieldGen.destroyed : false;
    bool bCoreExp = boss ? (boss->state == BossState::Phase2_ExposedCore) : false;

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
                player->wingAlertTimer, player->wingAlertMessage);

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
