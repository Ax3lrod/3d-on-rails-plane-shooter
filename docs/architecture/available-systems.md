# Available Game Systems & Asset Inventory

> **Last Updated**: Phase 39  
> **Source of Truth**: Codebase inspection via `graphify` and `headers/`  
> **Purpose**: AI agents must verify existing systems against this inventory before proposing or implementing stage content.

---

## 1. Player Starfighter Subsystems (`PlayerStarfighter.h`)

| Subsystem | State / Mechanic | Controls | Notes / Limitations |
| :--- | :--- | :--- | :--- |
| **Primary Blasters** | Twin Plasma Cannons | `Left Click` / `Gamepad A` | Fires dual forward lasers. If a wing is severed, automatically falls back to single center blaster. |
| **Charged Shot** | Homing Lock-on Blast | Hold & release fire | Charges blue plasma vortex at nose; locks onto nearest enemy/boss weak point; deals AoE splash damage on impact. |
| **Smart Bomb** | Screen-clearing Torpedo | `Right Click` / `Gamepad B` | Detonates on command or impact; clears enemy projectiles and damages all surrounding entities. Pips shown on HUD. |
| **Boost** | Dynamic forward thrust | `Space` / `Gamepad RT` | Drains boost meter; increases forward velocity; zooms camera FOV outwards. |
| **Airbrake** | Reverse thrust | `Left Shift` / `Gamepad LT` | Slows forward corridor flight; tightens camera FOV for precise targeting. |
| **Barrel Roll** | Tactical projectile deflection | Double-tap `Q`/`E` or `LB`/`RB` | Spins ship 360°; repels enemy blaster bolts for 0.45s. |
| **Somersault** | Loop-de-loop evasion | `S` + Boost (All-Range) | Available in All-Range mode. Performs vertical loop, placing ship behind pursuers. |
| **U-Turn** | 180° turnaround | `S` + Brake (All-Range) | Available in All-Range mode. Flips ship heading 180° instantly. |
| **Wing Health** | Asymmetric damage model | Collisions / enemy fire | Left and Right wings have independent HP. Severed wings cause aerodynamic drag/pull. Restored by Silver Rings. |
| **Cockpit View** | First-person camera mode | `V` key | Switches between 3rd-person chase cam and 1st-person interior canopy cockpit with HUD horizon. |

---

## 2. Combat & Enemy Subsystems (`EnemyManager.h`)

| Enemy Archetype | Mesh Factory | Behavior & Flight Pattern | Positioning & Role |
| :--- | :--- | :--- | :--- |
| **`EnemyType::Drone`** | `Mesh::CreateDrone` | Swoop down, strafe across corridor, fly-by, or fire single blaster bolt. | Air ($Y = 0$ to $12$). Fast fodder waves. |
| **`EnemyType::GroundTank`** | `Mesh::CreateGroundTank` | Drives along ground tracks; elevates turret to fire arcing explosive shells at player. | Ground ($Y = -7.5$, $X \in [-15, 15]$). |
| **`EnemyType::FlakTurret`** | `Mesh::CreateFlakTurret` | Rotating defense battery mounted on roads or building rooftops; fires 3-round flak bursts. | Ground or rooftops ($Y = -7.5$ or $Y = 15$). |
| **`EnemyType::EliteInterceptor`** | `Mesh::CreateEliteInterceptor` | High-speed agility; barrel rolls to evade player fire; fires burst lasers; actively pursues wingmen. | Air ($Y = 2$ to $16$). Squad leaders and dogfight aces. |

### Choreographed Formations:
- `v_formation`: 3 or 5 ships flying in arrowhead formation.
- `staggered`: Successive echelon waves with delay.
- `line`: Horizontal or vertical sweeping line.
- `pincer`: Two groups attacking simultaneously from left and right corridor flanks.
- `pursuit`: Elite fighters spawned directly behind a wingman target.

---

## 3. Boss Archetypes (`IBoss.h`)

| Boss Class | Stage / Sector | Key Weakpoints & Tactics | Attack Patterns |
| :--- | :--- | :--- | :--- |
| **`BossWalkingRobot`** | Sector 1 (Fallen City) | **Left & Right Knee Joints** (250 HP each). Breaking knee staggers boss for 5s, exposing **Rear Coolant Exhaust** for 4.0x critical damage. Head core = 2.0x damage. | Shoulder cannons, ground shockwave missile fan (Phase 2), rapid double cannon fire (Phase 3). |
| **`BossDreadnought`** | Sector 1 (Canyon) / Sector 5 | Port/Starboard shield domes, turret batteries, and overheating central core. Front spiked mace flails. | 3-way plasma spread, flail sweeps, missile barrages, core desperation laser. |
| **`BossTwinHelicopters`** | Sector 2 (Railway) | Rotor hubs and cockpit domes. Pincer coordination between Gunship Alpha & Beta. | Crossfire vulcan strafes, homing missile swarms, enrage ramming runs. |
| **`BossMegaTank`** | Sector 3 (Iron Fortress) | 4 destructible tread assemblies to immobilize, followed by rear exhaust manifold. | Heavy rail cannon shells, flame throwers, mine dispersal. |
| **`BossMechaWorm`** | Sector 4 (Dune Pass) | Segmented armored carapaces and glowing maw during aerial breach loops. | Subterranean burrowing geysers, rock throwing, aerial arched breaches. |

---

## 4. World Environment & Decorative Props (`WorldEnvironment.h`)

| Prop / Theme | Mesh Function | Collision / Gameplay Effect |
| :--- | :--- | :--- |
| **City Road & Buildings** | `CreateCityRoad`, `CreateBuilding` | 5 brutalist building variants with neon windows lining the corridor flanks ($X < -42$ and $X > 42$). Rubble piles. |
| **Collapsing Spire** | Procedural antenna spire | **Dynamic Set Piece**: At $Z = -6800$, triggers warning siren, explosions, and topples 45° across the road. Destructible (350 HP) or avoidable via boost/banking. |
| **Wind Turbines** | `CreateWindTurbineTower`, `...Blades` | Spinning 3-blade wind generators scattered along hills and plains. Destructible blades. |
| **Pine Trees / Foliage** | `CreateLowPolyTree` | Low-poly vegetation scattered on countryside / open field terrain. |
| **Desert Cacti & Pyramids** | `CreateCactus`, `CreateDesertPyramid` | Biome props for desert / dune pass stages. |
| **Ice Crystals** | `CreateIceCrystal` | Clustered hexagonal spires for glacial sectors. |
| **Armored Train** | `TrainConvoy` (`CreateTrainLocomotive`) | Track-following locomotive and cargo containers on railway tracks ($X = 24$, $Y = -7.5$). Destructible cargo. |
| **Horizon Mountains** | `CreateBiomeHorizon` | Multi-layered, flat-shaded horizon silhouette (Canyon, City Skyline, Desert Pyramids, Basalt Towers). |

---

## 5. Wingman Squadron & Narrative Comms (`WingmanSquadron.h`, `HUD.h`)

- **Squadmates**:
  - Echo-1 "Striker" (Aggressive dogfighter, calls for assistance when ambushed).
  - Echo-2 "Aegis" (Defensive wingman, alerts player to incoming hazards and boss weakpoints).
- **Interactive Rescue Event (`TimelineEventType::WingmanRescue`)**:
  - Scripted ambush where 3 Ace Interceptors lock onto a wingman.
  - Wingman shield drains while pursued.
  - Successful rescue restores wingman shield, triggers voice chatter, and drops a high-tier supply cache (Dual Laser / Shield Ring / Bomb + 5,000 pts).
- **Comms Card**: 16-bit retro anime vector portrait, speaking mouth sync, voice waveform oscilloscope, and callsign title banner.

---

## 6. Timeline Event Types (`LevelTimeline.h`)

All stage timeline JSON files (`resource/stages/stage*.json`) support these validated event types:

```json
{
  "events": [
    {
      "trigger_z": -1200.0,
      "type": "enemy_wave",
      "count": 4,
      "spacing": 1.2,
      "spawn_x": 0.0,
      "formation": "v_formation",
      "enemy_type": "drone"
    },
    {
      "trigger_z": -4500.0,
      "type": "wingman_rescue",
      "target_wingman": "Striker",
      "pursuer_count": 3
    },
    {
      "trigger_z": -6800.0,
      "type": "hazard_spire",
      "hp": 350.0
    },
    {
      "trigger_z": -7000.0,
      "type": "dialogue",
      "sender": "Striker",
      "message": "Watch that falling spire! Punch your boost!"
    },
    {
      "trigger_z": -13000.0,
      "type": "boss_trigger"
    }
  ]
}
```
