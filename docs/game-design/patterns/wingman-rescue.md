# Design Pattern: Wingman Rescue

> **Inspired by**: *Star Fox 64* (Corneria Falco / Slippy rescue)  
> **Classification**: Dynamic Tactical Encounter  
> **Status**: Implemented & Verified (Phase 38)

---

## 1. Purpose & Fantasy
To inject immediate tension, test player marksmanship under pressure, and humanize the squadron. Transforming wingmen from background scenery into active squadmates who can be saved or lost.

---

## 2. Player Experience & Sequence
1. **The Ambush**: Wingman enters the corridor under fire, screaming over the radio (`"Get this guy off my tail!"`).
2. **Visual Telegraph**: 2–3 enemy ace interceptors lock in a tight pursuit formation directly behind the wingman, with red targeting beams or laser tracers.
3. **The Choice**:
   - Player can ignore them and focus on score/fodder, letting the wingman take heavy hull damage or retreat.
   - Player can boost forward, line up the pursuers, and shoot them down with blasters or a lock-on Charged Shot.
4. **Resolution**:
   - **Success**: Wingman breaks into an evasive loop, thanks the player over the radio, and drops a high-value supply crate (Dual Blaster / Shield Ring / Bomb + 5,000 pts).
   - **Failure**: Wingman's ship begins smoking and retreats from combat for the remainder of the stage.

---

## 3. Design Constraints & Common Failure Modes
- **Readability**: Pursuers must fly on a predictable trajectory so the player can actually lead shots without random erratic jerking.
- **Reaction Window**: The player must be given at least 4–6 seconds of chase time before the wingman's health depletes to zero.
- **Reward Significance**: Saving a wingman must feel worth the risk (granting weapon upgrades or massive bonus score).

---

## 4. Required Systems
- `WingmanSquadron`: Tracks wingman state, distress flag, health, and rescue rewards.
- `EnemyManager`: Spawns pursuit formation targeting wingman position.
- `LevelTimeline`: Triggers event via `wingman_rescue` at scripted $Z$.
- `HUD`: Displays vector portrait distress alert.

---

## 5. Timeline JSON Example
```json
{
  "trigger_z": -4500.0,
  "type": "wingman_rescue",
  "target_wingman": "Striker",
  "pursuer_count": 3
}
```
