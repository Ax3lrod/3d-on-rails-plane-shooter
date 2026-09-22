# AGENTS.md — AI Developer & Level Designer Guide

> **Project**: 3D Rail-Shooter (*Star Fox 64* & *Ex-Zodiac* inspired)  
> **Tech Stack**: C++17, OpenGL 3.3 Core, GLFW, GLAD, GLM, Windows waveOut / XInput  
> **Build System**: CMake 3.16+ (MinGW GCC on Windows)  
> **Knowledge Graph**: `graphify-out/` (Queryable via `graphify query "<question>"`)

---

## 1. Project Philosophy & Core Directive

This project is a high-octane 3D arcade rail-shooter built around deliberate encounter pacing, crisp arcade physics, low-poly retro aesthetics, and responsive dogfighting.

### Rules of Engagement for AI Agents:
1. **Never write implementation code without design clearance**: When asked to create or change levels/features, never jump straight to dumping 2,000 lines of code. Follow the **Staged Workflow** below.
2. **Never hallucinate engine features**: Always consult [`docs/architecture/available-systems.md`](file:///C:/Users/Aryasatya%20Alaauddin/projects/plane-shooter/docs/architecture/available-systems.md) or run `graphify query "<feature>"` before designing or writing events. If a mechanic doesn't exist in C++, define its scope before using it in stage JSON files.
3. **Preserve documentation and comments**: Do not remove functional comments or rewrite working code.
4. **Follow the Anti-Slop Filter**: No vague AI-slop comments (e.g. `// initialize variable`, `// update logic`). Write concise, self-documenting code with purposeful comments.

---

## 2. Staged Development Workflow

When developing stages, enemies, or gameplay mechanics, adopt the four-stage discipline:

```
[1. Game Designer]  ──>  [2. Game Architect]  ──>  [3. Implementation Coder]  ──>  [4. Reviewer/Critic]
```

### Stage 1: Game Designer (No Code)
- **Deliverable**: A 5-act beat sheet document in [`docs/levels/`](file:///C:/Users/Aryasatya%20Alaauddin/projects/plane-shooter/docs/levels/).
- **Focus**:
  - Fantasy and setting (e.g., ruined industrial metropolis, volcanic fortress).
  - Pacing curve: Act 1 (Intro/Warm-up) $\rightarrow$ Act 2 (Escalation) $\rightarrow$ Act 3 (Set Piece / Rescue) $\rightarrow$ Act 4 (Climax/Gauntlet) $\rightarrow$ Act 5 (Boss Encounter).
  - Player emotional beats, hazard telegraphs, and dialogue flow.
  - Zero code written in this stage.

### Stage 2: Game Architect (Mapping & Data Schema)
- **Deliverable**: Architectural delta plan.
- **Focus**:
  - Run `graphify query "<subsystem>"` to map design beats to existing C++ classes.
  - Validate what can be done via data (`stage.json`) vs what requires new C++ code.
  - Ensure all event types, enemy spawns, and boss states strictly match [`docs/architecture/stage-schema.md`](file:///C:/Users/Aryasatya%20Alaauddin/projects/plane-shooter/docs/architecture/stage-schema.md).

### Stage 3: Implementation Coder (Targeted & Smallest Increments)
- **Deliverable**: Compilable, tested code changes.
- **Focus**:
  - Implement smallest complete increments.
  - Follow existing conventions in [`headers/`](file:///C:/Users/Aryasatya%20Alaauddin/projects/plane-shooter/headers/) and [`src/`](file:///C:/Users/Aryasatya%20Alaauddin/projects/plane-shooter/src/).
  - Always test build with:
    ```powershell
    cmake --build build --config Release -j4
    ```

### Stage 4: Reviewer / Critic (Self-Critique Rubric)
- **Deliverable**: Evaluation report before claiming completion.
- **Rubric**:
  - **Clarity**: Are enemy telegraphs and hazards visually readable at 21 units/sec?
  - **Pacing**: Is there sufficient breathing room between intense waves?
  - **Gameplay**: Can the player avoid damage through skill (barrel roll, boost, brake)?
  - **Narrative**: Does radio chatter sound authentic, urgent, and concise?
  - **Technical Safety**: Clean build? No memory leaks? Culling distances handled?
  - **Anti-Slop**: Code comments clean and meaningful?

---

## 3. Codebase Directory Map

```text
plane-shooter/
├── AGENTS.md                                # This document
├── plan.md                                  # Active project roadmap & completed phases
├── CMakeLists.txt                           # Master CMake build configuration
├── headers/                                 # C++ header files
│   ├── core/                                # Engine, Camera, Input, Transform
│   ├── entities/                            # PlayerStarfighter, EnemyManager, WingmanSquadron, Bosses
│   ├── combat/                              # CombatSystem (lasers, bombs, collisions)
│   ├── renderer/                            # Mesh, Shader, PostProcessor, VAO/VBO/EBO
│   ├── world/                               # LevelTimeline, WorldEnvironment, TrainConvoy
│   ├── ui/                                  # HUD, radar, comms card, gauges
│   └── audio/                               # SoundManager (multi-channel software synth)
├── src/                                     # C++ implementation files matching headers/
├── resource/
│   ├── shaders/                             # default.vert, default.frag, screen.vert, screen.frag
│   └── stages/                              # stage1.json ... stage5.json data-driven timelines
├── docs/                                    # Persistent project memory
│   ├── architecture/
│   │   ├── available-systems.md             # Inventory of all C++ classes & capabilities
│   │   └── stage-schema.md                  # JSON timeline event schemas
│   ├── game-design/
│   │   └── patterns/                        # Reusable design patterns (wingman rescue, hazards)
│   └── levels/                              # Beat sheets for each mission
└── graphify-out/                            # Codebase knowledge graph (graph.json, wiki, html)
```

---

## 4. Key Subsystem Conventions

- **Player Coordinates**: Player flies along negative Z axis ($Z \to -\infty$). Forward speed is $\sim 21.0\text{ units/s}$. $X \in [-30, 30]$ is standard corridor width; $Y \in [-5, 18]$ is flight altitude ($Y = -7.5$ is ground level).
- **All-Range Mode**: Triggered in boss encounters via `player->SetAllRangeMode(true, arenaCenter, arenaRadius)`. In this mode, player can pitch, roll, yaw, somersault ($S + \text{Boost}$), and U-turn ($S + \text{Brake}$) in 360°.
- **Level Timeline**: Driven by `player->transform.position.z <= event.triggerZ`. Events in `stage.json` must be ordered from highest $Z$ (e.g. $-300$) to lowest $Z$ (e.g. $-13000$).
- **Knowledge Graph Maintenance**: After creating or modifying C++ source files, always run:
  ```powershell
  python -m graphify.cli update .
  ```
