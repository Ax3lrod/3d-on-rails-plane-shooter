#include "SoundManager.h"
#include <cmath>
#include <cstdlib>
#include <algorithm>
#include <iostream>

static const float PI = 3.14159265358979323846f;

static inline int16_t ClampSample(float x) {
    if (x > 32767.0f) return 32767;
    if (x < -32767.0f) return -32767;
    return static_cast<int16_t>(x);
}

static inline float RandFloat() {
    return (static_cast<float>(rand()) / static_cast<float>(RAND_MAX)) * 2.0f - 1.0f;
}

SoundManager::SoundManager()
    : isRunning(false),
      masterVolume(0.85f)
#ifdef _WIN32
      , hWaveOut(nullptr),
      audioEvent(nullptr)
#endif
{
    for (int i = 0; i < MAX_VOICES; ++i) {
        voices[i].active = false;
        voices[i].samples = nullptr;
        voices[i].sampleCount = 0;
        voices[i].position = 0.0f;
        voices[i].volume = 1.0f;
        voices[i].pitch = 1.0f;
        voices[i].loop = false;
    }
}

SoundManager::~SoundManager() {
    Shutdown();
}

bool SoundManager::Init() {
    PrebakeSounds();

#ifdef _WIN32
    WAVEFORMATEX wfx;
    wfx.wFormatTag = WAVE_FORMAT_PCM;
    wfx.nChannels = 1; // Mono output
    wfx.nSamplesPerSec = SAMPLE_RATE;
    wfx.wBitsPerSample = 16;
    wfx.nBlockAlign = (wfx.nChannels * wfx.wBitsPerSample) / 8;
    wfx.nAvgBytesPerSec = wfx.nSamplesPerSec * wfx.nBlockAlign;
    wfx.cbSize = 0;

    audioEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
    if (!audioEvent) {
        std::cerr << "[Audio] Failed to create audio event handle.\n";
        return false;
    }

    MMRESULT result = waveOutOpen(&hWaveOut, WAVE_MAPPER, &wfx,
                                  reinterpret_cast<DWORD_PTR>(audioEvent), 0, CALLBACK_EVENT);
    if (result != MMSYSERR_NOERROR) {
        std::cerr << "[Audio] Failed to open waveOut device (code " << result << ").\n";
        CloseHandle(audioEvent);
        audioEvent = nullptr;
        return false;
    }

    for (int i = 0; i < BUFFER_COUNT; ++i) {
        std::fill_n(audioBuffers[i], BUFFER_SAMPLES, static_cast<int16_t>(0));
        ZeroMemory(&waveHeaders[i], sizeof(WAVEHDR));
        waveHeaders[i].lpData = reinterpret_cast<LPSTR>(audioBuffers[i]);
        waveHeaders[i].dwBufferLength = BUFFER_SAMPLES * sizeof(int16_t);
        waveHeaders[i].dwFlags = 0;

        waveOutPrepareHeader(hWaveOut, &waveHeaders[i], sizeof(WAVEHDR));
        waveOutWrite(hWaveOut, &waveHeaders[i], sizeof(WAVEHDR));
    }

    isRunning = true;
    audioThread = std::thread(&SoundManager::AudioThreadFunc, this);
    std::cout << "[Audio] SoundManager initialized (44.1kHz 16-bit PCM multi-channel mixer).\n";
    return true;
#else
    return true;
#endif
}

void SoundManager::Shutdown() {
    if (!isRunning) return;
    isRunning = false;

#ifdef _WIN32
    if (audioEvent) {
        SetEvent(audioEvent);
    }

    if (audioThread.joinable()) {
        audioThread.join();
    }

    if (hWaveOut) {
        waveOutReset(hWaveOut);
        for (int i = 0; i < BUFFER_COUNT; ++i) {
            waveOutUnprepareHeader(hWaveOut, &waveHeaders[i], sizeof(WAVEHDR));
        }
        waveOutClose(hWaveOut);
        hWaveOut = nullptr;
    }

    if (audioEvent) {
        CloseHandle(audioEvent);
        audioEvent = nullptr;
    }
#endif
}

void SoundManager::Play(SoundID id, float volume, float pitch) {
    size_t soundIndex = static_cast<size_t>(id);
    if (soundIndex >= soundBank.size() || soundBank[soundIndex].pcmData.empty()) return;

    std::lock_guard<std::mutex> lock(voiceMutex);

    // Find free voice or steal oldest active one-shot voice
    int bestVoice = -1;
    for (int i = 0; i < MAX_VOICES; ++i) {
        if (!voices[i].active) {
            bestVoice = i;
            break;
        }
    }

    if (bestVoice == -1) {
        // Find a non-looping voice to overwrite
        for (int i = 0; i < MAX_VOICES; ++i) {
            if (!voices[i].loop) {
                bestVoice = i;
                break;
            }
        }
    }

    if (bestVoice != -1) {
        voices[bestVoice].samples = soundBank[soundIndex].pcmData.data();
        voices[bestVoice].sampleCount = soundBank[soundIndex].pcmData.size();
        voices[bestVoice].position = 0.0f;
        voices[bestVoice].volume = volume;
        voices[bestVoice].pitch = std::clamp(pitch, 0.2f, 3.0f);
        voices[bestVoice].loop = false;
        voices[bestVoice].soundId = id;
        voices[bestVoice].active = true;
    }
}

void SoundManager::PlayLoop(SoundID id, float volume, float pitch) {
    size_t soundIndex = static_cast<size_t>(id);
    if (soundIndex >= soundBank.size() || soundBank[soundIndex].pcmData.empty()) return;

    std::lock_guard<std::mutex> lock(voiceMutex);

    // If already looping, just update volume/pitch
    for (int i = 0; i < MAX_VOICES; ++i) {
        if (voices[i].active && voices[i].loop && voices[i].soundId == id) {
            voices[i].volume = volume;
            voices[i].pitch = pitch;
            return;
        }
    }

    // Otherwise assign free voice
    for (int i = 0; i < MAX_VOICES; ++i) {
        if (!voices[i].active) {
            voices[i].samples = soundBank[soundIndex].pcmData.data();
            voices[i].sampleCount = soundBank[soundIndex].pcmData.size();
            voices[i].position = 0.0f;
            voices[i].volume = volume;
            voices[i].pitch = pitch;
            voices[i].loop = true;
            voices[i].soundId = id;
            voices[i].active = true;
            break;
        }
    }
}

void SoundManager::StopLoop(SoundID id) {
    std::lock_guard<std::mutex> lock(voiceMutex);
    for (int i = 0; i < MAX_VOICES; ++i) {
        if (voices[i].active && voices[i].loop && voices[i].soundId == id) {
            voices[i].active = false;
        }
    }
}

bool SoundManager::IsLoopPlaying(SoundID id) const {
    std::lock_guard<std::mutex> lock(voiceMutex);
    for (int i = 0; i < MAX_VOICES; ++i) {
        if (voices[i].active && voices[i].loop && voices[i].soundId == id) {
            return true;
        }
    }
    return false;
}

void SoundManager::SetMasterVolume(float vol) {
    masterVolume = std::clamp(vol, 0.0f, 1.0f);
}

void SoundManager::AudioThreadFunc() {
#ifdef _WIN32
    float mixBuffer[BUFFER_SAMPLES];

    while (isRunning) {
        DWORD waitResult = WaitForSingleObject(audioEvent, 50);
        if (!isRunning) break;

        for (int b = 0; b < BUFFER_COUNT; ++b) {
            if (waveHeaders[b].dwFlags & WHDR_DONE) {
                std::fill_n(mixBuffer, BUFFER_SAMPLES, 0.0f);

                {
                    std::lock_guard<std::mutex> lock(voiceMutex);

                    for (int v = 0; v < MAX_VOICES; ++v) {
                        if (!voices[v].active) continue;

                        float voiceVol = voices[v].volume * masterVolume.load() * 32767.0f;
                        float pitch = voices[v].pitch;
                        const int16_t* pcm = voices[v].samples;
                        size_t maxSamples = voices[v].sampleCount;

                        for (int s = 0; s < BUFFER_SAMPLES; ++s) {
                            size_t idx = static_cast<size_t>(voices[v].position);
                            if (idx >= maxSamples) {
                                if (voices[v].loop) {
                                    voices[v].position = std::fmod(voices[v].position, static_cast<float>(maxSamples));
                                    idx = static_cast<size_t>(voices[v].position);
                                } else {
                                    voices[v].active = false;
                                    break;
                                }
                            }

                            float normSample = pcm[idx] / 32767.0f;
                            mixBuffer[s] += normSample * voiceVol;
                            voices[v].position += pitch;
                        }
                    }
                }

                // Soft saturation clamp into buffer
                for (int s = 0; s < BUFFER_SAMPLES; ++s) {
                    float val = mixBuffer[s];
                    if (val > 30000.0f) val = 30000.0f + (val - 30000.0f) * 0.25f;
                    else if (val < -30000.0f) val = -30000.0f + (val + 30000.0f) * 0.25f;
                    audioBuffers[b][s] = ClampSample(val);
                }

                waveOutWrite(hWaveOut, &waveHeaders[b], sizeof(WAVEHDR));
            }
        }
    }
#endif
}

// =========================================================================
// PROCEDURAL DSP SYNTHESIS ALGORITHMS
// =========================================================================

void SoundManager::PrebakeSounds() {
    soundBank.resize(17);
    GenerateLaserSound();
    GenerateEnemyLaserSound();
    GenerateChargeHumSound();
    GenerateLockOnPingSound();
    GenerateChargedShotFireSound();
    GenerateBombLaunchSound();
    GenerateBombExplosionSound();
    GenerateBarrelRollSound();
    GenerateBoostRoarSound();
    GenerateBrakeHissSound();
    GenerateExplosionSmallSound();
    GenerateExplosionLargeSound();
    GenerateRingCollectSound();
    GenerateWarningSirenSound();
    GenerateVictoryFanfareSound();
    GenerateWingSnapSound();
    GenerateWingRepairSound();
}

// 1. Dual Laser Fire (Crisp Star Fox square/saw downward sweep)
void SoundManager::GenerateLaserSound() {
    float duration = 0.12f;
    size_t count = static_cast<size_t>(SAMPLE_RATE * duration);
    std::vector<int16_t>& pcm = soundBank[static_cast<size_t>(SoundID::LaserFire)].pcmData;
    pcm.resize(count);

    float phase = 0.0f;
    for (size_t i = 0; i < count; ++i) {
        float t = static_cast<float>(i) / SAMPLE_RATE;
        float progress = t / duration;

        // Exponential frequency drop from 1800Hz to 280Hz
        float freq = 1800.0f * std::pow(0.15f, progress);
        phase += 2.0f * PI * freq / SAMPLE_RATE;

        // 65% square + 35% saw wave
        float sq = (std::sin(phase) >= 0.0f) ? 1.0f : -1.0f;
        float saw = 2.0f * (phase / (2.0f * PI) - std::floor(0.5f + phase / (2.0f * PI)));
        float wave = sq * 0.65f + saw * 0.35f;

        // Envelope: instant attack, exponential drop
        float env = std::pow(1.0f - progress, 1.6f);
        pcm[i] = ClampSample(wave * env * 24000.0f);
    }
}

// 2. Enemy Laser (Alien plasma buzz sweep)
void SoundManager::GenerateEnemyLaserSound() {
    float duration = 0.16f;
    size_t count = static_cast<size_t>(SAMPLE_RATE * duration);
    std::vector<int16_t>& pcm = soundBank[static_cast<size_t>(SoundID::EnemyLaser)].pcmData;
    pcm.resize(count);

    float phase = 0.0f;
    for (size_t i = 0; i < count; ++i) {
        float t = static_cast<float>(i) / SAMPLE_RATE;
        float progress = t / duration;

        float freq = 950.0f * std::pow(0.20f, progress);
        phase += 2.0f * PI * freq / SAMPLE_RATE;

        float saw = 2.0f * (phase / (2.0f * PI) - std::floor(0.5f + phase / (2.0f * PI)));
        float tremolo = 0.8f + 0.2f * std::sin(2.0f * PI * 40.0f * t);
        float env = std::pow(1.0f - progress, 1.4f);

        pcm[i] = ClampSample(saw * tremolo * env * 22000.0f);
    }
}

// 3. Charge Hum (Pulsing plasma reactor loop)
void SoundManager::GenerateChargeHumSound() {
    float duration = 0.50f;
    size_t count = static_cast<size_t>(SAMPLE_RATE * duration);
    std::vector<int16_t>& pcm = soundBank[static_cast<size_t>(SoundID::ChargeHum)].pcmData;
    pcm.resize(count);

    for (size_t i = 0; i < count; ++i) {
        float t = static_cast<float>(i) / SAMPLE_RATE;
        float baseFreq = 340.0f;
        float vibrato = 15.0f * std::sin(2.0f * PI * 14.0f * t);

        float s1 = std::sin(2.0f * PI * (baseFreq + vibrato) * t);
        float s2 = std::sin(2.0f * PI * (baseFreq * 2.0f + vibrato) * t) * 0.4f;
        float tremolo = 0.75f + 0.25f * std::sin(2.0f * PI * 14.0f * t);

        pcm[i] = ClampSample((s1 + s2) * tremolo * 18000.0f);
    }
}

// 4. Lock-On Ping (High-frequency crystal target acquired chime)
void SoundManager::GenerateLockOnPingSound() {
    float duration = 0.24f;
    size_t count = static_cast<size_t>(SAMPLE_RATE * duration);
    std::vector<int16_t>& pcm = soundBank[static_cast<size_t>(SoundID::LockOnPing)].pcmData;
    pcm.resize(count);

    for (size_t i = 0; i < count; ++i) {
        float t = static_cast<float>(i) / SAMPLE_RATE;
        float progress = t / duration;

        float c1 = std::sin(2.0f * PI * 1760.0f * t); // A6
        float c2 = std::sin(2.0f * PI * 2640.0f * t) * 0.65f; // E7
        float env = std::pow(1.0f - progress, 2.8f);

        pcm[i] = ClampSample((c1 + c2) * env * 23000.0f);
    }
}

// 5. Charged Shot Fire (Heavy plasma torpedo discharge)
void SoundManager::GenerateChargedShotFireSound() {
    float duration = 0.38f;
    size_t count = static_cast<size_t>(SAMPLE_RATE * duration);
    std::vector<int16_t>& pcm = soundBank[static_cast<size_t>(SoundID::ChargedShotFire)].pcmData;
    pcm.resize(count);

    float phase = 0.0f;
    for (size_t i = 0; i < count; ++i) {
        float t = static_cast<float>(i) / SAMPLE_RATE;
        float progress = t / duration;

        float freq = 560.0f * std::pow(0.12f, progress);
        phase += 2.0f * PI * freq / SAMPLE_RATE;

        float bass = std::sin(phase);
        float sub = std::sin(phase * 0.5f) * 0.5f;
        float env = std::pow(1.0f - progress, 1.5f);

        pcm[i] = ClampSample((bass + sub) * env * 28000.0f);
    }
}

// 6. Smart Bomb Launch (Ascending whistle)
void SoundManager::GenerateBombLaunchSound() {
    float duration = 0.28f;
    size_t count = static_cast<size_t>(SAMPLE_RATE * duration);
    std::vector<int16_t>& pcm = soundBank[static_cast<size_t>(SoundID::BombLaunch)].pcmData;
    pcm.resize(count);

    float phase = 0.0f;
    for (size_t i = 0; i < count; ++i) {
        float t = static_cast<float>(i) / SAMPLE_RATE;
        float progress = t / duration;

        float freq = 320.0f + 1100.0f * std::pow(progress, 1.4f);
        phase += 2.0f * PI * freq / SAMPLE_RATE;

        float s = std::sin(phase);
        float env = (progress < 0.2f) ? (progress / 0.2f) : (1.0f - (progress - 0.2f) / 0.8f);

        pcm[i] = ClampSample(s * env * 20000.0f);
    }
}

// 7. Smart Bomb Explosion (Colossal sub-bass shockwave and rolling rumble)
void SoundManager::GenerateBombExplosionSound() {
    float duration = 2.4f;
    size_t count = static_cast<size_t>(SAMPLE_RATE * duration);
    std::vector<int16_t>& pcm = soundBank[static_cast<size_t>(SoundID::BombExplosion)].pcmData;
    pcm.resize(count);

    float phase = 0.0f;
    float lpfNoise = 0.0f;

    for (size_t i = 0; i < count; ++i) {
        float t = static_cast<float>(i) / SAMPLE_RATE;
        float progress = t / duration;

        // Sub-bass sine drop 65Hz -> 20Hz
        float freq = 65.0f * std::pow(0.30f, progress);
        phase += 2.0f * PI * freq / SAMPLE_RATE;
        float sub = std::sin(phase);

        // Low-pass filtered noise
        float rawNoise = RandFloat();
        float alpha = 0.08f + 0.15f * std::pow(1.0f - progress, 2.0f);
        lpfNoise += alpha * (rawNoise - lpfNoise);

        float env = std::pow(1.0f - progress, 1.8f);
        float total = (sub * 0.55f + lpfNoise * 0.75f) * env;

        pcm[i] = ClampSample(total * 30000.0f);
    }
}

// 8. Barrel Roll Evasion (Aerodynamic wind shear whoosh)
void SoundManager::GenerateBarrelRollSound() {
    float duration = 0.45f;
    size_t count = static_cast<size_t>(SAMPLE_RATE * duration);
    std::vector<int16_t>& pcm = soundBank[static_cast<size_t>(SoundID::BarrelRoll)].pcmData;
    pcm.resize(count);

    float bpVal = 0.0f;
    for (size_t i = 0; i < count; ++i) {
        float t = static_cast<float>(i) / SAMPLE_RATE;
        float progress = t / duration;

        // Bell curve envelope: rises to 1.0 at midpoint then decays
        float env = std::sin(progress * PI);

        // Filtered whoosh noise
        float rawNoise = RandFloat();
        float sweepCoeff = 0.12f + 0.30f * env;
        bpVal += sweepCoeff * (rawNoise - bpVal);

        pcm[i] = ClampSample(bpVal * env * 24000.0f);
    }
}

// 9. Boost Roar (Seamless thruster burn loop)
void SoundManager::GenerateBoostRoarSound() {
    float duration = 0.60f;
    size_t count = static_cast<size_t>(SAMPLE_RATE * duration);
    std::vector<int16_t>& pcm = soundBank[static_cast<size_t>(SoundID::BoostRoar)].pcmData;
    pcm.resize(count);

    float lpf = 0.0f;
    for (size_t i = 0; i < count; ++i) {
        float t = static_cast<float>(i) / SAMPLE_RATE;
        float rawNoise = RandFloat();
        lpf += 0.15f * (rawNoise - lpf);

        float rumble = std::sin(2.0f * PI * 58.0f * t) * 0.45f;
        pcm[i] = ClampSample((lpf * 0.6f + rumble) * 19000.0f);
    }
}

// 10. Brake Hiss (Airbrake deceleration loop)
void SoundManager::GenerateBrakeHissSound() {
    float duration = 0.50f;
    size_t count = static_cast<size_t>(SAMPLE_RATE * duration);
    std::vector<int16_t>& pcm = soundBank[static_cast<size_t>(SoundID::BrakeHiss)].pcmData;
    pcm.resize(count);

    float prev = 0.0f;
    for (size_t i = 0; i < count; ++i) {
        float rawNoise = RandFloat();
        // High-pass filter
        float hp = rawNoise - prev;
        prev = rawNoise * 0.85f;

        pcm[i] = ClampSample(hp * 14000.0f);
    }
}

// 11. Small Explosion (Enemy drone burst)
void SoundManager::GenerateExplosionSmallSound() {
    float duration = 0.35f;
    size_t count = static_cast<size_t>(SAMPLE_RATE * duration);
    std::vector<int16_t>& pcm = soundBank[static_cast<size_t>(SoundID::ExplosionSmall)].pcmData;
    pcm.resize(count);

    float lpf = 0.0f;
    for (size_t i = 0; i < count; ++i) {
        float t = static_cast<float>(i) / SAMPLE_RATE;
        float progress = t / duration;

        float raw = RandFloat();
        lpf += 0.25f * (raw - lpf);

        float thud = std::sin(2.0f * PI * (120.0f * (1.0f - progress)) * t) * 0.5f;
        float env = std::pow(1.0f - progress, 2.0f);

        pcm[i] = ClampSample((lpf + thud) * env * 25000.0f);
    }
}

// 12. Large Explosion (Hazard pillar / Asteroid / Boss part)
void SoundManager::GenerateExplosionLargeSound() {
    float duration = 0.85f;
    size_t count = static_cast<size_t>(SAMPLE_RATE * duration);
    std::vector<int16_t>& pcm = soundBank[static_cast<size_t>(SoundID::ExplosionLarge)].pcmData;
    pcm.resize(count);

    float lpf = 0.0f;
    float phase = 0.0f;
    for (size_t i = 0; i < count; ++i) {
        float t = static_cast<float>(i) / SAMPLE_RATE;
        float progress = t / duration;

        float f = 80.0f * std::pow(0.35f, progress);
        phase += 2.0f * PI * f / SAMPLE_RATE;
        float sub = std::sin(phase);

        float raw = RandFloat();
        lpf += 0.18f * (raw - lpf);

        float env = std::pow(1.0f - progress, 1.6f);
        pcm[i] = ClampSample((sub * 0.6f + lpf * 0.7f) * env * 28000.0f);
    }
}

// 13. Ring Collect (Pure ascending 4-note arpeggio: C6, E6, G6, C7)
void SoundManager::GenerateRingCollectSound() {
    float duration = 0.36f;
    size_t count = static_cast<size_t>(SAMPLE_RATE * duration);
    std::vector<int16_t>& pcm = soundBank[static_cast<size_t>(SoundID::RingCollect)].pcmData;
    pcm.resize(count);

    float notes[4] = {1046.50f, 1318.51f, 1567.98f, 2093.00f};
    float noteDur = duration / 4.0f;

    for (size_t i = 0; i < count; ++i) {
        float t = static_cast<float>(i) / SAMPLE_RATE;
        int noteIdx = std::clamp(static_cast<int>(t / noteDur), 0, 3);
        float noteTime = std::fmod(t, noteDur);

        float freq = notes[noteIdx];
        float s = std::sin(2.0f * PI * freq * t);
        float noteEnv = std::pow(1.0f - (noteTime / noteDur), 1.2f);

        pcm[i] = ClampSample(s * noteEnv * 20000.0f);
    }
}

// 14. Warning Siren (Emergency battleship klaxon)
void SoundManager::GenerateWarningSirenSound() {
    float duration = 0.70f;
    size_t count = static_cast<size_t>(SAMPLE_RATE * duration);
    std::vector<int16_t>& pcm = soundBank[static_cast<size_t>(SoundID::WarningSiren)].pcmData;
    pcm.resize(count);

    for (size_t i = 0; i < count; ++i) {
        float t = static_cast<float>(i) / SAMPLE_RATE;
        float freq = (t < 0.35f) ? 880.0f : 660.0f;

        float s = (std::sin(2.0f * PI * freq * t) >= 0.0f) ? 0.85f : -0.85f;
        pcm[i] = ClampSample(s * 17000.0f);
    }
}

// 15. Victory Fanfare (Triumphant 5-note brass fanfare)
void SoundManager::GenerateVictoryFanfareSound() {
    float duration = 1.50f;
    size_t count = static_cast<size_t>(SAMPLE_RATE * duration);
    std::vector<int16_t>& pcm = soundBank[static_cast<size_t>(SoundID::VictoryFanfare)].pcmData;
    pcm.resize(count);

    // C5, E5, G5, B5, C6 (sustained)
    float notes[5] = {523.25f, 659.25f, 783.99f, 987.77f, 1046.50f};
    float durations[5] = {0.18f, 0.18f, 0.18f, 0.18f, 0.78f};

    float elapsed = 0.0f;
    int currentNote = 0;

    for (size_t i = 0; i < count; ++i) {
        float t = static_cast<float>(i) / SAMPLE_RATE;

        if (currentNote < 4 && t > elapsed + durations[currentNote]) {
            elapsed += durations[currentNote];
            currentNote++;
        }

        float noteTime = t - elapsed;
        float freq = notes[currentNote];

        // Trumpet/brass timbre (fundamental + harmonics)
        float s1 = std::sin(2.0f * PI * freq * t);
        float s2 = std::sin(2.0f * PI * (freq * 2.0f) * t) * 0.45f;
        float s3 = std::sin(2.0f * PI * (freq * 3.0f) * t) * 0.25f;

        float noteEnv = (currentNote == 4)
                        ? std::pow(1.0f - (noteTime / durations[currentNote]), 1.1f)
                        : (1.0f - 0.3f * (noteTime / durations[currentNote]));

        pcm[i] = ClampSample((s1 + s2 + s3) * noteEnv * 20000.0f);
    }
}

// 16. Wing Snap (Crunchy metal stress, shear, and detachment snap)
void SoundManager::GenerateWingSnapSound() {
    float duration = 0.42f;
    size_t count = static_cast<size_t>(SAMPLE_RATE * duration);
    std::vector<int16_t>& pcm = soundBank[static_cast<size_t>(SoundID::WingSnap)].pcmData;
    pcm.resize(count);

    float noiseFilter = 0.0f;
    float phase = 0.0f;
    float subPhase = 0.0f;

    for (size_t i = 0; i < count; ++i) {
        float t = static_cast<float>(i) / SAMPLE_RATE;
        float progress = t / duration;

        // White noise transient for the shear crack
        float rawNoise = (static_cast<float>(rand()) / RAND_MAX) * 2.0f - 1.0f;
        noiseFilter += 0.35f * (rawNoise - noiseFilter);

        // Low resonant groan / metallic stress tone
        float stressFreq = 160.0f * (1.0f - progress * 0.7f);
        phase += 2.0f * PI * stressFreq / SAMPLE_RATE;
        float metalGroan = (std::sin(phase) >= 0.0f ? 0.7f : -0.7f);

        // Sub-bass impact thud
        subPhase += 2.0f * PI * (75.0f * (1.0f - progress)) / SAMPLE_RATE;
        float subThud = std::sin(subPhase);

        float snapEnv = (progress < 0.08f) ? (progress / 0.08f) : std::pow(1.0f - (progress - 0.08f) / 0.92f, 2.0f);
        float combined = (noiseFilter * 0.6f + metalGroan * 0.3f + subThud * 0.4f) * snapEnv;

        pcm[i] = ClampSample(combined * 27000.0f);
    }
}

// 17. Wing Repair (High-tech holographic reassembly chirp & confirmation chime)
void SoundManager::GenerateWingRepairSound() {
    float duration = 0.55f;
    size_t count = static_cast<size_t>(SAMPLE_RATE * duration);
    std::vector<int16_t>& pcm = soundBank[static_cast<size_t>(SoundID::WingRepair)].pcmData;
    pcm.resize(count);

    float phase1 = 0.0f;
    float phase2 = 0.0f;
    float phaseChime = 0.0f;

    for (size_t i = 0; i < count; ++i) {
        float t = static_cast<float>(i) / SAMPLE_RATE;
        float progress = t / duration;

        // Phase 1: Rising holographic frequency sweep
        float freq1 = 520.0f + 1200.0f * std::pow(progress, 1.2f);
        phase1 += 2.0f * PI * freq1 / SAMPLE_RATE;
        float sweep = std::sin(phase1);

        // Phase 2: Shimmering octave harmonic
        phase2 += 2.0f * PI * (freq1 * 1.5f) / SAMPLE_RATE;
        float harmonic = std::sin(phase2) * 0.4f;

        // Final crystalline lock chime during second half
        float chime = 0.0f;
        if (progress > 0.45f) {
            float chimeProgress = (progress - 0.45f) / 0.55f;
            phaseChime += 2.0f * PI * 1318.5f / SAMPLE_RATE; // E6
            chime = std::sin(phaseChime) * std::pow(1.0f - chimeProgress, 1.5f) * 0.7f;
        }

        float env = (progress < 0.1f) ? (progress / 0.1f) : std::pow(1.0f - (progress - 0.1f) / 0.9f, 1.2f);
        float out = (sweep * 0.5f + harmonic * 0.3f + chime * 0.6f) * env;

        pcm[i] = ClampSample(out * 23000.0f);
    }
}
