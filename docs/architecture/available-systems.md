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
| **Emerald Coastline & Archipelago** | `CreateCoastlineTerrain`, `CreateOceanIsland` | Zone A ($Z = 0 \to -2200$): Vast open ocean water floor ($X \in [-220, 220]$) with retro-arcade azure/cyan water bands and foam crests, dotted with scattered tropical archipelago islands (sandy beaches, rocky ledges, emerald plateaus, palm trees) and hovering reward rings. Rooster tail spray triggers on low water skim. |
| **Winding Coastal Highway Transition** | `CreateCoastalHighwayTransition` | Transition ($Z = -2200 \to -2800$): 600m continuous cinematic set-piece sweeping gracefully in from far off-screen right ($X = 145 \to 0$) over the turquoise open bay into the central flight corridor. Features vast blue water floor ($X \in [-220, 220]$, sloping $Y = -7.5 \to -18.0$), concrete pier pylons, streetlights, and a scenic rocky cape with an octagonal red-and-white lighthouse with radiant glowing beacon. Zero tunnel portal arches or rock collisions. |
| **Winding Elevated Highway & Suspension Bridge** | `CreateElevatedHighway` | Zone B ($Z = -2800 \to -5000$): 2200m continuous winding coastal causeway viaduct ($X(t) = 16.0 \sin(3\pi t)\sin(\pi t)$) raised over an expansive deep-blue bay water floor ($X \in [-220, 220]$, $Y = -18.0$). Features crash guardrails, steel girders, dual concrete pylons, streetlights, and two soaring 38m suspension bridge A-frame cable towers ($Z = -3425$ and $-4375$) with red aviation warning beacons and fanned steel tension cables. |
| **Naval Sea Fortress** | `CreateNavalFortress` | Zone B ($Z = -2800 \to -5000$): Offshore hexagonal reinforced concrete bastion rising from bay water ($Y = -18.0$) at $X = \pm 70 \dots 90$ with helipad markings, armored command bunker, rotating radar dish, and blinking amber beacon. |
| **Cruising Container Cargo Ship** | `CreateCargoShip` | Zone B ($Z = -2800 \to -5000$): Low-poly 72m container freighter slowly cruising forward at ~3.0 units/s in bay waters at $X = \pm 70 \dots 95$ with dark red antifouling bottom, black topsides, white deckhouse superstructure, twin smokestacks, and multi-colored container stacks. |
| **City Cross-Street Intersections** | `CreateCityIntersection` | Zone C ($Z = -5000 \to -8500$): Grand 4-way cross-avenue intersections at $Z = -5400, -5940, -6480, -7020, -7560, -8160$. Perpendicular avenues cut out to $X = \pm 180\text{m}$ with yellow centerlines, zebra crosswalks, sidewalk plazas, and 4 cantilever traffic signal heads (illuminated red/yellow/green lenses and cyan street signs). |
| **Sky-High Concrete Jungle Towers** | `CreateBuilding` | Zone C ($Z = -5000 \to -8500$): Towering 8-variant metropolis including colossal 125m, 150m, and 165m skyscrapers lining the corridor, creating a claustrophobic concrete canyon where the starfighter flies right through the middle. |
| **Sheared Ruined Skyscraper** | `CreateRuinedBuilding` | Zone C ($Z = -5000 \to -8500$): 110m half-destroyed tower catastrophically sheared at a 30° angle midway up, exposing internal concrete floor slabs, glowing embers, blast scorch marks, and twisted steel rebar spikes, flanked by concrete rubble mounds. |
| **Cyberpunk Billboard Towers** | `CreateCyberBillboardBuilding` | Zone C ($Z = -5000 \to -8500$): 145m corporate monolith with rooftop helipad and communications spire, featuring a giant holographic billboard ($32\text{m} \to 74\text{m}$) with 3 retro ad variants (Cyber Arrow Nova, Titan Heavy Dynamics, Hyperion Starburst). |
| **Elevated Corridor Skybridges** | `CreateSkybridge` | Zone C ($Z = -5000 \to -8500$): Multi-level illuminated glass-and-steel skybridge tubes at $Z = -5640, -6240, -7260, -7860$ spanning across the highway corridor ($X \in [-38, 38]$) at $Y = 6.5\text{m} \to 12.5\text{m}$, with panoramic glowing windows, hazard chevrons, and pulsing aviation warning strobes. Player can fly under or climb over. |
| **Sunken Canal & Aqueduct** | `CreateCityCanal` | Zone D ($Z = -8500 \to -11000$): Central industrial water channel with 35° concrete dyke walls, hazard stripes, and overhead floodgate beams. |
| **Colossus Arena Plaza** | `CreateArenaPlaza`, `CreateArenaPillar` | Zone E ($Z \le -11000$): Grand octagonal arena floor with 16 colossal perimeter floodlight columns for 360° All-Range combat. |
| **Collapsing Spire** | `CreateCollapsingSpire` | **Dynamic Set Piece**: At $Z = -6800$ & $Z = -10800$, triggers warning siren, sparks, and topples 45° across the road. Destructible (350 HP) or avoidable via boost/banking. |
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
