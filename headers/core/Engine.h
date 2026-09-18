#ifndef ENGINE_H
#define ENGINE_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <string>
#include <memory>
#include <vector>

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
#include "PostProcessor.h"
#include "LevelTimeline.h"

enum class GameState {
    TitleHangar,
    SettingsMenu,
    MissionBriefing,
    Playing,
    StageClearWarp,
    Victory,
    GameOver,
    Leaderboard
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

    void LoadHighScores();
    void SaveHighScores();
    void CheckNewHighScore(int score, int stage, bool complete);

    void StartMission();
    void TriggerHyperspaceWarp();
    void CompleteHyperspaceWarp();
    void ReturnToTitle();

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
    std::unique_ptr<PostProcessor> postProcessor;
    std::unique_ptr<LevelTimeline> levelTimeline;

    // Hangar & Menu
    std::unique_ptr<Mesh> hangarFloorMesh;
    std::unique_ptr<Mesh> turntableMesh;
    float hangarRotAngle;
    int selectedTitleMenu;
    int selectedSettingsIndex;
    bool invertPitchY;
    bool defaultCockpitMode;
    bool flatShading;

    // Visual FX
    std::unique_ptr<Mesh> shadowMesh;

    // Route & Warp
    float warpTransitionTimer;
    bool hardRouteWon;
    std::vector<HighScoreEntry> highScores;

    // Ex-Zodiac Hit Combo Chain System
    int comboHits;
    float comboTimer;
    float comboMaxDuration;
    float comboAnimScale;
    void RegisterHitCombo(const glm::vec3& hitPos, int baseScore);

    bool bossSpawned;
    float victoryTimer;
    bool wasChargingAudio;
    bool wasBoostingAudio;
    bool wasBrakingAudio;
    bool hadLockOnLastFrame;
    bool wasSomersaultingAudio;
    bool victoryFanfarePlayed;
    bool bossDeathCamTriggered;
    float lastFrameTime;
};

#endif
