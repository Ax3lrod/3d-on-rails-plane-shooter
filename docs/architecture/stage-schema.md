# Stage Timeline Data Schema & Validation Rules

> **Path**: `resource/stages/stage<N>.json`  
> **Consumer**: `LevelTimeline.cpp`  
> **Schema Version**: 1.0

---

## 1. Top-Level Object

```json
{
  "stage_id": "sector_1",
  "stage_name": "FALLEN CITY",
  "terrain_theme": "city",
  "boss_type": "walking_robot",
  "events": [ ... ]
}
```

### Required Fields:
- `stage_id` (string): Unique identifier (e.g., `"sector_1"`).
- `stage_name` (string): Display name on briefing HUD.
- `terrain_theme` (string): Must be one of `"city"`, `"canyon"`, `"railway_canyon"`, `"iron_fortress"`, `"dune_pass"`, `"glacial"`.
- `boss_type` (string): Must be one of `"walking_robot"`, `"dreadnought"`, `"twin_helicopters"`, `"mega_tank"`, `"mecha_worm"`.
- `events` (array of Event objects): List of timeline events sorted in **descending order of `trigger_z`** (e.g., $-500.0, -1200.0, \dots, -13000.0$).

---

## 2. Event Types & Schemas

### 2.1 `enemy_wave` / `formation`
Spawns a synchronized group of enemy craft or ground units.

```json
{
  "trigger_z": -1500.0,
  "type": "enemy_wave",
  "count": 4,
  "spacing": 1.5,
  "spawn_x": -10.0,
  "formation": "v_formation",
  "enemy_type": "drone"
}
```
- `trigger_z` (float): Z position threshold where player triggers the spawn.
- `count` (int): Number of enemies in the wave (1–8).
- `spacing` (float): Delay in seconds or spatial gap between spawns.
- `spawn_x` (float): Center X offset across corridor ($X \in [-25.0, 25.0]$).
- `formation` (string): `"v_formation"`, `"line"`, `"staggered"`, `"pincer"`, `"single"`.
- `enemy_type` (string): `"drone"`, `"ground_tank"`, `"flak_turret"`, `"elite_interceptor"`.

### 2.2 `dialogue`
Displays a pilot comms card on HUD and plays synthesized radio voice babble.

```json
{
  "trigger_z": -3200.0,
  "type": "dialogue",
  "sender": "Striker",
  "message": "Heavy flak fire ahead! Keep your shields up!"
}
```
- `sender` (string): `"Striker"`, `"Aegis"`, `"Control"`, or `"Boss"`.
- `message` (string): Subtitle text (keep under 60 characters for crisp readability).

### 2.3 `wingman_rescue`
Triggers an interactive tactical wingman distress encounter.

```json
{
  "trigger_z": -4500.0,
  "type": "wingman_rescue",
  "target_wingman": "Striker",
  "pursuer_count": 3
}
```
- `target_wingman` (string): `"Striker"` or `"Aegis"`.
- `pursuer_count` (int): Number of enemy ace pursuers (typically 2–3).

### 2.4 `hazard_spire`
Activates an environmental obstacle set piece (e.g. collapsing radio spire).

```json
{
  "trigger_z": -6800.0,
  "type": "hazard_spire",
  "hp": 350.0
}
```
- `hp` (float): Health of the collapsing structure. If reduced to 0 by player fire, shatters into harmless debris; otherwise, topples across the highway requiring flight evasion.

### 2.5 `boss_trigger`
Terminates the corridor scrolling phase and initiates the arena transition.

```json
{
  "trigger_z": -13000.0,
  "type": "boss_trigger"
}
```
- Switches player to **All-Range Mode** with bounding arena radius $\sim 280\text{ units}$.
- Starts the dynamic boss intro cinematic camera sweep.
