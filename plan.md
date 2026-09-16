# 3D Rail-Shooter Game Engine Plan

## Project Overview
A modular C++ / OpenGL 3D rail-shooter game engine inspired by the arcade corridor-flight mechanics of *Star Fox 64*, featuring an original starfighter, dynamic camera corridor system, 3D reticle aiming, tactical flight maneuvers (boost, brake, evasion spin), weapon systems, enemy flight waves, and a retro low-poly aesthetic.

---

## Phase 1: 3D Math Pipeline & Engine Core Architecture
- [x] **Task 1.1**: Integrate GLM (OpenGL Mathematics) via CMake `FetchContent` and configure include directories.
- [x] **Task 1.2**: Upgrade `Shader` class to support uniform setters (`SetMat4`, `SetVec3`, `SetVec4`, `SetFloat`) and update vertex/fragment shaders for 3D MVP transforms, directional lighting, and distance fog.
- [x] **Task 1.3**: Implement a flexible 3D `Mesh` and `Vertex` struct (`glm::vec3 position`, `glm::vec3 normal`, `glm::vec3 color`).
- [x] **Task 1.4**: Implement a 3D `Camera` system with perspective projection, smooth follow damping, and shake.
- [x] **Task 1.5**: Implement clean `Engine` (application lifecycle, delta-time game loop) and `Input` manager (key states, just-pressed detection).

## Phase 2: Player Starfighter & Flight Controller
- [x] **Task 2.1**: Generate procedural low-poly 3D Player Starfighter mesh (original interceptor design: wedge hull, swept wings, twin engines).
- [x] **Task 2.2**: Implement 3D flight mechanics: Pitch (up/down), Yaw (steering), dynamic banking/roll coupling.
- [x] **Task 2.3**: Implement corridor bounding constraints (clamping player smoothly inside the camera frustum).
- [x] **Task 2.4**: Implement Boost and Airbrake mechanics with camera FOV dynamic zoom.
- [x] **Task 2.5**: Implement tactical evasion spin (barrel roll) with roll animation and projectile deflection state.

## Phase 3: Weapons, Aiming Reticle & Combat
- [x] **Task 3.1**: Implement dual 3D targeting reticle (near crosshair and projected far convergence reticle).
- [x] **Task 3.2**: Implement dual plasma blaster cannon with projectile pooling and forward velocity.
- [x] **Task 3.3**: Implement laser tracers and muzzle flare effects.

## Phase 4: Rail Corridor System & Environment
- [x] **Task 4.1**: Implement Rail Path Manager (forward scrolling corridor through 3D space).
- [x] **Task 4.2**: Implement speed rings / energy gate obstacles to fly through.
- [x] **Task 4.3**: Implement low-poly asteroid field / space debris environment.
- [x] **Task 4.4**: Implement distance fog and lighting in shaders.

## Phase 5: Enemies, AI Patterns & Collisions
- [x] **Task 5.1**: Implement Bounding Sphere and AABB collision system (lasers vs enemies, player vs obstacles/rings).
- [x] **Task 5.2**: Implement Enemy Drone entities with wave flight patterns (swoop, strafe, fly-by).
- [x] **Task 5.3**: Implement particle burst explosion effects on impact/destruction.

## Phase 6: HUD & Game Loop Polish
- [x] **Task 6.1**: Implement on-screen HUD (Shield/Hull integrity, Boost energy, Score, Reticle).
- [x] **Task 6.2**: Implement game state management (Playing, Victory, Game Over, Restart).

## Phase 7: Advanced Combat & Ordnance (Charged Lock-on & Smart Bomb)
- [x] **Task 7.1**: Implement Charge Shot system (charging orb at starfighter nose, charging particle vortex, lock-on targeting on nearest enemy).
- [x] **Task 7.2**: Implement homing behavior and Area-of-Effect detonation for charged plasma blast.
- [x] **Task 7.3**: Implement Smart Bomb ordnance (launch torpedo, manual/impact detonation, expanding shockwave ring).
- [x] **Task 7.4**: Implement screen-clearing shockwave physics (cancelling enemy bullets, damaging surrounding enemies, screen shake).
- [x] **Task 7.5**: Update HUD with Smart Bomb stock counters and lock-on HUD brackets.

## Phase 8: Planetary Canyon Trench & Tunnel Environment
- [x] **Task 8.1**: Create procedural low-poly Canyon Trench mesh (jagged canyon floor and towering rock cliff walls) and geological Rock Archways.
- [x] **Task 8.2**: Create Hazard Pillars / Monolith obstacles standing in the flight corridor with destructible health and rock shatter debris.
- [x] **Task 8.3**: Integrate seamless chunk streaming in `WorldEnvironment` for canyon sections, arches, rings, and monoliths.
- [x] **Task 8.4**: Add collision interactions for hazard pillars (damage from player blaster, charged homing blast, smart bomb shockwave, and ship impact).
- [x] **Task 8.5**: Tune atmospheric canyon lighting and horizon fog palette for dramatic scale and speed.

## Phase 9: End-of-Stage Boss Encounter (Colossal Dreadnought)
- [x] **Task 9.1**: Build procedural 3D Colossal Dreadnought Boss mesh (armored warship hull, port/starboard turret batteries, shield generator dome, and glowing reactor core).
- [x] **Task 9.2**: Implement `BossDreadnought` class with multi-phase state machine (Phase 1: Turrets & Shields; Phase 2: Exposed Overheating Core; Phase 3: Death Spiral).
- [x] **Task 9.3**: Implement Boss bullet hell attacks (alternating heavy plasma bursts, 3-way spread barrages, and desperation fire).
- [x] **Task 9.4**: Integrate weakpoint lock-on targeting with player Charged Shots and Smart Bomb shockwave damage.
- [x] **Task 9.5**: Update HUD with Boss health bar, subsystem status readouts, Warning banner, and Mission Complete victory state.

## Phase 10: Audio & Sound Synthesizer System
- [x] **Task 10.1**: Implement `SoundManager` audio engine with low-latency multi-channel software mixer and Windows `waveOut` audio backend.
- [x] **Task 10.2**: Implement procedural synth DSP algorithms for retro arcade waveforms (sine, square, triangle, saw, white noise, frequency sweeps, ADSR envelopes).
- [x] **Task 10.3**: Synthesize procedural sound assets (Twin Lasers, Enemy Plasma, Charge Hum, Lock-On Ping, Smart Bomb Detonation, Barrel Roll Whoosh, Explosions, Ring Chime, Warning Siren, and Boost Roar).
- [x] **Task 10.4**: Hook audio triggers into game systems (weapons, starfighter maneuvers, collisions, pickups, and boss alert).
- [x] **Task 10.5**: Compile, test, and verify audio performance and latency.

## Phase 11: Wing Damage & Starfighter Breakdown System
- [x] **Task 11.1**: Modular Starfighter Mesh Decomposition & Tumbling Wing Debris: Decompose `PlayerStarfighter` into Fuselage, Left Wing, and Right Wing meshes; implement tumbling wing physics debris and trailing smoke/sparks.
- [x] **Task 11.2**: Asymmetrical Flight Physics & Weapon Degradation: Implement wing health states, asymmetric aerodynamic roll/yaw drag on severed wings, and single/center blaster firing logic.
- [x] **Task 11.3**: Wing-Specific Collision Detection & Metal Tearing Audio: Implement wingtip collision bounds for canyon walls, monoliths, and enemy fire; synthesize `WingSnap` tearing audio in `SoundManager`.
- [x] **Task 11.4**: Silver Repair Rings & Wing Restoration: Enhance silver rings to repair damaged/lost wings to 100%, extinguish smoke, and restore twin blasters.
- [x] **Task 11.5**: Cockpit HUD Wing Status Silhouette & Audio Alerts: Render real-time `[L-WING]` and `[R-WING]` status indicators on HUD with damage/lost colors and warning banners.
- [x] **Task 11.6**: Verification & Playtest Build: Compile, test, and verify debris tumbling, flight mechanics, collision shearing, and wing repair.
