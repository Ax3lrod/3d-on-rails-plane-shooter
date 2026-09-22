# Design Pattern: Boss Arena & All-Range Transition

> **Inspired by**: *Star Fox 64* (Granga / Katina Boss Arenas)  
> **Classification**: Level Climax & Mode Shift  
> **Status**: Implemented & Verified

---

## 1. Purpose & Fantasy
To dramatically elevate the stakes by transitioning the gameplay from linear rail scrolling into an open 360° dogfight arena. The player is given full spatial agency to outmaneuver a massive opponent.

---

## 2. Player Experience & Sequence
1. **The Approach**: After surviving the corridor gauntlet, the corridor buildings open up into an expansive industrial plaza or open field.
2. **Cinematic Intro**:
   - The rail lock releases.
   - Camera initiates a dramatic cinematic sweep around the emerging boss while the boss health bar fills with a warning siren.
   - Radio banter delivers the boss's menacing challenge or squadmate tactical advice.
3. **All-Range Dogfight**:
   - HUD switches to 360° tactical radar.
   - Boundary alert system activates: straying past the arena radius ($280\text{ units}$) triggers a visual `"TURN BACK"` warning and auto-course correction.
   - Player utilizes acrobatics: Somersault (`S` + Boost) to loop behind the boss, or U-Turn (`S` + Brake) to reverse heading.
4. **Targeted Weakpoint Destruction**:
   - Rather than bullet-sponging, the boss has multi-part vulnerability (e.g. knee joints triggering staggers to reveal rear coolant vents).
5. **Climax & Victory Cam**:
   - On HP reaching 0, time slows into a slow-motion orbital camera sweep around the exploding colossus, followed by stage medal tally and hyperspace jump.

---

## 3. Design Constraints & Common Failure Modes
- **Disorientation**: Without clear environmental landmarks (e.g. distant horizon mountains or central structures), players can become disoriented in 360° flight.
- **Crosshair Alignment**: In all-range flight, the reticle must smoothly track the camera forward vector rather than snapping awkwardly.

---

## 4. Required Systems
- `PlayerStarfighter::SetAllRangeMode(bool active, glm::vec3 center, float radius)`
- `Camera::StartBossIntro(glm::vec3 bossPos, glm::vec3 playerPos)`
- `IBoss`: Subsystem health, attack state machine, and weakpoint multipliers.
- `HUD`: 360° radar blips, boss health bar, and turn-back warning banner.

---

## 5. Timeline JSON Example
```json
{
  "trigger_z": -13000.0,
  "type": "boss_trigger"
}
```
