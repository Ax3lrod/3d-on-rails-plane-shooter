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

## Phase 12: Cockpit 1st-Person Mode & Dynamic Cinematic Camera (Option B)
- [x] **Task 12.1**: Implement First-Person Cockpit View mode toggle (`V` key) with interior canopy frame struts and glass reflections.
- [x] **Task 12.2**: Implement Cinematic Camera Director: Boss Intro cinematic sweep and Slow-Mo Orbit Kill-Cam on Dreadnought destruction.
- [x] **Task 12.3**: Update HUD for First-Person immersion (integrated artificial pitch horizon, canopy frame glare, and seamless reticle alignment).
- [x] **Task 12.4**: Compile, test, and verify Phase 12.

## Phase 13: All-Range Mode 360° Dogfight Arena & Acrobatics (Option C)
- [x] **Task 13.1**: Implement All-Range 360° dogfight arena physics transition at the boss arena with boundary warnings ("TURN BACK").
- [x] **Task 13.2**: Implement Evasive Acrobatics: Somersault Loop-de-loop (`S + Boost`) and U-Turn maneuver (`S + Brake`).
- [x] **Task 13.3**: Implement 360° Tactical Radar Minimap on HUD displaying player heading, arena perimeter, boss blip, and enemy bogeys.
- [x] **Task 13.4**: Compile, test, and verify Phase 13.

## Phase 14: Wingmen AI Squadron & Radio Chatter System (Option D)
- [x] **Task 14.1**: Implement `WingmanSquadron` with two wingmen (Echo-1 "Striker" and Echo-2 "Aegis"), formation flight AI, and dogfight targeting.
- [x] **Task 14.2**: Implement Wingman Distress events (enemy bogeys tailing wingmen, rescue rewards, radio distress alerts).
- [x] **Task 14.3**: Synthesize procedural 16-bit arcade radio static and speech babble in `SoundManager`.
- [x] **Task 14.4**: Implement HUD Vector Comms Box with animated pilot portraits, voice waveform bars, and dynamic dialog subtitle feed.
- [x] **Task 14.5**: Compile, test, and verify Phase 14.

## Phase 15: Procedural Chiptune/FM Stage Soundtrack & Boss BGM (Option A)
- [x] **Task 15.1**: Implement multi-channel software music sequencer in `SoundManager` with lead synth, bassline, arpeggio, and procedural percussion (kick, noise snare, hi-hat).
- [x] **Task 15.2**: Compose and synthesize Canyon Stage 1 BGM (driving 16-bit heroic sci-fi melody in D minor, 132 BPM).
- [x] **Task 15.3**: Compose and synthesize Dreadnought Boss Battle BGM (tense, rapid battle rhythm in F minor with escalating Phase 2 core intensity).
- [x] **Task 15.4**: Implement dynamic music state transitions (Title, Stage 1, Boss Phase 1, Boss Phase 2, Victory Fanfare) with cross-fade volume control.
- [x] **Task 15.5**: Compile, test, and verify Phase 15.

## Phase 16: Post-Processing Pipeline: Bloom Glow, Retro CRT Scanlines & Hyperspace Warp (Option B)
- [x] **Task 16.1**: Implement `PostProcessor` FBO pipeline (off-screen render target, HDR color texture, screen quad VAO).
- [x] **Task 16.2**: Implement Glowing Bloom Pass (bright color extraction for lasers/thrusters/explosions, 2-pass blur, additive blend).
- [x] **Task 16.3**: Implement Retro Arcade CRT shader (`F1` toggle) with scanlines, phosphor mask, barrel vignette curvature, and subtle chromatic aberration.
- [x] **Task 16.4**: Implement Hyperspace Warp Tunnel effect on stage clear (radial star streaks, FOV warp stretch, chromatic tunnel vortex).
- [x] **Task 16.5**: Compile, test, and verify Phase 16.

## Phase 17: Branching Mission Routes & Secret Objectives (Option C)
- [x] **Task 17.1**: Implement 3 Secret Planetary Radar Relays hidden inside rock archways along the canyon trench with destructible HP and HUD brackets.
- [x] **Task 17.2**: Implement Dual Mission Outcomes: "Mission Accomplished" (Normal route) vs "Mission Complete" (Hard route: all 3 relays destroyed + both wingmen intact).
- [x] **Task 17.3**: Implement Sector 2: Deep Space Debris Field (open starfield, dense asteroid cluster storm, swirling nebulae, elite stealth fighters).
- [x] **Task 17.4**: Implement seamless warp jump transition between Canyon Sector 1 and Space Sector 2.
- [x] **Task 17.5**: Compile, test, and verify Phase 17.

## Phase 18: 3D Hangar Title Screen, Settings Menu & High-Score Hall of Fame (Option D)
- [x] **Task 18.1**: Implement 3D Hangar Title Screen with rotating turntable starfighter pad, hangar neon lights, and pilot status readouts.
- [x] **Task 18.2**: Implement Settings Menu (Master Volume, Music Volume, SFX Volume, Invert Y-Axis, CRT Filter toggle, View Mode default).
- [x] **Task 18.3**: Implement Local High-Score Hall of Fame persistence (`scores.dat` binary save/load, pilot callsigns, stage medals).
- [x] **Task 18.4**: Wire full game state machine: Title -> Briefing -> Playing -> Stage Clear / Warp -> Game Over / Victory -> Leaderboard.
- [x] **Task 18.5**: Final end-to-end playtest, compilation, and verification.

## Phase 19: Ex-Zodiac Camera & Flight Physics Overhaul
- [x] **Task 19.1**: Implement Elastic Camera Spring Lag & Decoupled Frustum Traversal (camera tracks player with ~45% ratio, allowing the ship to swoop freely across screen corners).
- [x] **Task 19.2**: Implement Dynamic Camera Roll Tilt & Pitch Horizon Anticipation (smooth 10°-12° roll tilt into banking turns and pitch look-ahead).
- [x] **Task 19.3**: Polish Starfighter Flight Physics: snappy banking transitions, lateral inertia, and responsive centering spring.
- [x] **Task 19.4**: Implement Low-Altitude Surface Spray / Ground Plume FX (vertical spray fan trailing beneath the starfighter when flying near the ground).
- [x] **Task 19.5**: Compile, test, and verify flight controls and camera dynamics.

## Phase 20: Ex-Zodiac HUD & UI Overhaul
- [x] **Task 20.1**: Implement Slanted Parallelogram Gauges for Hull Shield and Boost Energy with color gradients (amber/green/cyan).
- [x] **Task 20.2**: Implement Diamond Smart Bomb Stock Pips and Restyled Reticle Brackets matching Ex-Zodiac HUD.
- [x] **Task 20.3**: Implement Hit Combo Chain Counter (`HIT x3`, `x5`, `COMBO!`) with multiplier score popups and reward chimes.
- [x] **Task 20.4**: Restyle On-Screen Boss Health Bar and Wingman Comms Card with sleek arcade bezel frames.
- [x] **Task 20.5**: Compile, test, and verify the HUD overhaul.

## Phase 22: Full Gamepad / Controller Integration (Option C)
- [x] **Task 22.1**: Implement GLFW Gamepad Polling & Deadzone Processing in `Input` (Left stick analog flight, triggers, bumpers, face buttons, d-pad).
- [x] **Task 22.2**: Integrate Analog Flight Steering & Triggers in `PlayerStarfighter` (smooth analog pitch/yaw, RT boost, LT brake, LB/RB barrel roll).
- [x] **Task 22.3**: Implement Windows XInput Controller Rumble & Haptic Feedback in `Input` (blaster fire, boost roar, damage impacts, bomb shockwave).
- [x] **Task 22.4**: Update HUD & Menu Input Prompts for dynamic Gamepad / Keyboard prompt switching.
- [x] **Task 22.5**: Compile, test, and verify gamepad controls and vibration.

## Phase 23: Retro Arcade Visual FX & Ground Shadow Projection (Option D)
- [x] **Task 23.1**: Implement Dynamic Ground Shadow Projection below starfighter and low-flying enemy drones in Sector 1.
- [x] **Task 23.2**: Implement Water Ripple Rings and Enhanced Surface Spray Plume when skimming low over canyon water.
- [x] **Task 23.3**: Implement Critical Hull Damage FX (billowing black smoke and flame sparks for critical shield < 25% and severed wings).
- [x] **Task 23.4**: Implement Flat-Faceted Arcade Shading Mode in `default.frag` with Settings toggle (`[ARCADE FLAT]` vs `[SMOOTH]`).
- [x] **Task 23.5**: Compile, test, and verify Phase 22 and Phase 23 in Release build.

## Phase 24: True Retro Pixel Resolution (Low-Res Framebuffer) & 4x4 Ordered Bayer Dithering
- [x] **Task 24.1**: Implement retro low-resolution internal render target (480x270 / 16:9) in `PostProcessor` with `GL_NEAREST` filtering for authentic chunky polygon pixelation.
- [x] **Task 24.2**: Implement 4x4 Ordered Bayer Dithering in `default.frag` for fog distance fade and dithered transparency stippling.
- [x] **Task 24.3**: Add Settings menu toggle: `RENDER RESOLUTION: [RETRO ARCADE (480x270)] / [HD CRISP (1080p)]`.
- [x] **Task 24.4**: Compile, test, and verify retro pixel scaling and dithering.

## Phase 25: Ex-Zodiac Reticle Palette & Signature Violet Starfighter Livery
- [x] **Task 25.1**: Restyle `TargetingReticle` in `Mesh` and `CombatSystem` with Red/Salmon outer brackets `[ ]` (`vec3(0.96, 0.24, 0.32)`) and bright Cyan inner crosshair (`vec3(0.24, 0.90, 1.0)`).
- [x] **Task 25.2**: Update Player Starfighter default livery to Ex-Zodiac Signature: Royal Violet fuselage with Cyan wingtips and golden canopy glass.
- [x] **Task 25.3**: Compile, test, and verify reticle and ship livery visuals.

## Phase 26: Rooster Tail Water Spray Plume & Coastline Sea Environment
- [x] **Task 26.1**: Implement vertical Rooster Tail Water Spray Plume in `ParticleSystem`: high-density upward jet of white/cyan water foam trailing behind the craft when flying near water.
- [x] **Task 26.2**: Enhance Sector 1 environment: azure water surface, sandstone/pastel pink sea-cliffs with natural archways, and floating dome pavilions.
- [x] **Task 26.3**: Compile, test, and verify water spray plumes and coastline terrain.

## Phase 27: Retro Anime Pilot Comms Portrait & Story Dialogue
- [x] **Task 27.1**: Implement high-detail pixel-art anime pilot portrait in `HUD::DrawCommsBox` matching `exzodiac2.png` (dark blue hair, gold accessories, animated eyes/mouth speaking).
- [x] **Task 27.2**: Add narrative flight banter and radio calls triggered at key sector landmarks (low skim, archways, boss encounter).
- [x] **Task 27.3**: Compile, test, and verify comms portrait and dialogue.

## Phase 28: Rotating Wind Turbines & Boss Dual Spiked Mace Arms
- [x] **Task 28.1**: Create 3D Wind Turbine / Windmill mesh with rotating 3-blade propeller and place along plains/cliffs matching `exzodiac3.png`.
- [x] **Task 28.2**: Upgrade Boss Dreadnought with dual rotating Spiked Mace / Flail arms in front and glowing dual plasma thruster exhausts.
- [x] **Task 28.3**: Compile, test, and verify windmill animation and boss weapon mechanics.

## Phase 29: Final Playtesting, Verification & Git Remote Sync
- [x] **Task 29.1**: Full Release compilation and verification.
- [x] **Task 29.2**: End-to-end playtest of flight, visuals, and boss battle.
- [x] **Task 29.3**: Stage, commit, and push all changes to GitHub remote repository.

