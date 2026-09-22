#include "LevelTimeline.h"
#include "TrainConvoy.h"
#include "WorldEnvironment.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <unordered_map>
#include <cctype>

namespace {
    enum class JsonType { Null, Bool, Number, String, Array, Object };

    struct JsonValue {
        JsonType type = JsonType::Null;
        bool bVal = false;
        double nVal = 0.0;
        std::string sVal;
        std::vector<JsonValue> aVal;
        std::unordered_map<std::string, JsonValue> oVal;

        bool has(const std::string& key) const {
            return type == JsonType::Object && oVal.find(key) != oVal.end();
        }

        const JsonValue& operator[](const std::string& key) const {
            static JsonValue dummy;
            if (type != JsonType::Object) return dummy;
            auto it = oVal.find(key);
            return (it != oVal.end()) ? it->second : dummy;
        }

        const JsonValue& operator[](size_t idx) const {
            static JsonValue dummy;
            if (type != JsonType::Array || idx >= aVal.size()) return dummy;
            return aVal[idx];
        }

        std::string asString(const std::string& def = "") const {
            return type == JsonType::String ? sVal : def;
        }

        float asFloat(float def = 0.0f) const {
            return type == JsonType::Number ? static_cast<float>(nVal) : def;
        }

        int asInt(int def = 0) const {
            return type == JsonType::Number ? static_cast<int>(nVal) : def;
        }

        bool asBool(bool def = false) const {
            return type == JsonType::Bool ? bVal : def;
        }
    };

    void skipWhitespace(const std::string& str, size_t& pos) {
        while (pos < str.size()) {
            char c = str[pos];
            if (c == ' ' || c == '\t' || c == '\r' || c == '\n') {
                pos++;
            } else if (c == '/' && pos + 1 < str.size() && str[pos + 1] == '/') {
                pos += 2;
                while (pos < str.size() && str[pos] != '\n') pos++;
            } else {
                break;
            }
        }
    }

    std::string parseString(const std::string& str, size_t& pos) {
        std::string result;
        if (pos >= str.size() || str[pos] != '"') return result;
        pos++;
        while (pos < str.size()) {
            char c = str[pos++];
            if (c == '"') {
                break;
            } else if (c == '\\' && pos < str.size()) {
                char esc = str[pos++];
                if (esc == '"') result += '"';
                else if (esc == '\\') result += '\\';
                else if (esc == 'n') result += '\n';
                else if (esc == 't') result += '\t';
                else result += esc;
            } else {
                result += c;
            }
        }
        return result;
    }

    JsonValue parseValue(const std::string& str, size_t& pos);

    JsonValue parseObject(const std::string& str, size_t& pos) {
        JsonValue val;
        val.type = JsonType::Object;
        pos++;
        skipWhitespace(str, pos);

        while (pos < str.size() && str[pos] != '}') {
            skipWhitespace(str, pos);
            if (pos >= str.size() || str[pos] != '"') break;
            std::string key = parseString(str, pos);
            skipWhitespace(str, pos);
            if (pos < str.size() && str[pos] == ':') {
                pos++;
            }
            skipWhitespace(str, pos);
            JsonValue child = parseValue(str, pos);
            val.oVal[key] = child;

            skipWhitespace(str, pos);
            if (pos < str.size() && str[pos] == ',') {
                pos++;
            }
            skipWhitespace(str, pos);
        }
        if (pos < str.size() && str[pos] == '}') {
            pos++;
        }
        return val;
    }

    JsonValue parseArray(const std::string& str, size_t& pos) {
        JsonValue val;
        val.type = JsonType::Array;
        pos++;
        skipWhitespace(str, pos);

        while (pos < str.size() && str[pos] != ']') {
            skipWhitespace(str, pos);
            JsonValue elem = parseValue(str, pos);
            val.aVal.push_back(elem);

            skipWhitespace(str, pos);
            if (pos < str.size() && str[pos] == ',') {
                pos++;
            }
            skipWhitespace(str, pos);
        }
        if (pos < str.size() && str[pos] == ']') {
            pos++;
        }
        return val;
    }

    JsonValue parseNumber(const std::string& str, size_t& pos) {
        size_t start = pos;
        if (str[pos] == '-') pos++;
        while (pos < str.size() && ((str[pos] >= '0' && str[pos] <= '9') || str[pos] == '.' || str[pos] == 'e' || str[pos] == 'E' || str[pos] == '+' || str[pos] == '-')) {
            pos++;
        }
        std::string numStr = str.substr(start, pos - start);
        JsonValue val;
        val.type = JsonType::Number;
        try {
            val.nVal = std::stod(numStr);
        } catch (...) {
            val.nVal = 0.0;
        }
        return val;
    }

    JsonValue parseValue(const std::string& str, size_t& pos) {
        skipWhitespace(str, pos);
        if (pos >= str.size()) return JsonValue();

        char c = str[pos];
        if (c == '{') return parseObject(str, pos);
        if (c == '[') return parseArray(str, pos);
        if (c == '"') {
            JsonValue v;
            v.type = JsonType::String;
            v.sVal = parseString(str, pos);
            return v;
        }
        if (c == '-' || (c >= '0' && c <= '9')) return parseNumber(str, pos);
        if (str.compare(pos, 4, "true") == 0) {
            pos += 4;
            JsonValue v;
            v.type = JsonType::Bool;
            v.bVal = true;
            return v;
        }
        if (str.compare(pos, 5, "false") == 0) {
            pos += 5;
            JsonValue v;
            v.type = JsonType::Bool;
            v.bVal = false;
            return v;
        }
        if (str.compare(pos, 4, "null") == 0) {
            pos += 4;
            return JsonValue();
        }

        pos++;
        return JsonValue();
    }
} // anonymous namespace

LevelTimeline::LevelTimeline()
    : stageName("Sector 1"),
      musicTrack("stage1"),
      bossType("dreadnought"),
      isLoaded(false),
      bossTriggered(false) {}

bool LevelTimeline::LoadFromFile(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "[LevelTimeline] Failed to open stage file: " << filepath << std::endl;
        return false;
    }

    std::stringstream ss;
    ss << file.rdbuf();
    std::string content = ss.str();

    size_t pos = 0;
    JsonValue root = parseValue(content, pos);
    if (root.type != JsonType::Object) {
        std::cerr << "[LevelTimeline] Invalid JSON format in: " << filepath << std::endl;
        return false;
    }

    stageName = root["stage_name"].asString("Sector 1");
    musicTrack = root["music_track"].asString("stage1");
    bossType = root["boss_type"].asString("dreadnought");

    events.clear();
    const JsonValue& evArr = root["events"];
    if (evArr.type == JsonType::Array) {
        for (const auto& evObj : evArr.aVal) {
            TimelineEvent ev;
            ev.triggerZ = evObj["trigger_z"].asFloat(0.0f);
            ev.triggered = false;

            std::string typeStr = evObj["type"].asString();
            if (typeStr == "spawn_wave") {
                ev.type = TimelineEventType::SpawnWave;
                std::string et = evObj["enemy_type"].asString("drone");
                if (et == "tank") ev.enemyType = EnemyType::GroundTank;
                else if (et == "turret") ev.enemyType = EnemyType::FlakTurret;
                else if (et == "interceptor") ev.enemyType = EnemyType::EliteInterceptor;
                else ev.enemyType = EnemyType::Drone;

                ev.formation = evObj["formation"].asString("v_formation");
                ev.count = evObj["count"].asInt(1);
                ev.spawnX = evObj["spawn_x"].asFloat(0.0f);
                ev.spawnY = evObj["spawn_y"].asFloat(0.0f);
                ev.spawnZ = evObj["spawn_z"].asFloat(0.0f);
                ev.spacingX = evObj["spacing_x"].asFloat(4.5f);
                ev.spacingZ = evObj["spacing_z"].asFloat(12.0f);
            } else if (typeStr == "spawn_train") {
                ev.type = TimelineEventType::SpawnTrain;
                ev.trackX = evObj["track_x"].asFloat(22.0f);
            } else if (typeStr == "transmission") {
                ev.type = TimelineEventType::Transmission;
                std::string spk = evObj["speaker"].asString("Echo1");
                ev.speaker = (spk == "Echo2" || spk == "Aegis") ? WingmanID::Aegis : WingmanID::Striker;
                ev.line1 = evObj["line1"].asString();
                ev.line2 = evObj["line2"].asString();
                ev.duration = evObj["duration"].asFloat(4.0f);
            } else if (typeStr == "wingman_rescue") {
                ev.type = TimelineEventType::WingmanRescue;
                std::string spk = evObj["wingman"].asString("Striker");
                ev.rescueTarget = (spk == "Echo2" || spk == "Aegis") ? WingmanID::Aegis : WingmanID::Striker;
                ev.threatCount = evObj["threat_count"].asInt(3);
                ev.rescueTimeout = evObj["timeout"].asFloat(14.0f);
                ev.line1 = evObj["line1"].asString("Hostiles on my six! Need immediate backup!");
                ev.line2 = evObj["line2"].asString("Commander, get them off me before shields collapse!");
            } else if (typeStr == "collapsing_hazard") {
                ev.type = TimelineEventType::CollapsingHazard;
                ev.hazardX = evObj["hazard_x"].asFloat(-32.0f);
            } else if (typeStr == "boss_trigger") {
                ev.type = TimelineEventType::BossTrigger;
            } else if (typeStr == "clear_enemies") {
                ev.type = TimelineEventType::ClearEnemies;
            }

            events.push_back(ev);
        }
    }

    isLoaded = true;
    std::cout << "[LevelTimeline] Successfully loaded stage: " << stageName
              << " (" << events.size() << " scripted events, Boss: " << bossType << ")" << std::endl;
    return true;
}

void LevelTimeline::Reset() {
    bossTriggered = false;
    for (auto& ev : events) {
        ev.triggered = false;
    }
}

void LevelTimeline::Update(float playerZ, EnemyManager& enemies, WingmanSquadron* wingmen,
                           TrainConvoy* train, SoundManager* audio, WorldEnvironment* environment) {
    if (!isLoaded) return;

    for (auto& ev : events) {
        if (!ev.triggered && playerZ <= ev.triggerZ) {
            ev.triggered = true;

            switch (ev.type) {
                case TimelineEventType::SpawnWave: {
                    float sZ = (ev.spawnZ != 0.0f) ? ev.spawnZ : (playerZ - 220.0f);
                    enemies.SpawnCustomWave(ev.enemyType, ev.formation, ev.count,
                                           ev.spawnX, ev.spawnY, sZ,
                                           ev.spacingX, ev.spacingZ);
                    break;
                }

                case TimelineEventType::SpawnTrain: {
                    if (train) {
                        train->Spawn(playerZ, ev.trackX);
                    }
                    break;
                }

                case TimelineEventType::Transmission: {
                    if (wingmen) {
                        wingmen->TriggerTransmission(ev.speaker, ev.line1, ev.line2, ev.duration, audio);
                    }
                    break;
                }

                case TimelineEventType::WingmanRescue: {
                    if (wingmen) {
                        wingmen->TriggerScriptedRescue(ev.rescueTarget, ev.threatCount, ev.rescueTimeout,
                                                       ev.line1, ev.line2, enemies, audio);
                    }
                    break;
                }

                case TimelineEventType::CollapsingHazard: {
                    if (environment) {
                        environment->SpawnCollapsingSpire(ev.triggerZ, ev.hazardX);
                    }
                    if (wingmen) {
                        wingmen->TriggerTransmission(WingmanID::Striker,
                            "WATCH OUT! That communications tower is coming down!",
                            "Boost low under the beam or shoot the joint!", 3.8f, audio);
                    }
                    if (audio) {
                        audio->Play(SoundID::WarningSiren, 0.75f, 1.1f);
                    }
                    break;
                }

                case TimelineEventType::BossTrigger: {
                    bossTriggered = true;
                    break;
                }

                case TimelineEventType::ClearEnemies: {
                    enemies.Clear();
                    break;
                }
            }
        }
    }
}

std::vector<StageDefinition> LevelTimeline::GetStandardCampaignStages() {
    return {
        {
            "sector_1",
            "SECTOR 1: FALLEN CITY",
            "War-Torn Urban Ruins & Bipedal Walker Arena",
            "resource/stages/stage1.json",
            "IRON COLOSSUS",
            "walking_robot",
            "city",
            "NORMAL",
            glm::vec3(0.25f, 0.65f, 0.95f)
        },
        {
            "sector_2",
            "SECTOR 2: CANYON RAILWAY",
            "Red Rock Gorges & Armored Train Convoy",
            "resource/stages/stage2.json",
            "TWIN VIPER GUNSHIPS",
            "twin_helicopters",
            "railway_canyon",
            "HARD",
            glm::vec3(1.0f, 0.55f, 0.18f)
        },
        {
            "sector_3",
            "SECTOR 3: IRON FORTRESS",
            "Fortified Ground Citadel & Heavy Armor",
            "resource/stages/stage3.json",
            "GOLIATH MEGA-TANK",
            "mega_tank",
            "iron_fortress",
            "HARD",
            glm::vec3(0.95f, 0.28f, 0.25f)
        },
        {
            "sector_4",
            "SECTOR 4: FORBIDDEN DUNES",
            "Shifting Sands & Subterranean Leviathans",
            "resource/stages/stage4.json",
            "CYBERNETIC SANDWORM",
            "sandworm",
            "dune_pass",
            "EXPERT",
            glm::vec3(0.95f, 0.82f, 0.20f)
        },
        {
            "sector_5",
            "SECTOR 5: COSMIC DEBRIS",
            "Deep Space Asteroids & Elite Wings",
            "resource/stages/stage5.json",
            "STEALTH DREADNOUGHT",
            "dreadnought",
            "cosmic_debris",
            "EXPERT",
            glm::vec3(0.35f, 0.65f, 1.0f)
        }
    };
}

