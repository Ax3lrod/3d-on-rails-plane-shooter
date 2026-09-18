#ifndef LEVEL_TIMELINE_H
#define LEVEL_TIMELINE_H

#include <string>
#include <vector>
#include <glm/glm.hpp>
#include "EnemyManager.h"
#include "WingmanSquadron.h"
#include "SoundManager.h"

class TrainConvoy;

enum class TimelineEventType {
    SpawnWave,
    Transmission,
    SpawnTrain,
    BossTrigger,
    ClearEnemies
};

struct TimelineEvent {
    float triggerZ = 0.0f;
    TimelineEventType type = TimelineEventType::SpawnWave;
    bool triggered = false;

    // Enemy spawn fields
    EnemyType enemyType = EnemyType::Drone;
    std::string formation = "v_formation";
    int count = 1;
    float spawnX = 0.0f;
    float spawnY = 0.0f;
    float spawnZ = 0.0f; // 0.0f = auto-project ahead of player
    float spacingX = 4.5f;
    float spacingZ = 12.0f;

    // Train spawn fields
    float trackX = 22.0f;

    // Radio comms fields
    WingmanID speaker = WingmanID::Striker;
    std::string line1;
    std::string line2;
    float duration = 4.0f;
};

struct StageDefinition {
    std::string id;
    std::string title;
    std::string subtitle;
    std::string scriptPath;
    std::string bossName;
    std::string bossType; // "dreadnought", "twin_helicopters", "mega_tank", "sandworm"
    std::string terrainTheme; // "coastline", "railway_canyon", "iron_fortress", "dune_pass", "cosmic_debris"
    std::string difficulty; // "NORMAL", "HARD", "EXPERT"
    glm::vec3 themeColor;
};

class LevelTimeline {
public:
    std::string stageName;
    std::string musicTrack;
    std::string bossType;
    std::vector<TimelineEvent> events;
    bool isLoaded;
    bool bossTriggered;

    LevelTimeline();

    bool LoadFromFile(const std::string& filepath);
    void Reset();
    void Update(float playerZ, EnemyManager& enemies, WingmanSquadron* wingmen,
                TrainConvoy* train, SoundManager* audio);

    static std::vector<StageDefinition> GetStandardCampaignStages();
};

#endif
