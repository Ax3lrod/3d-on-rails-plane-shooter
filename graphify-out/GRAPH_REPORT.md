# Graph Report - plane-shooter  (2026-09-23)

## Corpus Check
- 70 files · ~285,816 words
- Verdict: corpus is large enough that graph structure adds value.
- Unclassified: 5 file(s) not represented in the graph (top: .frag 2, .vert 2, (none) 1)

## Summary
- 2082 nodes · 3201 edges · 112 communities (106 shown, 6 thin omitted)
- Extraction: 91% EXTRACTED · 9% INFERRED · 0% AMBIGUOUS · INFERRED: 275 edges (avg confidence: 0.87)
- Token cost: 0 input · 0 output

## Graph Freshness
- Built from commit: `462be33e`
- Run `git rev-parse HEAD` and compare to check if the graph is stale.
- Run `graphify update .` after code changes (no API cost).

## Community Hubs (Navigation)
- PlayerStarfighter
- Shader
- WorldEnvironment.h
- Mesh
- Mesh.cpp
- WorldEnvironment
- Engine
- LevelTimeline.cpp
- SoundManager.cpp
- TimelineEvent
- BossWalkingRobot
- BossMechaWorm
- BossMegaTank
- PostProcessor
- BossDreadnought
- BossTwinHelicopters
- Camera
- Input
- VBO
- CombatSystem.cpp
- SoundManager
- glad.c
- SoundManager::PrebakeSounds
- PlayerStarfighter.cpp
- ParticleSystem
- TrainConvoy
- BossDreadnought.cpp
- Engine.cpp
- Input.cpp
- Wingman
- WingmanSquadron
- Camera.cpp
- IsKeyPressed
- Enemy
- OrdnanceManager
- EBO
- vec3
- algorithm
- Engine::ProcessInput
- WingmanSquadron.cpp
- WorldEnvironment.cpp
- BossWeakpoint
- EnemyManager
- BossMechaWorm.cpp
- OceanIslandObstacle
- BossTwinHelicopters.cpp
- CommsMessage
- SoundManager.h
- agentsetup.md
- ChargedShot
- Voice
- CollapsingSpireHazard
- ProjectileManager
- IsKeyDown
- TumblingWing
- Vertex
- Particle
- Projectile
- Shockwave
- Engine::Init
- BuildingObstacle
- iostream
- SmartBomb
- SecretRelay
- Engine::Update
- SetRumble
- AsteroidObstacle
- HazardPillar
- RingGate
- TargetingReticle
- WorldEnvironment::SetTerrainTheme
- SpaceDebris
- CactusObstacle
- IceCrystalSpire
- StageDefinition
- _XINPUT_VIBRATION
- PlayerStarfighter::Draw
- TriggerUTurn
- glad.h
- Mesh::CreateBiomeHorizon
- 3D Rail-Shooter Game Engine Plan
- IBoss
- corneria.md
- LevelTimeline
- 🚀 Aegis Starfighter — 3D Rail-Shooter Engine
- 2. Staged Development Workflow
- 2. Event Types & Schemas
- Available Game Systems & Asset Inventory
- Design Pattern: Choreographed Enemy Formation
- 10. What Makes Corneria Feel Like a Real Place?
- 14. What I Would Borrow From Corneria
- 15. A Concrete Example for Your Own Game
- Design Pattern: Boss Arena & All-Range Transition
- Design Pattern: Collapsing Environmental Hazard
- Design Pattern: Wingman Rescue
- 2. 5-Act Encounter Choreography (Beat Sheet)
- 3. Opening: Approaching Corneria
- Environmental Elements
- 7. The Secret Route: Seven Stone Arches and a Waterfall
- Stage 1: Fallen City — Level Design Specification
- 6. The Wingmate Rescue: Falco's Distress Call
- 8. The Boss: Granga
- TreeObstacle
- rules/graphify.md
- workflows/graphify.md
- BossMegaTank.cpp
- .GetArenaCenter
- .GetBossName
- WindTurbineObstacle
- cmath
- CargoShipObstacle
- BossDreadnought::GetHealthRatio

## God Nodes (most connected - your core abstractions)
1. `Mesh` - 181 edges
2. `PlayerStarfighter` - 121 edges
3. `WorldEnvironment` - 99 edges
4. `Engine` - 94 edges
5. `Camera` - 67 edges
6. `SoundManager` - 66 edges
7. `AddQuad()` - 65 edges
8. `Input` - 58 edges
9. `Shader` - 58 edges
10. `BossDreadnought` - 54 edges

## Surprising Connections (you probably didn't know these)
- `Phase 9: End-of-Stage Boss Encounter (Colossal Dreadnought)` --references--> `BossDreadnought`  [INFERRED]
  plan.md → headers/entities/BossDreadnought.h
- `Phase 11: Wing Damage & Starfighter Breakdown System` --references--> `PlayerStarfighter`  [INFERRED]
  plan.md → headers/entities/PlayerStarfighter.h
- `4. Required Systems` --references--> `LevelTimeline`  [INFERRED]
  docs/game-design/patterns/collapsing-hazard.md → headers/world/LevelTimeline.h
- `Phase 26: Rooster Tail Water Spray Plume & Coastline Sea Environment` --references--> `ParticleSystem`  [INFERRED]
  plan.md → headers/combat/CombatSystem.h
- `Phase 25: Ex-Zodiac Reticle Palette & Signature Violet Starfighter Livery` --references--> `TargetingReticle`  [INFERRED]
  plan.md → headers/combat/CombatSystem.h

## Import Cycles
- None detected.

## Communities (112 total, 6 thin omitted)

### Community 0 - "PlayerStarfighter"
Cohesion: 0.02
Nodes (99): string, vector, PlayerStarfighter, AddBombs, AddShield, arenaCenter, arenaRadius, baseSpeed (+91 more)

### Community 1 - "Shader"
Cohesion: 0.06
Nodes (84): cstdio, IsLastInputGamepad, GLuint, Shader, Activate, Delete, ID, SetFloat (+76 more)

### Community 2 - "WorldEnvironment.h"
Cohesion: 0.07
Nodes (34): combatsystem, glad, glm, vector, SoundManager, string, mat4, vec3 (+26 more)

### Community 3 - "Mesh"
Cohesion: 0.03
Nodes (75): GLuint, vector, Mesh, Clear, CreateArcadeHorizon, CreateArenaPillar, CreateArenaPlaza, CreateAsteroid (+67 more)

### Community 4 - "Mesh.cpp"
Cohesion: 0.09
Nodes (68): constants, AddQuad(), AddTriangle(), vec3, Mesh::CreateArcadeHorizon(), Mesh::CreateArenaPillar(), Mesh::CreateArenaPlaza(), Mesh::CreateAsteroid() (+60 more)

### Community 5 - "WorldEnvironment"
Cohesion: 0.03
Nodes (74): SectorStage, string, vector, WorldEnvironment, arenaCenter, arenaPillarMesh, arenaPlazaMesh, arenaSpawned (+66 more)

### Community 6 - "Engine"
Cohesion: 0.03
Nodes (59): GameState, Engine, audio, availableStages, boss, bossDeathCamTriggered, bossSpawned, camera (+51 more)

### Community 7 - "LevelTimeline.cpp"
Cohesion: 0.06
Nodes (46): cctype, fstream, CheckCompileErrors, CompileProgram, JsonType, GLuint, mat4, string (+38 more)

### Community 8 - "SoundManager.cpp"
Cohesion: 0.08
Nodes (42): PrebakeSounds, Shutdown, StopBGM, ClampSample(), BGMTrack, SoundID, MidiHz(), RandFloat() (+34 more)

### Community 9 - "TimelineEvent"
Cohesion: 0.08
Nodes (25): EnemyType, WingmanID, TimelineEvent, count, duration, enemyType, formation, hazardSpan (+17 more)

### Community 10 - "BossWalkingRobot"
Cohesion: 0.05
Nodes (34): BossWalkingRobot, ApplyShockwaveDamage, arenaCenter, beamBurstCount, beamBurstTimer, beamCooldown, bodyMesh, cannonCooldown (+26 more)

### Community 11 - "BossMechaWorm"
Cohesion: 0.04
Nodes (38): BossMechaWorm, ApplyShockwaveDamage, approachProgress, breachApexPos, breachDuration, breachEndPos, breachStartPos, breachTimer (+30 more)

### Community 12 - "BossMegaTank"
Cohesion: 0.04
Nodes (37): BossMegaTank, ApplyShockwaveDamage, approachProgress, chargeLaserTimer, chassisMesh, CheckLaserHit, coreDestroyed, coreHp (+29 more)

### Community 13 - "PostProcessor"
Cohesion: 0.05
Nodes (33): GLuint, PostProcessor, BeginRender, Cleanup, colorTexture, crtEnabled, EndRender, fbo (+25 more)

### Community 14 - "BossDreadnought"
Cohesion: 0.05
Nodes (33): BossState, BossDreadnought, ApplyShockwaveDamage, approachProgress, attackTimer, CheckLaserHit, coreMesh, corePulseTime (+25 more)

### Community 15 - "BossTwinHelicopters"
Cohesion: 0.07
Nodes (22): BossTwinHelicopters, ApplyShockwaveDamage, approachProgress, CheckLaserHit, deathTimer, Draw, FindLockTarget, fuselageMesh (+14 more)

### Community 16 - "Camera"
Cohesion: 0.05
Nodes (40): CinematicMode, Camera, aspectRatio, baseFov, cinematicDuration, cinematicFocusPos, cinematicMode, cinematicOrbitAngle (+32 more)

### Community 17 - "Input"
Cohesion: 0.07
Nodes (34): glfw3, Input, activeGamepadJid, currentGamepadButtons, currentKeys, currentTime, DOUBLE_TAP_THRESHOLD, doubleTapKeys (+26 more)

### Community 18 - "VBO"
Cohesion: 0.07
Nodes (25): GLint, GLsizei, GLuint, VAO, Bind, Delete, ID, LinkAttrib (+17 more)

### Community 19 - "CombatSystem.cpp"
Cohesion: 0.10
Nodes (22): SpawnWaterRipple, SpawnLaserWithDir, mat4, vec3, OrdnanceManager::Draw(), OrdnanceManager::SpawnChargedShot(), OrdnanceManager::SpawnSmartBomb(), OrdnanceManager::TriggerShockwave() (+14 more)

### Community 20 - "SoundManager"
Cohesion: 0.07
Nodes (27): BGMTrack, SoundManager, audioThread, AudioThreadFunc, BUFFER_COUNT, BUFFER_SAMPLES, currentBgm, Init (+19 more)

### Community 21 - "glad.c"
Cohesion: 0.15
Nodes (25): dlfcn, GLADloadproc, close_gl(), find_coreGL(), find_extensionsGL(), free_exts(), get_exts(), gladLoadGL() (+17 more)

### Community 22 - "SoundManager::PrebakeSounds"
Cohesion: 0.07
Nodes (27): GenerateBarrelRollSound, GenerateBombExplosionSound, GenerateBombLaunchSound, GenerateBoostRoarSound, GenerateBossBGM, GenerateBrakeHissSound, GenerateChargedShotFireSound, GenerateChargeHumSound (+19 more)

### Community 23 - "PlayerStarfighter.cpp"
Cohesion: 0.11
Nodes (13): GetForwardVector, vec3, PlayerStarfighter::GetFarTargetPos(), PlayerStarfighter::GetForwardVector(), PlayerStarfighter::GetLeftMuzzlePos(), PlayerStarfighter::GetLeftWingRootWorldPos(), PlayerStarfighter::GetLeftWingTipWorldPos(), PlayerStarfighter::GetNearTargetPos() (+5 more)

### Community 24 - "ParticleSystem"
Cohesion: 0.09
Nodes (22): bosswalkingrobot, ParticleSystem, Clear, Draw, particleMesh, particles, SpawnChargeInwardSparks, SpawnCriticalDamageSmoke (+14 more)

### Community 25 - "TrainConvoy"
Cohesion: 0.08
Nodes (25): vec3, vector, TrainCar, destroyed, fireTimer, hasTurret, health, position (+17 more)

### Community 26 - "BossDreadnought.cpp"
Cohesion: 0.13
Nodes (19): bossdreadnought, FirePhase1Attacks, FirePhase2Attacks, UpdateDeathSpiral, UpdateSubsystemsWorldPos, BossDreadnought::ApplyShockwaveDamage(), BossDreadnought::BossDreadnought(), BossDreadnought::CheckLaserHit() (+11 more)

### Community 27 - "Engine.cpp"
Cohesion: 0.09
Nodes (17): cstring, RegisterHitCombo, SaveHighScores, StartMission, EndFrame, input, GLFWwindow, string (+9 more)

### Community 28 - "Input.cpp"
Cohesion: 0.09
Nodes (7): StopRumble, UpdateRumble, GLFWwindow, Input::Init(), Input::KeyCallback(), Input::Update(), Input::UpdateRumble()

### Community 29 - "Wingman"
Cohesion: 0.09
Nodes (23): Wingman, bobOffset, callsign, currentBank, currentPitch, currentYaw, distressTimer, evasiveTimer (+15 more)

### Community 30 - "WingmanSquadron"
Cohesion: 0.09
Nodes (23): deque, vector, WingmanSquadron, activeMessage, asteroidTriggered, bossCoreTriggered, bossTriggered, bossTurretTriggered (+15 more)

### Community 31 - "Camera.cpp"
Cohesion: 0.13
Nodes (14): SetTargetFOV, SetViewMode, StopCinematic, Camera::Follow(), Camera::GetProjectionMatrix(), Camera::GetViewMatrix(), Camera::SetViewMode(), Camera::StartBossDeathSlowMo() (+6 more)

### Community 32 - "IsKeyPressed"
Cohesion: 0.21
Nodes (17): IsDoubleTap, IsGamepadButtonPressed, IsKeyPressed, Input::IsBombPressed(), Input::IsBoostPressed(), Input::IsBrakePressed(), Input::IsCockpitTogglePressed(), Input::IsFirePressed() (+9 more)

### Community 33 - "Enemy"
Cohesion: 0.12
Nodes (17): Enemy, active, aimAnglePitch, aimAngleYaw, fireCooldown, fireTimer, health, maxHealth (+9 more)

### Community 34 - "OrdnanceManager"
Cohesion: 0.12
Nodes (16): OrdnanceManager, bombMesh, chargeBallMesh, chargedShots, Clear, DetonateBomb, Draw, shockwaveMesh (+8 more)

### Community 35 - "EBO"
Cohesion: 0.15
Nodes (12): EBO, Bind, BufferData, Delete, ID, Unbind, GLuint, GLenum (+4 more)

### Community 36 - "vec3"
Cohesion: 0.08
Nodes (25): CanyonSlice, position, sliceType, CityGantryObstacle, position, spanWidth, DesertPyramidObstacle, position (+17 more)

### Community 37 - "algorithm"
Cohesion: 0.14
Nodes (12): algorithm, SpawnCustomWave, SpawnWave, EnemyType, SoundManager, string, vec3, EnemyManager::Draw() (+4 more)

### Community 38 - "Engine::ProcessInput"
Cohesion: 0.13
Nodes (15): CheckNewHighScore, ReturnToTitle, IsBombPressed, IsCockpitTogglePressed, IsFirePressed, IsFireReleased, IsGamepadButtonReleased, IsKeyReleased (+7 more)

### Community 39 - "WingmanSquadron.cpp"
Cohesion: 0.26
Nodes (10): TriggerTransmission, SoundManager, string, vec3, WingmanID, Wingman::Wingman(), WingmanSquadron::Draw(), WingmanSquadron::TriggerScriptedRescue() (+2 more)

### Community 40 - "WorldEnvironment.cpp"
Cohesion: 0.17
Nodes (10): cstdlib, WorldEnvironment, GenerateChunk, SpawnCollapsingSpire, SectorStage, WorldEnvironment::Clear(), WorldEnvironment::Draw(), WorldEnvironment::GenerateChunk() (+2 more)

### Community 41 - "BossWeakpoint"
Cohesion: 0.14
Nodes (13): BossWeakpoint, destroyed, fireInterval, fireTimer, health, invulnerable, localOffset, maxHealth (+5 more)

### Community 42 - "EnemyManager"
Cohesion: 0.14
Nodes (14): EnemyManager, Clear, Draw, droneMesh, enemies, FindLockTarget, interceptorMesh, lastPlayerZ (+6 more)

### Community 43 - "BossMechaWorm.cpp"
Cohesion: 0.19
Nodes (8): bossmechaworm, BossMechaWorm::ApplyShockwaveDamage(), BossMechaWorm::CheckLaserHit(), BossMechaWorm::Draw(), BossMechaWorm::FindLockTarget(), BossMechaWorm::Update(), SoundManager, vec3

### Community 44 - "OceanIslandObstacle"
Cohesion: 0.50
Nodes (4): OceanIslandObstacle, position, rotation, scale

### Community 45 - "BossTwinHelicopters.cpp"
Cohesion: 0.19
Nodes (8): bosstwinhelicopters, BossTwinHelicopters::ApplyShockwaveDamage(), BossTwinHelicopters::CheckLaserHit(), BossTwinHelicopters::Draw(), BossTwinHelicopters::FindLockTarget(), BossTwinHelicopters::Update(), SoundManager, vec3

### Community 46 - "CommsMessage"
Cohesion: 0.15
Nodes (13): CommsMessage, callsign, chatterTimer, color, duration, line1, line2, speaker (+5 more)

### Community 47 - "SoundManager.h"
Cohesion: 0.18
Nodes (10): atomic, cstdint, SoundSample, pcmData, sampleRate, memory, mmsystem, mutex (+2 more)

### Community 48 - "agentsetup.md"
Cohesion: 0.05
Nodes (36): 10. Use a Planner → Coder → Reviewer Workflow, 11. Give the AI Actual Gameplay Feedback, 12. Use Playtest Metrics, 13. Build a Design Evaluation Rubric, 14. Don't Let AI Generate Content Without a Design Library, 15. Give the AI a Clear Asset and System Inventory, 16. How I'd Organize Your Actual Development Workflow, 17. The Prompt I'd Actually Give agy (+28 more)

### Community 49 - "ChargedShot"
Cohesion: 0.18
Nodes (11): ChargedShot, active, aoeRadius, damage, hasHomingTarget, homingTarget, homingTurnSpeed, lifetime (+3 more)

### Community 50 - "Voice"
Cohesion: 0.20
Nodes (10): SoundID, Voice, active, loop, pitch, position, sampleCount, samples (+2 more)

### Community 51 - "CollapsingSpireHazard"
Cohesion: 0.20
Nodes (10): CollapsingSpireHazard, collapseSpeed, currentAngle, destroyed, health, height, isTriggered, position (+2 more)

### Community 52 - "ProjectileManager"
Cohesion: 0.22
Nodes (9): vector, ProjectileManager, Clear, Draw, enemyLaserMesh, laserMesh, projectiles, SpawnLaser (+1 more)

### Community 53 - "IsKeyDown"
Cohesion: 0.33
Nodes (9): GetGamepadAxis, IsFireDown, IsGamepadButtonDown, IsKeyDown, Input::GetAxisHorizontal(), Input::GetAxisVertical(), Input::IsBoostDown(), Input::IsBrakeDown() (+1 more)

### Community 54 - "TumblingWing"
Cohesion: 0.22
Nodes (9): vec3, TumblingWing, active, isLeft, lifetime, position, rotation, rotSpeed (+1 more)

### Community 55 - "Vertex"
Cohesion: 0.20
Nodes (10): vec3, SetupMesh, Vertex, color, normal, position, Phase 1: 3D Math Pipeline & Engine Core Architecture, GLuint (+2 more)

### Community 56 - "Particle"
Cohesion: 0.25
Nodes (8): Particle, active, color, lifetime, maxLifetime, position, size, velocity

### Community 57 - "Projectile"
Cohesion: 0.25
Nodes (8): Projectile, active, color, isPlayer, lifetime, position, radius, velocity

### Community 58 - "Shockwave"
Cohesion: 0.25
Nodes (8): Shockwave, active, alpha, currentRadius, damage, expansionSpeed, maxRadius, position

### Community 59 - "Engine::Init"
Cohesion: 0.25
Nodes (8): LoadHighScores, Init, CreateCube, CreateRing, CreateShadowDisc, GetStandardCampaignStages, Engine::Init(), Engine::StartMission()

### Community 60 - "BuildingObstacle"
Cohesion: 0.25
Nodes (8): BuildingObstacle, depth, height, meshVariant, position, rotation, scale, width

### Community 61 - "iostream"
Cohesion: 0.29
Nodes (6): engine, exception, Init, Run, iostream, main()

### Community 62 - "SmartBomb"
Cohesion: 0.29
Nodes (7): vec3, SmartBomb, active, lifetime, position, radius, velocity

### Community 63 - "SecretRelay"
Cohesion: 0.29
Nodes (7): SecretRelay, destroyed, health, maxHealth, position, pulseTimer, radius

### Community 64 - "Engine::Update"
Cohesion: 0.33
Nodes (6): CompleteHyperspaceWarp, HandleCollisions, ProcessInput, TriggerHyperspaceWarp, Update, Engine::Update()

### Community 65 - "SetRumble"
Cohesion: 0.33
Nodes (6): SetRumble, GetLeftWingRootWorldPos, GetRightWingRootWorldPos, PlayerStarfighter::DamageLeftWing(), PlayerStarfighter::DamageRightWing(), PlayerStarfighter::TakeDamage()

### Community 66 - "AsteroidObstacle"
Cohesion: 0.33
Nodes (6): AsteroidObstacle, destroyed, position, radius, rotation, rotSpeed

### Community 67 - "HazardPillar"
Cohesion: 0.33
Nodes (6): HazardPillar, destroyed, health, height, position, radius

### Community 68 - "RingGate"
Cohesion: 0.33
Nodes (6): RingGate, collected, isGold, position, radius, rotation

### Community 69 - "TargetingReticle"
Cohesion: 0.33
Nodes (6): TargetingReticle, crosshairMesh, Draw, farPointMesh, lockOnMesh, Phase 25: Ex-Zodiac Reticle Palette & Signature Violet Starfighter Livery

### Community 70 - "WorldEnvironment::SetTerrainTheme"
Cohesion: 0.40
Nodes (5): CreateBiomeHorizon, CreateCityRoad, CreateOpenFieldTerrain, string, WorldEnvironment::SetTerrainTheme()

### Community 72 - "SpaceDebris"
Cohesion: 0.40
Nodes (5): SpaceDebris, position, radius, rotation, rotSpeed

### Community 73 - "CactusObstacle"
Cohesion: 0.50
Nodes (4): CactusObstacle, position, rotation, scale

### Community 74 - "IceCrystalSpire"
Cohesion: 0.50
Nodes (4): IceCrystalSpire, position, rotation, scale

### Community 75 - "StageDefinition"
Cohesion: 0.17
Nodes (12): string, vec3, StageDefinition, bossName, bossType, difficulty, id, scriptPath (+4 more)

### Community 77 - "_XINPUT_VIBRATION"
Cohesion: 0.50
Nodes (4): _XINPUT_VIBRATION, wLeftMotorSpeed, wRightMotorSpeed, WORD

### Community 78 - "PlayerStarfighter::Draw"
Cohesion: 0.67
Nodes (3): GetChargeProgress, GetNosePos, PlayerStarfighter::Draw()

### Community 79 - "TriggerUTurn"
Cohesion: 0.67
Nodes (3): HandleInput, TriggerUTurn, PlayerStarfighter::Update()

### Community 82 - "3D Rail-Shooter Game Engine Plan"
Cohesion: 0.07
Nodes (29): 3D Rail-Shooter Game Engine Plan, Phase 10: Audio & Sound Synthesizer System, Phase 11: Wing Damage & Starfighter Breakdown System, Phase 12: Cockpit 1st-Person Mode & Dynamic Cinematic Camera (Option B), Phase 13: All-Range Mode 360° Dogfight Arena & Acrobatics (Option C), Phase 15: Procedural Chiptune/FM Stage Soundtrack & Boss BGM (Option A), Phase 17: Branching Mission Routes & Secret Objectives (Option C), Phase 18: 3D Hangar Title Screen, Settings Menu & High-Score Hall of Fame (Option D) (+21 more)

### Community 83 - "IBoss"
Cohesion: 0.10
Nodes (15): IBoss, ApplyShockwaveDamage, CheckLaserHit, Draw, FindLockTarget, GetBossName, GetHealthRatio, IsActive (+7 more)

### Community 85 - "corneria.md"
Cohesion: 0.15
Nodes (12): 11. The Most Important Lesson: Corneria Is Not Just a Collection of Objects, 12. How I'd Represent Corneria in a Level Editor, 13. A Better Data Model for Your Game, 1. The Core Concept: A Flight Through a War Zone, 2. The Level Flow, 5. The First Real Gameplay Rhythm, 9. The Alternate Boss: Attack Carrier, Design A: Object Placement (+4 more)

### Community 86 - "LevelTimeline"
Cohesion: 0.15
Nodes (13): vector, LevelTimeline, bossTriggered, bossType, events, isLoaded, LoadFromFile, musicTrack (+5 more)

### Community 87 - "🚀 Aegis Starfighter — 3D Rail-Shooter Engine"
Cohesion: 0.18
Nodes (11): 🚀 Aegis Starfighter — 3D Rail-Shooter Engine, 🤖 AI Development Workflow & Staged Pipeline, 🎮 Controls, 🚀 How to Build & Run, 🌟 Key Features, 📜 License & Credits, 🐧 Linux (Ubuntu / Debian / Arch), 🍏 macOS (+3 more)

### Community 88 - "2. Staged Development Workflow"
Cohesion: 0.20
Nodes (10): 1. Project Philosophy & Core Directive, 2. Staged Development Workflow, 3. Codebase Directory Map, 4. Key Subsystem Conventions, AGENTS.md — AI Developer & Level Designer Guide, Rules of Engagement for AI Agents:, Stage 1: Game Designer (No Code), Stage 2: Game Architect (Mapping & Data Schema) (+2 more)

### Community 89 - "2. Event Types & Schemas"
Cohesion: 0.20
Nodes (9): 1. Top-Level Object, 2.1 `enemy_wave` / `formation`, 2.2 `dialogue`, 2.3 `wingman_rescue`, 2.4 `hazard_spire`, 2.5 `boss_trigger`, 2. Event Types & Schemas, Required Fields: (+1 more)

### Community 90 - "Available Game Systems & Asset Inventory"
Cohesion: 0.18
Nodes (8): 1. Player Starfighter Subsystems (`PlayerStarfighter.h`), 2. Combat & Enemy Subsystems (`EnemyManager.h`), 3. Boss Archetypes (`IBoss.h`), 4. World Environment & Decorative Props (`WorldEnvironment.h`), 5. Wingman Squadron & Narrative Comms (`WingmanSquadron.h`, `HUD.h`), 6. Timeline Event Types (`LevelTimeline.h`), Available Game Systems & Asset Inventory, Choreographed Formations:

### Community 91 - "Design Pattern: Choreographed Enemy Formation"
Cohesion: 0.22
Nodes (8): 1. Purpose & Fantasy, 2.1 The Classic V-Formation (Arrowhead), 2.2 The Ground-Air Crossfire Pincer, 2.3 The Rooftop Flak Battery, 2. Standard Formation Typologies, 3. Design Constraints & Common Failure Modes, 4. Timeline JSON Example, Design Pattern: Choreographed Enemy Formation

### Community 92 - "10. What Makes Corneria Feel Like a Real Place?"
Cohesion: 0.29
Nodes (7): 10. What Makes Corneria Feel Like a Real Place?, A. City, B. Ground Vehicles, C. Buildings Under Attack, D. Ocean and Waterfall, E. Enemy Formations, F. Wingmates

### Community 93 - "14. What I Would Borrow From Corneria"
Cohesion: 0.29
Nodes (7): 14. What I Would Borrow From Corneria, 1. Establish the world immediately, 2. Introduce mechanics through situations, 3. Vary the encounter rhythm, 4. Use the environment to support gameplay, 5. Reward exploration and mastery, 6. Make the boss a change in experience

### Community 94 - "15. A Concrete Example for Your Own Game"
Cohesion: 0.29
Nodes (7): 15. A Concrete Example for Your Own Game, Section 1: Approach, Section 2: Colony Defense, Section 3: Wingmate Rescue, Section 4: Hidden Route, Section 5: Boss, Setting

### Community 95 - "Design Pattern: Boss Arena & All-Range Transition"
Cohesion: 0.29
Nodes (6): 1. Purpose & Fantasy, 2. Player Experience & Sequence, 3. Design Constraints & Common Failure Modes, 4. Required Systems, 5. Timeline JSON Example, Design Pattern: Boss Arena & All-Range Transition

### Community 96 - "Design Pattern: Collapsing Environmental Hazard"
Cohesion: 0.29
Nodes (6): 1. Purpose & Fantasy, 2. Player Experience & Sequence, 3. Design Constraints & Common Failure Modes, 4. Required Systems, 5. Timeline JSON Example, Design Pattern: Collapsing Environmental Hazard

### Community 97 - "Design Pattern: Wingman Rescue"
Cohesion: 0.29
Nodes (6): 1. Purpose & Fantasy, 2. Player Experience & Sequence, 3. Design Constraints & Common Failure Modes, 4. Required Systems, 5. Timeline JSON Example, Design Pattern: Wingman Rescue

### Community 98 - "2. 5-Act Encounter Choreography (Beat Sheet)"
Cohesion: 0.33
Nodes (6): 2. 5-Act Encounter Choreography (Beat Sheet), Act 1: The Outskirts Approach ($Z = 0$ to $-2,500$), Act 2: Suburban Escalation ($Z = -2,500$ to $-4,500$), Act 3: Striker Rescue & Tactical Respite ($Z = -4,500$ to $-6,500$), Act 4: Downtown Canyon & The Collapsing Spire ($Z = -6,500$ to $-11,000$), Act 5: Boss Approach & Colossus Arena ($Z = -11,000$ to $-13,000+$)

### Community 99 - "3. Opening: Approaching Corneria"
Cohesion: 0.40
Nodes (5): 3. Opening: Approaching Corneria, Design lesson for your game, What happens, What the player sees, Why it works

### Community 100 - "Environmental Elements"
Cohesion: 0.40
Nodes (5): 4. The City Assault, A. Buildings Are Gameplay Objects, B. Ground Combat Adds Variety, C. Environmental Threats Create Anticipation, Environmental Elements

### Community 101 - "7. The Secret Route: Seven Stone Arches and a Waterfall"
Cohesion: 0.40
Nodes (5): 7. The Secret Route: Seven Stone Arches and a Waterfall, A. The Player Isn't Given a Menu, B. The Route Is Skill-Based, C. The Route Has Narrative Context, Design Lesson

### Community 102 - "Stage 1: Fallen City — Level Design Specification"
Cohesion: 0.40
Nodes (4): 1. Stage Vision & Fantasy, 3. Boss Design: Iron Colossus (`BossWalkingRobot`), Multi-Part Tactical Mechanics:, Stage 1: Fallen City — Level Design Specification

### Community 103 - "6. The Wingmate Rescue: Falco's Distress Call"
Cohesion: 0.50
Nodes (4): 6. The Wingmate Rescue: Falco's Distress Call, How You Could Implement It, The Encounter Structure, Why This Is Effective

### Community 104 - "8. The Boss: Granga"
Cohesion: 0.67
Nodes (3): 8. The Boss: Granga, The Stage Changes Its Gameplay, Why the Leg Mechanic Is Good

### Community 105 - "TreeObstacle"
Cohesion: 0.50
Nodes (4): TreeObstacle, position, rotation, scale

### Community 108 - "BossMegaTank.cpp"
Cohesion: 0.19
Nodes (8): bossmegatank, BossMegaTank::ApplyShockwaveDamage(), BossMegaTank::CheckLaserHit(), BossMegaTank::Draw(), BossMegaTank::FindLockTarget(), BossMegaTank::Update(), SoundManager, vec3

### Community 111 - "WindTurbineObstacle"
Cohesion: 0.50
Nodes (4): WindTurbineObstacle, position, rotation, rotSpeed

### Community 112 - "cmath"
Cohesion: 0.20
Nodes (7): cmath, TrainConvoy, SoundManager, vec3, TrainConvoy::CheckLaserHit(), TrainConvoy::Draw(), TrainConvoy::Update()

### Community 113 - "CargoShipObstacle"
Cohesion: 0.33
Nodes (6): CargoShipObstacle, position, rotation, scale, speed, Phase 42: Zone C Downtown Concrete Jungle Overhaul & Dynamic Marine Life

### Community 114 - "BossDreadnought::GetHealthRatio"
Cohesion: 0.67
Nodes (3): GetMaxHealth, GetTotalHealth, BossDreadnought::GetHealthRatio()

## Knowledge Gaps
- **1071 isolated node(s):** `pcmData`, `sampleRate`, `samples`, `sampleCount`, `position` (+1066 more)
  These have ≤1 connection - possible missing edges or undocumented components. (Counts symbols only; 1339 node(s) total have ≤1 connection when file, concept and rationale nodes are included.)
- **6 thin communities (<3 nodes) omitted from report** — run `graphify query` to explore isolated nodes.

## Suggested Questions
_Questions this graph is uniquely positioned to answer:_

- **Why does `Mesh` connect `Mesh` to `PlayerStarfighter`, `Shader`, `WorldEnvironment.h`, `Mesh.cpp`, `WorldEnvironment`, `Engine`, `BossWalkingRobot`, `BossMechaWorm`, `BossMegaTank`, `BossDreadnought`, `BossTwinHelicopters`, `VBO`, `ParticleSystem`, `TrainConvoy`, `Wingman`, `OrdnanceManager`, `EBO`, `EnemyManager`, `ProjectileManager`, `Vertex`, `Engine::Init`, `TargetingReticle`, `WorldEnvironment::SetTerrainTheme`, `Mesh::CreateBiomeHorizon`?**
  _High betweenness centrality (0.407) - this node is a cross-community bridge._
- **Why does `Engine` connect `Engine` to `PlayerStarfighter`, `Shader`, `WorldEnvironment.h`, `Mesh`, `PostProcessor`, `Camera`, `ParticleSystem`, `Engine.cpp`, `WingmanSquadron`, `OrdnanceManager`, `Engine::ProcessInput`, `WorldEnvironment.cpp`, `EnemyManager`, `ProjectileManager`, `Vertex`, `Engine::Init`, `iostream`, `Engine::Update`, `TargetingReticle`, `IBoss`, `LevelTimeline`, `cmath`?**
  _High betweenness centrality (0.201) - this node is a cross-community bridge._
- **Why does `PlayerStarfighter` connect `PlayerStarfighter` to `SetRumble`, `WorldEnvironment.h`, `Mesh`, `Engine`, `PlayerStarfighter::Draw`, `TriggerUTurn`, `Input`, `3D Rail-Shooter Game Engine Plan`, `TumblingWing`, `PlayerStarfighter.cpp`?**
  _High betweenness centrality (0.112) - this node is a cross-community bridge._
- **Are the 2 inferred relationships involving `PlayerStarfighter` (e.g. with `Phase 11: Wing Damage & Starfighter Breakdown System` and `Phase 22: Full Gamepad / Controller Integration (Option C)`) actually correct?**
  _`PlayerStarfighter` has 2 INFERRED edges - model-reasoned connections that need verification._
- **Are the 4 inferred relationships involving `Engine` (e.g. with `Phase 1: 3D Math Pipeline & Engine Core Architecture` and `Phase 31: Data-Driven Level Scripting, Choreographed Waves & Ground Units (Tanks & Turrets)`) actually correct?**
  _`Engine` has 4 INFERRED edges - model-reasoned connections that need verification._
- **What connects `pcmData`, `sampleRate`, `samples` to the rest of the system?**
  _1071 weakly-connected nodes found - possible documentation gaps or missing edges._
- **Should `PlayerStarfighter` be split into smaller, more focused modules?**
  _Cohesion score 0.019417475728155338 - nodes in this community are weakly interconnected._