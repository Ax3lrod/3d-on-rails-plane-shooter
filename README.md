# 🚀 Aegis Starfighter — 3D Rail-Shooter Engine

[![C++17](https://img.shields.io/badge/C%2B%2B-17-blue.svg)](https://en.cppreference.com/w/cpp/17)
[![OpenGL](https://img.shields.io/badge/OpenGL-3.3%20Core-green.svg)](https://www.opengl.org/)
[![License](https://img.shields.io/badge/License-MIT-purple.svg)](LICENSE)
[![Platform](https://img.shields.io/badge/Platform-Windows%20%7C%20Linux%20%7C%20macOS-lightgrey.svg)]()

> A high-octane 3D arcade rail-shooter game engine inspired by the iconic mechanics of ***Star Fox 64*** and the vibrant retro aesthetic of ***Ex-Zodiac***. Built from scratch in pure modern C++17 and OpenGL 3.3 Core with zero heavyweight external engine bloat.

---

## 🌟 Key Features

- **Precision Corridor Flight & Dogfighting**:
  - Snappy analog flight physics with dynamic banking, lateral inertia, and screen corner swooping.
  - Tactical flight maneuvers: **Boost**, **Airbrake**, **Barrel Roll** (projectile deflection), **Somersault** (loop-de-loop), and **U-Turn**.
  - **Modular Wing Damage Model**: Collisions shear off left or right wings, inducing asymmetric aerodynamic drag and single-blaster fallback until repaired by Silver Rings.
  - **Seamless All-Range Mode**: 360° free-flight arena transitions during colossal boss encounters with boundary radar and turn-back alerts.

- **Deep Combat & Arsenal**:
  - Dual plasma blaster cannons with rapid-fire convergence targeting.
  - **Charged Lock-On Shot**: Charges an energy vortex at the ship's nose, locks onto enemy bogeys or boss weakpoints, and unleashes area-of-effect splash damage.
  - **Smart Bomb**: Screen-clearing torpedo with expanding shockwave ring that obliterates incoming projectiles and damages surrounding waves.
  - **Ex-Zodiac Inspired Hit Combos**: Hit multipliers (`HIT x3`, `x5`, `COMBO!`) and score reward chimes.

- **Choreographed 5-Act Missions & Bosses**:
  - **Stage 1 (Fallen City)**: A 10-minute urban warzone featuring brutalist skyscrapers, roadside tanks, rooftop flak turrets, interactive wingman distress rescues, and a collapsing highway antenna spire set piece.
  - **Multi-Part Boss Tactics**:
    - **Iron Colossus (`BossWalkingRobot`)**: Granga-inspired bipedal walker. Shatter knee joints to induce staggers and expose the glowing rear coolant port for **4.0x critical damage**.
    - **Colossal Dreadnought (`BossDreadnought`)**: Massive warship with spinning spiked flail arms, shield generators, and bullet-hell core barrages.
    - **Twin Attack Gunships (`BossTwinHelicopters`)**, **Armored Mega-Tank (`BossMegaTank`)**, and **Burrowing Mecha-Worm (`BossMechaWorm`)**.

- **Retro Visuals & Software Synthesizer**:
  - Flat-faceted arcade polygon rendering with custom 4x4 Bayer dithering and distance fog.
  - **Post-Processing Pipeline**: Bloom glow extraction, chromatic aberration, and optional retro CRT scanline filter (`F1` toggle).
  - **First-Person Cockpit View (`V` key)**: Authentic interior canopy struts, artificial pitch horizon, and HUD canopy glare.
  - **Procedural Audio Synthesizer**: Zero external audio files required! Synthesizes multi-channel arcade chiptune music, dynamic boss BGM, weapon sounds, and 16-bit radio static speech babble in real time.
  - **Controller Vibration**: Full XInput controller rumble and haptic feedback on blaster fire, boost roar, and explosions.

---

## 🎮 Controls

The engine supports seamless hot-plugging between **Keyboard** and **Gamepad** (Xbox / DualShock / DirectInput).

| Action | Keyboard | Gamepad (Xbox / Generic) |
| :--- | :--- | :--- |
| **Pitch & Steering** | `W` / `A` / `S` / `D` or Arrow Keys | Left Analog Stick / D-Pad |
| **Primary Blasters** | `Space` or `J` (Hold to charge) | `A` Button or Right Trigger (`RT`) |
| **Charged Lock-On Shot** | Hold & Release `Space` / `J` | Hold & Release `A` Button / `RT` |
| **Smart Bomb** | `B` or `K` | `B` Button |
| **Boost Thrust** | `Left Shift` / `Right Shift` | `X` Button / Right Trigger |
| **Airbrake** | `Left Ctrl` / `Right Ctrl` / `Left Alt` | Left Trigger (`LT`) |
| **Barrel Roll (Deflect)** | Double-tap `A` / `D` or `Q` / `E` | Left / Right Bumper (`LB` / `RB`) |
| **Somersault (Loop)** | `S` + Boost *(All-Range Mode)* | `Y` Button or `Down` + `RT` |
| **U-Turn (Reverse)** | `S` + Brake *(All-Range Mode)* | `Down` + `LT` |
| **Cockpit / Chase Cam** | `V` | `Back` / Right Stick Click |
| **Retro CRT Scanlines** | `F1` | — |
| **Menu Confirm / Pause** | `Enter` / `ESC` | `A` Button / `Start` |

---

## 🛠️ Prerequisites & Dependencies

The project uses CMake's `FetchContent` to download dependencies automatically during configuration. You only need:

1. **Git** (for dependency fetching).
2. **CMake 3.16+**.
3. **C++17 Compiler**:
   - **Windows**: MinGW-w64 (GCC 8+) or MSVC (Visual Studio 2019+).
   - **Linux**: GCC 9+ or Clang 10+.
   - **macOS**: Apple Clang (macOS 10.15+).
4. **OpenGL 3.3 Core** compatible graphics card.

*Included / Auto-fetched Libraries*:
- **GLFW 3.4+** (Windowing and input polling).
- **GLAD** (OpenGL 3.3 Core loader, included in repository).
- **GLM 1.0+** (OpenGL Mathematics).
- **Windows waveOut / XInput** (Native Windows audio & haptics; no external audio DLLs needed).

---

## 🚀 How to Build & Run

### 🪟 Windows (Recommended: MinGW GCC)

1. **Clone the repository**:
   ```powershell
   git clone https://github.com/Ax3lrod/3d-on-rails-plane-shooter.git
   cd 3d-on-rails-plane-shooter
   ```

2. **Configure the project**:
   ```powershell
   cmake -B build -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release
   ```

3. **Build the executable**:
   ```powershell
   cmake --build build --config Release -j4
   ```

4. **Run the game**:
   > ⚠️ **Important**: Run the game from the **project root folder** so it can locate `resource/shaders/` and `resource/stages/`.
   ```powershell
   ./build/OpenGLProject.exe
   ```

---

### 🐧 Linux (Ubuntu / Debian / Arch)

1. **Install dependencies**:
   ```bash
   # Ubuntu / Debian
   sudo apt-get update
   sudo apt-get install build-essential cmake git libx11-dev libxi-dev libxrandr-dev libxinerama-dev libxcursor-dev libgl1-mesa-dev
   ```

2. **Configure, build, and run**:
   ```bash
   cmake -B build -DCMAKE_BUILD_TYPE=Release
   cmake --build build -j4
   ./build/OpenGLProject
   ```

---

### 🍏 macOS

1. **Build and run**:
   ```bash
   cmake -B build -DCMAKE_BUILD_TYPE=Release
   cmake --build build -j4
   ./build/OpenGLProject
   ```

---

## 📂 Project Architecture

```text
plane-shooter/
├── AGENTS.md                          # AI Developer & Level Designer Protocol
├── CMakeLists.txt                     # Master build configuration
├── headers/                           # C++ Engine Headers
│   ├── core/                          # Application loop, Camera, Input, Transform
│   ├── entities/                      # PlayerStarfighter, EnemyManager, Bosses, Wingmen
│   ├── combat/                        # Blaster projectiles, smart bombs, collisions
│   ├── renderer/                      # Mesh factories, Shaders, PostProcessor, VAO/VBO/EBO
│   ├── world/                         # LevelTimeline, WorldEnvironment, TrainConvoy
│   ├── ui/                            # HUD, Vector Comms Card, Radar minimap
│   └── audio/                         # SoundManager (multi-channel software synthesizer)
├── src/                               # C++ Implementation source files
├── resource/
│   ├── shaders/                       # default.vert, default.frag, screen.vert, screen.frag
│   └── stages/                        # stage1.json ... stage5.json data-driven timelines
├── docs/                              # Game Design & Architecture Documentation
│   ├── architecture/                  # available-systems.md, stage-schema.md
│   ├── game-design/patterns/          # wingman-rescue.md, collapsing-hazard.md, etc.
│   └── levels/                        # stage1-fallen-city.md (5-act beat sheet)
└── graphify-out/                      # Codebase Knowledge Graph (AST queryable)
```

---

## 🤖 AI Development Workflow & Staged Pipeline

This project adheres to the **Staged AI Development Protocol** outlined in [`AGENTS.md`](AGENTS.md):
1. **Game Designer**: Drafts 5-act beat sheets in `docs/levels/` without code.
2. **Game Architect**: Validates mechanics against [`docs/architecture/available-systems.md`](docs/architecture/available-systems.md) and maps data in `stage.json`.
3. **Implementation Coder**: Writes isolated, testable C++ code in small increments.
4. **Critic / Reviewer**: Audits gameplay pacing, telegraph clarity, and technical safety.

To explore codebase relationships or query subsystems:
```powershell
graphify query "how does boss walking robot take damage"
```

---

## 📜 License & Credits

- Developed by **Aryasatya Alaauddin** & the **Antigravity AI Agent Team**.
- Built with inspiration from Nintendo's *Star Fox 64* (Takaya Imamura / Shigeru Miyamoto) and MNKY's *Ex-Zodiac*.
- Licensed under the **MIT License**.
