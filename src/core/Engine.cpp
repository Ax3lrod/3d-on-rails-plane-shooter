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
    hud.reset();
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
    hud = std::make_unique<HUD>();

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
    state = GameState::Playing;
}

void Engine::ProcessInput(float) {
    if (state == GameState::GameOver) {
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

            glm::vec3 leftMuzzle = player->GetLeftMuzzlePos();
            glm::vec3 rightMuzzle = player->GetRightMuzzlePos();
            glm::vec3 target = player->GetFarTargetPos();

            projectiles->SpawnLaser(leftMuzzle, target, true, 220.0f);
            projectiles->SpawnLaser(rightMuzzle, target, true, 220.0f);

            particles->SpawnExplosion(leftMuzzle, 3, glm::vec3(0.2f, 1.0f, 0.5f));
            particles->SpawnExplosion(rightMuzzle, 3, glm::vec3(0.2f, 1.0f, 0.5f));
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
        }
    }

    // 4. Smart Bomb (Launch or Detonate Early)
    if (Input::IsKeyPressed(GLFW_KEY_B) || Input::IsKeyPressed(GLFW_KEY_K)) {
        if (!ordnance->smartBombs.empty()) {
            // Early manual detonation of in-flight bomb
            ordnance->DetonateBomb(0);
            camera.TriggerShake(1.2f, 0.6f);
        } else if (player->LaunchBomb()) {
            glm::vec3 nosePos = player->GetNosePos();
            ordnance->SpawnSmartBomb(nosePos, glm::vec3(0.0f, 0.0f, -1.0f));
            particles->SpawnExplosion(nosePos, 12, glm::vec3(1.0f, 0.85f, 0.2f));
            camera.TriggerShake(0.35f, 0.18f);
        }
    }
}

void Engine::HandleCollisions() {
    if (state != GameState::Playing) return;

    // 1. Player Regular Lasers vs Enemies
    for (auto& p : projectiles->projectiles) {
        if (!p.active || !p.isPlayer) continue;

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
                }
                break;
            }
        }
    }

    // 2. Charged Plasma Shots vs Enemies
    for (auto& cs : ordnance->chargedShots) {
        if (!cs.active) continue;

        for (auto& e : enemies->enemies) {
            if (!e.active) continue;

            float dist = glm::distance(cs.position, e.transform.position);
            if (dist < (cs.radius + e.radius)) {
                cs.active = false;
                // Trigger Area-of-Effect detonation!
                ordnance->TriggerShockwave(cs.position, cs.aoeRadius, 90.0f);
                particles->SpawnExplosion(cs.position, 40, glm::vec3(0.2f, 1.0f, 0.8f));
                camera.TriggerShake(0.75f, 0.35f);
                break;
            }
        }
    }

    // 3. Smart Bombs vs Enemies (Direct contact detonates immediately)
    for (size_t i = 0; i < ordnance->smartBombs.size(); ++i) {
        auto& b = ordnance->smartBombs[i];
        if (!b.active) continue;

        for (auto& e : enemies->enemies) {
            if (!e.active) continue;

            float dist = glm::distance(b.position, e.transform.position);
            if (dist < (b.radius + e.radius)) {
                ordnance->DetonateBomb(i);
                camera.TriggerShake(1.2f, 0.6f);
                break;
            }
        }
    }

    // 4. Expanding Shockwaves vs Everything (Bullet Wipe + Enemy Wipe)
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
                break;
            }
        }
    }

    // 6. Player vs Hazard Pillars
    for (auto& pil : environment->pillars) {
        if (pil.destroyed) continue;

        float dx = player->transform.position.x - pil.position.x;
        float dz = player->transform.position.z - pil.position.z;
        float dist = std::sqrt(dx * dx + dz * dz);
        if (dist < (pil.radius + 1.2f) && player->transform.position.y < 7.0f) {
            pil.destroyed = true;

            if (player->IsDeflecting()) {
                // Deflection spin obliterates the pillar!
                particles->SpawnExplosion(pil.position + glm::vec3(0.0f, 3.0f, 0.0f), 30, glm::vec3(0.2f, 0.9f, 1.0f));
                camera.TriggerShake(0.35f, 0.2f);
            } else {
                player->TakeDamage(30.0f);
                particles->SpawnExplosion(player->transform.position, 35, glm::vec3(0.7f, 0.45f, 0.3f));
                camera.TriggerShake(1.0f, 0.5f);

                if (player->shield <= 0.0f) {
                    state = GameState::GameOver;
                }
            }
        }
    }

    // 5. Enemy Lasers vs Player
    for (auto& p : projectiles->projectiles) {
        if (!p.active || p.isPlayer) continue;

        float dist = glm::distance(p.position, player->transform.position);
        if (dist < (p.radius + 1.8f)) {
            p.active = false;

            if (player->IsDeflecting()) {
                // Deflect laser during barrel roll!
                particles->SpawnExplosion(p.position, 14, glm::vec3(0.2f, 0.9f, 1.0f));
                camera.TriggerShake(0.2f, 0.15f);
            } else {
                player->TakeDamage(15.0f);
                particles->SpawnExplosion(p.position, 12, glm::vec3(1.0f, 0.2f, 0.2f));
                camera.TriggerShake(0.6f, 0.35f);

                if (player->shield <= 0.0f) {
                    particles->SpawnExplosion(player->transform.position, 60, glm::vec3(1.0f, 0.5f, 0.1f));
                    camera.TriggerShake(1.2f, 0.7f);
                    state = GameState::GameOver;
                }
            }
        }
    }

    // 6. Player vs Rings
    for (auto& r : environment->rings) {
        if (r.collected) continue;

        float dist = glm::distance(player->transform.position, r.position);
        if (dist < r.radius * 1.15f) {
            r.collected = true;
            player->ringsCollected++;
            player->score += r.isGold ? 1000 : 500;
            player->AddShield(r.isGold ? 35.0f : 20.0f);

            // Every 3 gold rings grants an extra Smart Bomb!
            if (r.isGold && player->ringsCollected % 3 == 0) {
                player->AddBombs(1);
            }

            particles->SpawnExplosion(r.position, 22,
                                      r.isGold ? glm::vec3(1.0f, 0.9f, 0.3f) : glm::vec3(0.3f, 0.9f, 1.0f));
        }
    }

    // 7. Player vs Asteroids
    for (auto& a : environment->asteroids) {
        if (a.destroyed) continue;

        float dist = glm::distance(player->transform.position, a.position);
        if (dist < (a.radius + 1.6f)) {
            a.destroyed = true;
            particles->SpawnExplosion(a.position, 35, glm::vec3(0.7f, 0.6f, 0.5f));
            camera.TriggerShake(0.9f, 0.45f);

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

        // Lock-on enemy targeting during charge shot
        if (player->isCharging) {
            particles->SpawnChargeInwardSparks(player->GetNosePos(), player->GetChargeProgress());
            int lockIdx = enemies->FindLockTarget(player->transform.position, player->GetFarTargetPos());
            if (lockIdx >= 0) {
                player->hasLockOn = true;
                player->lockTargetPos = enemies->enemies[lockIdx].transform.position;
            } else {
                player->hasLockOn = false;
            }
        } else {
            player->hasLockOn = false;
        }

        // Adjust camera FOV for boost warp
        if (player->isBoosting) {
            camera.SetTargetFOV(72.0f);
        } else if (player->isBraking) {
            camera.SetTargetFOV(54.0f);
        } else {
            camera.SetTargetFOV(60.0f);
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
        enemies->Update(player->transform.position.z, player->transform.position, *projectiles, dt);

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

    // 3. Draw 3D world entities
    environment->Draw(shader);
    enemies->Draw(shader);
    projectiles->Draw(shader);
    ordnance->Draw(shader);
    particles->Draw(shader);

    if (state == GameState::Playing) {
        player->Draw(shader);
        reticle->Draw(shader, player->GetNearTargetPos(), player->GetFarTargetPos(),
                      player->hasLockOn, player->lockTargetPos, player->lockRotation);
    }

    // 4. Draw Cockpit HUD
    hud->Render(shader, windowWidth, windowHeight,
                player->shield, player->maxShield,
                player->boostMeter, player->maxBoost, player->isOverheated,
                player->IsDeflecting(), player->score, player->ringsCollected,
                player->bombCount, player->GetChargeProgress(),
                state == GameState::GameOver);

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
