#ifndef ENGINE_H
#define ENGINE_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <string>
#include <memory>

#include "shaderClass.h"
#include "Camera.h"
#include "PlayerStarfighter.h"
#include "CombatSystem.h"
#include "WorldEnvironment.h"
#include "EnemyManager.h"
#include "BossDreadnought.h"
#include "HUD.h"
#include "SoundManager.h"
#include "WingmanSquadron.h"

enum class GameState {
    Playing,
    Victory,
    GameOver
};

class Engine {
public:
    Engine(int width = 1280, int height = 720, const std::string& title = "Aegis Starfighter - 3D Rail Shooter Engine");
    ~Engine();

    bool Init();
    void Run();

private:
    void ProcessInput(float dt);
    void Update(float dt);
    void Render();
    void HandleCollisions();
    void RestartGame();

    static void FramebufferSizeCallback(GLFWwindow* window, int width, int height);

    GLFWwindow* window;
    int windowWidth;
    int windowHeight;
    std::string windowTitle;

    GameState state;

    Shader shader;
    Camera camera;

    std::unique_ptr<PlayerStarfighter> player;
    std::unique_ptr<ProjectileManager> projectiles;
    std::unique_ptr<OrdnanceManager> ordnance;
    std::unique_ptr<ParticleSystem> particles;
    std::unique_ptr<TargetingReticle> reticle;
    std::unique_ptr<WorldEnvironment> environment;
    std::unique_ptr<EnemyManager> enemies;
    std::unique_ptr<BossDreadnought> boss;
    std::unique_ptr<HUD> hud;
    std::unique_ptr<SoundManager> audio;
    std::unique_ptr<WingmanSquadron> wingmen;

    bool bossSpawned;
    float victoryTimer;
    bool wasChargingAudio;
    bool wasBoostingAudio;
    bool wasBrakingAudio;
    bool hadLockOnLastFrame;
    bool victoryFanfarePlayed;
    bool bossDeathCamTriggered;
    float lastFrameTime;
};

#endif
