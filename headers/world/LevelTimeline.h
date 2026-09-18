#ifndef LEVEL_TIMELINE_H
#define LEVEL_TIMELINE_H

#include <string>
#include <vector>
#include <glm/glm.hpp>
#include "EnemyManager.h"
#include "WingmanSquadron.h"
#include "SoundManager.h"

enum class TimelineEventType {
    SpawnWave,
    Transmission,
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
    float spawnZ = 0.0f; // 0.0f = auto-project 220 units ahead of player
    float spacingX = 4.5f;
    float spacingZ = 12.0f;

    // Radio comms fields
    WingmanID speaker = WingmanID::Striker;
    std::string line1;
    std::string line2;
    float duration = 4.0f;
};

class LevelTimeline {
public:
    std::string stageName;
    std::string musicTrack;
    std::vector<TimelineEvent> events;
    bool isLoaded;
    bool bossTriggered;

    LevelTimeline();

    bool LoadFromFile(const std::string& filepath);
    void Reset();
    void Update(float playerZ, EnemyManager& enemies, WingmanSquadron* wingmen, SoundManager* audio);
};

#endif
