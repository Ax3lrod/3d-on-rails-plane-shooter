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
#ifdef DrawText
#undef DrawText
#endif
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
    WingRepair,
    RadioStatic,
    RadioChatter
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

enum class BGMTrack {
    None = 0,
    Title,
    Stage1,
    Boss,
    Sector2
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

    void PlayBGM(BGMTrack track, float volume = 0.70f);
    void StopBGM();
    BGMTrack GetCurrentBGM() const { return currentBgm; }

    void SetMasterVolume(float vol);
    void SetMusicVolume(float vol);
    void SetSFXVolume(float vol);
    float GetMasterVolume() const { return masterVolume.load(); }
    float GetMusicVolume() const { return musicVolume.load(); }
    float GetSFXVolume() const { return sfxVolume.load(); }

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
    void GenerateRadioStaticSound();
    void GenerateRadioChatterSound();

    void GenerateTitleBGM();
    void GenerateStage1BGM();
    void GenerateBossBGM();
    void GenerateSector2BGM();

    void AudioThreadFunc();

    static const int SAMPLE_RATE = 44100;
    static const int BUFFER_SAMPLES = 2048;
    static const int BUFFER_COUNT = 2;
    static const int MAX_VOICES = 32;

    std::vector<SoundSample> soundBank;
    std::vector<SoundSample> musicBank;
    Voice voices[MAX_VOICES];
    Voice musicVoice;
    BGMTrack currentBgm;
    mutable std::mutex voiceMutex;

    std::atomic<bool> isRunning;
    std::atomic<float> masterVolume;
    std::atomic<float> musicVolume;
    std::atomic<float> sfxVolume;
    std::thread audioThread;

#ifdef _WIN32
    HWAVEOUT hWaveOut;
    WAVEHDR waveHeaders[BUFFER_COUNT];
    int16_t audioBuffers[BUFFER_COUNT][BUFFER_SAMPLES];
    HANDLE audioEvent;
#endif
};

#endif

