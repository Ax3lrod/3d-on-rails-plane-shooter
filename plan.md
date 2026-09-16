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
