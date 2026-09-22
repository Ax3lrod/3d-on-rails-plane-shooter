# Design Pattern: Choreographed Enemy Formation

> **Inspired by**: Classic arcade shmups & *Star Fox 64*  
> **Classification**: Combat Encounter Building Block  
> **Status**: Implemented & Verified

---

## 1. Purpose & Fantasy
To create satisfying, readable shooting galleries where skilled players can achieve hit combos (`x3`, `x5`, `COMBO!`) and score multipliers, while beginners can easily read the incoming threat trajectory.

---

## 2. Standard Formation Typologies

### 2.1 The Classic V-Formation (Arrowhead)
- **Structure**: 1 leader at apex, 2–4 wing drones flanking behind in a chevron.
- **Player Experience**: Shooting the leader first destabilizes the formation; destroying all members before they exit awards an extra Star medal or supply pickup.
- **Placement**: Great for stage openings and warm-up beats.

### 2.2 The Ground-Air Crossfire Pincer
- **Structure**: 2 Ground Tanks on the highway floor accompanied by a low-altitude line of 3 Drone strafers above them.
- **Player Experience**: Forces the player to divide attention between low ground targets (requiring pitch-down aim) and incoming aerial plasma fire.

### 2.3 The Rooftop Flak Battery
- **Structure**: Flak turrets mounted on building ledges ($Y = 12$ to $18$) firing rhythmic triple bursts into the corridor center.
- **Player Experience**: Teaches the player to hug the opposite edge or use barrel rolls to deflect the burst while lining up return fire.

---

## 3. Design Constraints & Common Failure Modes
- **Screen Clutter**: Do not overlap more than 2 distinct formation types simultaneously; otherwise, bullet hell saturation destroys readable rail flight.
- **Spawn Distance**: Formations must spawn at least $250\text{ units}$ ahead of the player to allow reticle convergence and charged lock-on acquisition.

---

## 4. Timeline JSON Example
```json
{
  "trigger_z": -2200.0,
  "type": "enemy_wave",
  "count": 5,
  "spacing": 1.2,
  "spawn_x": 0.0,
  "formation": "v_formation",
  "enemy_type": "drone"
}
```
