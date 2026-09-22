# Design Pattern: Collapsing Environmental Hazard

> **Inspired by**: *Star Fox 64* (Corneria collapsing highway arch & stone pillars)  
> **Classification**: Environmental Set Piece  
> **Status**: Implemented & Verified (Phase 38)

---

## 1. Purpose & Fantasy
To break the monotony of empty sky corridors, showcase the physical destruction of the war-torn environment, and force the player to make instantaneous spatial decisions at high flight speeds ($21\text{ units/s}$).

---

## 2. Player Experience & Sequence
1. **Telegraphing**: As the player approaches within $250\text{ units}$, a mechanical warning klaxon sounds, and support girders emit explosive fire puffs and sparks.
2. **Dynamic Physics**: The tower / spire tilts 45° across the flight corridor, sweeping downwards over a 2.5-second arc.
3. **Player Tactical Options**:
   - **Boost Under**: Floor the throttle (`Space` / `RT`) to slip beneath the collapsing girder before it blocks the road.
   - **Bank Over / Around**: Roll 90° vertically and bank hard through the gap between the building flank and the leaning spire.
   - **Demolish with Ordnance**: Unleash blasters or a Smart Bomb to destroy the structure's health (350 HP), causing it to explode into harmless shrapnel and awarding 2,500 points.

---

## 3. Design Constraints & Common Failure Modes
- **Avoid "Gotcha" Deaths**: A hazard must never collapse instantaneously without audio/visual warning cues. The player must see it start to tilt while still far enough away to react.
- **Fair Hitboxes**: Bounding collision geometry must match the visible mesh, avoiding invisible collision hulls that clip player wingtips.
- **Framerate Stability**: Particle bursts during collapse must be pooled to prevent framerate stuttering during critical evasion moments.

---

## 4. Required Systems
- `WorldEnvironment`: `AntennaSpireHazard` state machine, toppling rotation math, and AABB/cylinder collision.
- `SoundManager`: Klaxon siren and structural metal tearing SFX.
- `LevelTimeline`: Triggers event via `hazard_spire` at scripted $Z$.

---

## 5. Timeline JSON Example
```json
{
  "trigger_z": -6800.0,
  "type": "hazard_spire",
  "hp": 350.0
}
```
