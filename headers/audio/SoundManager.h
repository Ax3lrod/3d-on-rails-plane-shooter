#ifndef SOUND_MANAGER_H
#define SOUND_MANAGER_H

#include <vector>
#include <memory>
#include <mutex>
#include <thread>
#include <atomic>
#include <cstdint>

#ifdef _WIN32
#include <windows.h>
#include <mmsystem.h>
#endif

enum class SoundID {
    LaserFire,
    EnemyLaser,
    ChargeHum,
    LockOnPing,
    ChargedShotFire,
    BombLaunch,
    BombExplosion,
    BarrelRoll,
    BoostRoar,
    BrakeHiss,
    ExplosionSmall,
    ExplosionLarge,
    RingCollect,
    WarningSiren,
    VictoryFanfare,
    WingSnap,
    WingRepair
};

struct SoundSample {
    std::vector<int16_t> pcmData;
    int sampleRate;
};

struct Voice {
    const int16_t* samples;
    size_t sampleCount;
    float position;
    float volume;
    float pitch;
    bool loop;
    bool active;
    SoundID soundId;
};

class SoundManager {
public:
    SoundManager();
    ~SoundManager();

    bool Init();
    void Shutdown();

    void Play(SoundID id, float volume = 1.0f, float pitch = 1.0f);
    void PlayLoop(SoundID id, float volume = 1.0f, float pitch = 1.0f);
    void StopLoop(SoundID id);
    bool IsLoopPlaying(SoundID id) const;
    void SetMasterVolume(float vol);

private:
    void PrebakeSounds();
    void GenerateLaserSound();
    void GenerateEnemyLaserSound();
    void GenerateChargeHumSound();
    void GenerateLockOnPingSound();
    void GenerateChargedShotFireSound();
    void GenerateBombLaunchSound();
    void GenerateBombExplosionSound();
    void GenerateBarrelRollSound();
    void GenerateBoostRoarSound();
    void GenerateBrakeHissSound();
    void GenerateExplosionSmallSound();
    void GenerateExplosionLargeSound();
    void GenerateRingCollectSound();
    void GenerateWarningSirenSound();
    void GenerateVictoryFanfareSound();
    void GenerateWingSnapSound();
    void GenerateWingRepairSound();

    void AudioThreadFunc();

    static const int SAMPLE_RATE = 44100;
    static const int BUFFER_SAMPLES = 2048;
    static const int BUFFER_COUNT = 2;
    static const int MAX_VOICES = 32;

    std::vector<SoundSample> soundBank;
    Voice voices[MAX_VOICES];
    mutable std::mutex voiceMutex;

    std::atomic<bool> isRunning;
    std::atomic<float> masterVolume;
    std::thread audioThread;

#ifdef _WIN32
    HWAVEOUT hWaveOut;
    WAVEHDR waveHeaders[BUFFER_COUNT];
    int16_t audioBuffers[BUFFER_COUNT][BUFFER_SAMPLES];
    HANDLE audioEvent;
#endif
};

#endif
