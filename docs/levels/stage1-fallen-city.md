# Stage 1: Fallen City — Level Design Specification

> **Theme**: Dark Brutalist Metropolis / War-Torn Ruins  
> **Boss**: Iron Colossus (`BossWalkingRobot`)  
> **Target Duration**: ~10 Minutes ($Z = 0$ to $Z = -13,000$)  
> **Data File**: `resource/stages/stage1.json`

---

## 1. Stage Vision & Fantasy
The player's starfighter squadron is deployed into the smoking ruins of a fallen metropolis overrun by mechanized invaders. Towering brutalist skyscrapers with neon window strips flank a cracked asphalt highway. The mission culminates in an expansive open plaza where an imposing bipedal walking colossus stands guard.

---

## 2. 5-Act Encounter Choreography (Beat Sheet)

```
[Act 1: Outskirts] ──> [Act 2: Suburbs] ──> [Act 3: Rescue] ──> [Act 4: Downtown Gauntlet] ──> [Act 5: Colossus Arena]
(Z: 0 to -2,500)      (Z: -2,500 to -4,500) (Z: -4,500 to -6,500)  (Z: -6,500 to -11,000)     (Z: -11,000 to -13,000+)
```

### Act 1: The Outskirts Approach ($Z = 0$ to $-2,500$)
- **Pacing**: Moderate / Warm-Up (Tension: 2/5).
- **Player Experience**: Calibrating aim and flight handling. Scattered rubble piles, low-density buildings.
- **Key Encounters**:
  - $Z = -300$: Squadron check-in radio chatter (`"Echo squadron, form up!"`).
  - $Z = -800$: V-formation scout drones introducing hit combo timing.
  - $Z = -1500$: Staggered drone wave; first silver recovery ring.
  - $Z = -2200$: First ground armor column (2 tanks on highway). Teaches pitch-down aiming.

### Act 2: Suburban Escalation ($Z = -2,500$ to $-4,500$)
- **Pacing**: Escalating (Tension: 3.5/5).
- **Player Experience**: The corridor narrows; building height increases. Ground armor coordinates with aerial flak.
- **Key Encounters**:
  - $Z = -2800$: Ground tank + Drone crossfire pincer.
  - $Z = -3500$: Rooftop flak turrets firing 3-round bursts; requires barrel roll deflection.
  - $Z = -4100$: Combined arms wave (Ground tank + Flak battery + Interceptor fly-by).

### Act 3: Striker Rescue & Tactical Respite ($Z = -4,500$ to $-6,500$)
- **Pacing**: Spike into Breathing Room (Tension: 4.5/5 $\to$ 1.5/5).
- **Player Experience**: High-stakes squadmate protection followed by a calm recovery stretch.
- **Key Encounters**:
  - $Z = -4500$: **Wingman Rescue Event**: Striker ambushed by 3 Ace Interceptors. Radio distress trigger (`"Get this guy off my tail!"`).
  - $Z = -4700$: Rescuing Striker yields combat supply cache (Dual Blaster upgrade, Shield ring, 5,000 pts).
  - $Z = -5200$ to $-6200$: **Breathing Room Section**: Sparse drone lines, scenic view of city ruins, supply pickups, allowing shield and energy regeneration before the gauntlet.

### Act 4: Downtown Canyon & The Collapsing Spire ($Z = -6,500$ to $-11,000$)
- **Pacing**: Maximum Corridor Intensity (Tension: 5/5).
- **Player Experience**: Dense urban canyon with 75m high-rises. Complex obstacles and reflex tests.
- **Key Encounters**:
  - $Z = -6800$: **Collapsing Spire Set Piece**: Siren wails; explosive charges shatter support struts; massive antenna spire tilts 45° across highway. Player must boost underneath, bank through side gap, or destroy it with ordnance (350 HP).
  - $Z = -7800$: Elite Interceptor squadron executing aggressive barrel rolls.
  - $Z = -8800$ to $-10500$: Heavy industrial fortress district; overlapping flak batteries and armored ground tanks.

### Act 5: Boss Approach & Colossus Arena ($Z = -11,000$ to $-13,000+$)
- **Pacing**: Atmospheric Buildup into Epic Climax.
- **Player Experience**: Buildings peel away into an open battlefield plaza. Horizon opens up to distant mountain silhouettes.
- **Key Encounters**:
  - $Z = -12000$: Distant heavy stomping tremors felt in camera shake; red klaxon warnings.
  - $Z = -13000$: **Mode Shift: All-Range Arena Battle**:
    - Camera transitions to cinematic boss intro sweep.
    - Colossus steps into the arena center with shoulder cannons charged.
    - Player granted 360° flight, somersaults, and tactical radar.

---

## 3. Boss Design: Iron Colossus (`BossWalkingRobot`)

### Multi-Part Tactical Mechanics:
1. **Left & Right Knee Joints (250 HP each)**:
   - Heavily armored leg joints vulnerable to sustained blaster fire or charged shots.
   - Shattering a knee causes the Colossus to violently stagger and kneel for 5.0 seconds with heavy smoke and electrical sparks.
2. **Rear Coolant Exhaust (Exposed Weakpoint)**:
   - When staggered, the glowing coolant manifold on the Colossus's rear opens, granting a **4.0x damage multiplier**!
   - Skilled players somersault or boost behind the kneeling boss to unleash rapid charged shots.
3. **Escalating Boss Phases**:
   - **Phase 1 (100% - 70% HP)**: Prowls laterally; fires alternating shoulder cannon shells.
   - **Phase 2 (70% - 35% HP)**: Hydraulic stomps release radial shockwave missile fans across the arena floor.
   - **Phase 3 (< 35% HP)**: Overdrive mode; rapid twin cannon barrages with aggressive tracking.
