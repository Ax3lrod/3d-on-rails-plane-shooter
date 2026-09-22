#ifndef WINGMAN_SQUADRON_H
#define WINGMAN_SQUADRON_H

#include <glm/glm.hpp>
#include <string>
#include <vector>
#include <deque>
#include "Transform.h"
#include "Mesh.h"
#include "shaderClass.h"
#include "CombatSystem.h"
#include "EnemyManager.h"
#include "SoundManager.h"

enum class WingmanID {
    Striker, // Echo-1: aggressive point scout, orange accents
    Aegis    // Echo-2: tactical escort, cyan accents
};

enum class WingmanState {
    InFormation,
    Attacking,
    Distressed, // Enemy bogey tailing them!
    Rescued,    // Just rescued by player!
    Retreating  // Heavy damage, emergency high-altitude evasion
};

struct Wingman {
    WingmanID id;
    std::string callsign;
    std::string pilotName;
    glm::vec3 themeColor;
    float voicePitch;

    Transform transform;
    Mesh mesh;

    float shield;
    float maxShield;
    WingmanState state;

    glm::vec3 formationOffset; // Base offset relative to player
    float fireTimer;
    float fireRate;

    // Distress system
    float distressTimer;
    int tailingEnemyIndex; // index into EnemyManager::enemies
    std::vector<int> pursuerIndices; // tracked pursuer enemies for scripted rescue

    // Flight animation
    float bobOffset;
    float evasiveTimer;
    float currentBank;
    float currentPitch;
    float currentYaw;

    Wingman(WingmanID id, const std::string& callsign, const std::string& pilot,
            const glm::vec3& color, float pitch, const glm::vec3& offset);
};

struct CommsMessage {
    WingmanID speaker;
    std::string callsign;
    std::string line1;
    std::string line2;
    glm::vec3 color;
    float voicePitch;
    float duration;
    float timer;
    float chatterTimer;
};

class WingmanSquadron {
public:
    std::vector<Wingman> wingmen;
    std::deque<CommsMessage> messageQueue;
    CommsMessage activeMessage;
    bool hasActiveMessage;

    // Story / checkpoint progression flags
    bool introTriggered;
    bool wave1Triggered;
    bool asteroidTriggered;
    bool bossTriggered;
    bool bossTurretTriggered;
    bool bossCoreTriggered;
    bool victoryTriggered;
    bool wingDamageAlertTriggered;

    float nextDistressCheckTime;
    bool pendingSupplyDrop;
    glm::vec3 supplyDropPosition;

    WingmanSquadron();

    void TriggerTransmission(WingmanID speaker, const std::string& line1, const std::string& line2,
                             float duration = 3.6f, SoundManager* audio = nullptr);

    void TriggerScriptedRescue(WingmanID target, int threatCount, float timeout,
                               const std::string& line1, const std::string& line2,
                               EnemyManager& enemies, SoundManager* audio = nullptr);

    void Update(float dt, const glm::vec3& playerPos, float playerHeadingYaw,
                float playerSpeed, bool isAllRange, bool playerLeftWingLost, bool playerRightWingLost,
                ProjectileManager& projectiles, ParticleSystem& particles,
                EnemyManager& enemies, SoundManager* audio, int& outScoreGained);

    void Draw(const Shader& shader) const;

    void Reset();
};

#endif
