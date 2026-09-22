If you want **agy to become genuinely good at designing Star Fox-like levels, game flow, and gameplay systems**, I would approach it differently from simply giving it prompts like:

> "Create a great level similar to Corneria."

That might produce something functional, but the AI won't necessarily understand your game's design philosophy, architecture, or what makes a level enjoyable.

What you want is closer to building an **AI-assisted game development workflow**, where agy acts as a combination of:

1. Game designer
2. Game architect
3. Level designer
4. Gameplay programmer
5. Code reviewer
6. Playtest analyst

The key is to give it **persistent project knowledge, structured responsibilities, constraints, and feedback loops.**

I'll explain how I'd set this up for your project.

---

# 1. First, Understand the Problem With AI Coding Agents

Imagine you tell agy:

```text
Create the first level of my Star Fox-like game.

Include:
- Enemies
- Obstacles
- Dialogue
- Boss fight
- Interesting game flow
```

An AI might generate:

```text
Stage 1
├── Spawn enemies
├── Spawn obstacles
├── Play dialogue
└── Spawn boss
```

The code could work perfectly.

But the level might feel terrible because:

* Enemy encounters lack pacing.
* Dialogue interrupts gameplay at awkward moments.
* Obstacles are randomly placed.
* The boss doesn't build on earlier mechanics.
* The player has no reason to explore.
* The level is a collection of features rather than a cohesive experience.

**The AI needs to understand the design intent before implementing the mechanics.**

This is why I'd separate your workflow into design, architecture, implementation, and evaluation.

---

# 2. Give agy a Persistent Project Brain

Your AI agent should not need to rediscover your game architecture every time you ask it to implement something.

Create a dedicated documentation structure in your repository.

For example:

```text
your-game/
│
├── AGENTS.md
│
├── docs/
│   ├── game-design/
│   │   ├── vision.md
│   │   ├── core-gameplay.md
│   │   ├── level-design-principles.md
│   │   ├── game-flow.md
│   │   └── difficulty.md
│   │
│   ├── architecture/
│   │   ├── overview.md
│   │   ├── gameplay-systems.md
│   │   ├── event-system.md
│   │   ├── boss-system.md
│   │   └── data-models.md
│   │
│   ├── levels/
│   │   ├── stage-01-design.md
│   │   ├── stage-01-flow.md
│   │   └── stage-01-encounters.md
│   │
│   └── decisions/
│       ├── ADR-001-event-driven-levels.md
│       └── ADR-002-boss-state-machines.md
│
├── src/
│
└── content/
    └── stages/
```

The exact structure can change depending on your engine, but the principle is important.

### Why this helps

Without persistent knowledge, agy might make decisions like:

> "I'll implement the boss using a simple timer."

Then, a few days later:

> "I'll replace the boss with a completely different architecture."

Your project becomes inconsistent.

With documentation, agy can reference:

* What your game is trying to achieve.
* Which systems already exist.
* What architectural decisions you've made.
* What constraints it must respect.
* Which features are unfinished.

Think of this documentation as the **context and long-term memory of your development agent**.

---

# 3. AGENTS.md: Your Most Important Starting Point

If agy supports `AGENTS.md` or equivalent project instruction files, use one as the entry point.

Don't fill it with hundreds of pages of instructions.

Keep it focused on rules that should apply to most coding tasks.

Here's an example tailored to your game.

### `AGENTS.md`

```markdown
# Project Instructions

## Project Overview

This is a Star Fox-inspired 3D rail shooter.

The game focuses on:
- Fast-paced aerial combat
- Scripted mission choreography
- Environmental storytelling
- Wingmate interactions
- Branching routes
- Distinct boss encounters

## Development Principles

1. Prefer data-driven level design.
2. Separate gameplay systems from level-specific content.
3. Use reusable systems instead of stage-specific hardcoding.
4. Avoid unnecessary architectural complexity.
5. Preserve existing functionality unless a change is intentional.
6. Prioritize gameplay feel and iteration speed.
7. Do not implement features that are not required by the current task.

## Level Design Principles

Every stage should have:
- A clear gameplay identity
- A coherent progression
- Encounter variety
- Meaningful environmental interaction
- A deliberate difficulty curve
- A memorable climax

Do not randomly place enemies or obstacles
without considering player movement, camera,
encounter pacing, and difficulty.

## Implementation Rules

Before modifying code:
1. Inspect the existing architecture.
2. Identify relevant systems.
3. Check existing data models.
4. Explain the proposed approach.

After implementation:
1. Run relevant tests.
2. Check for regressions.
3. Explain what changed.
4. Identify any remaining limitations.
```

This is a starting point, not a guarantee that agy will always follow every rule.

**The quality of the instructions matters less than whether the agent can consistently access and apply the relevant project context.**

---

# 4. Don't Ask One AI to Do Everything at Once

This is probably the biggest improvement you can make.

Instead of:

```text
Design and implement an entire amazing level.
```

Use a staged workflow.

## Phase 1: Game Designer

Ask agy to design the experience.

```text
You are the lead game designer.

Design Stage 01 for our Star Fox-inspired game.

Goals:
- Introduce the game's core mechanics.
- Establish the world and conflict.
- Include 5–7 distinct gameplay beats.
- Create a memorable climax.
- Avoid unnecessary mechanics that haven't been implemented.

Do not write code yet.

Deliver:
1. Stage fantasy
2. Player objectives
3. Gameplay progression
4. Encounter breakdown
5. Environmental storytelling
6. Difficulty progression
7. Boss concept
8. Risks and design weaknesses
```

The AI should produce a design document, not immediately generate 2,000 lines of code.

---

## Phase 2: Game Architect

Now ask it to translate the design into your existing architecture.

```text
You are the lead gameplay architect.

Read:
- AGENTS.md
- Game design document
- Existing architecture documentation
- Current source code

Determine how Stage 01 should be implemented.

Do not write implementation code yet.

Deliver:
1. Required gameplay systems
2. Existing systems that can be reused
3. Required data models
4. Event types
5. State transitions
6. Boss integration
7. Technical risks
8. Implementation plan

Do not create duplicate systems if an existing system
can be extended safely.
```

This forces agy to inspect your project before inventing architecture.

---

## Phase 3: Implementation Agent

Now let it implement one piece at a time.

```text
Implement the Stage 01 event system.

Scope:
- Stage event data model
- Event scheduler
- Event execution
- Basic enemy spawn event

Constraints:
- Follow existing architecture.
- Do not implement the boss yet.
- Do not modify unrelated systems.
- Add tests where practical.

Before coding:
- Inspect relevant files.
- Explain the implementation plan.

After coding:
- Run tests.
- Summarize changes.
- Identify limitations.
```

This makes the task easier to review and reduces the chance of architectural drift.

---

# 5. Give Your AI a Game Design Framework

This is where you can make agy much better at creating levels.

Don't just tell it to make the level "fun."

Give it a structured way to think.

## A. Stage Identity

Every level should have a clear fantasy.

Examples:

| Stage             | Core Fantasy                                       |
| ----------------- | -------------------------------------------------- |
| Corneria-inspired | Defend a civilization under attack                 |
| Asteroid Belt     | Navigate dangerous space terrain                   |
| Enemy Fleet       | Penetrate a heavily defended formation             |
| Abandoned Colony  | Investigate a destroyed settlement                 |
| Planetary Storm   | Fight while visibility and navigation are affected |

Ask agy:

> What should the player feel they are doing in this stage?

This prevents every level from becoming the same sequence of enemy waves.

---

## B. Gameplay Beats

I recommend using a beat structure.

For example:

```text
Stage 01: Last Light of Helios

Beat 1: Arrival
- Establish environment
- Introduce basic enemy threat

Beat 2: Defensive Line
- Enemy formations
- Introduce target prioritization

Beat 3: Environmental Hazard
- Falling debris
- Player navigation challenge

Beat 4: Wingmate Rescue
- Protect an ally
- Introduce a branching condition

Beat 5: Escalation
- Larger enemy formation
- Increased combat pressure

Beat 6: Boss Approach
- Visual transition
- Dialogue and anticipation

Beat 7: Boss Fight
- Introduce a new combat challenge
- Conclude the stage
```

For each beat, ask agy to define:

```text
Gameplay Beat
├── Purpose
├── Player Objective
├── Player Actions
├── Threats
├── Environment
├── Expected Duration
├── Difficulty
├── Transition
└── Failure / Success Conditions
```

This creates a more consistent design process.

---

# 6. Use a Structured Level Schema

This is where your software engineering background can really help.

Instead of having agy generate arbitrary level JSON, define a schema that constrains what a valid level can contain.

For example:

```typescript
interface StageDefinition {
  id: string;
  name: string;
  environment: EnvironmentDefinition;
  sections: StageSection[];
  routes: StageRoute[];
  boss?: BossEncounterDefinition;
}
```

```typescript
interface StageSection {
  id: string;
  name: string;
  events: StageEvent[];
  completionCondition: CompletionCondition;
}
```

```typescript
type StageEvent =
  | SpawnFormationEvent
  | SpawnObstacleEvent
  | DialogueEvent
  | CameraEvent
  | RescueEvent
  | BossTriggerEvent;
```

Now agy has a defined vocabulary.

Instead of inventing:

```json
{
  "type": "make_cool_enemy_thing"
}
```

It needs to use supported event types.

### Add Validation

Your game should validate the stage before running it.

```text
Stage Validation
├── Duplicate Event IDs
├── Invalid Enemy References
├── Missing Dialogue References
├── Invalid Route Conditions
├── Missing Boss Definition
├── Impossible Completion Conditions
└── Invalid Timing
```

The AI can generate content, but **your runtime should enforce correctness**.

This is important because LLMs are capable of generating plausible-looking but invalid configurations.

---

# 7. Separate Game Flow From Game Behavior

This distinction will help your architecture significantly.

Imagine a boss event:

```text
Start Boss Fight
```

The stage system should not necessarily know how the boss performs every attack.

Instead:

```text
Stage System
    │
    ▼
Boss Encounter Manager
    │
    ▼
Boss Controller
    │
    ├── Phase 1
    ├── Phase 2
    └── Phase 3
```

The stage defines **when and why the boss starts**.

The boss controller defines **how the boss behaves**.

Similarly:

```text
Stage System
    │
    ▼
Dialogue Manager
```

The stage triggers dialogue, but the dialogue system manages its own presentation.

This separation helps agy modify one system without breaking unrelated systems.

---

# 8. Build a Dedicated Level Design Agent Persona

You can instruct agy to act as a specific role for design tasks.

However, don't rely solely on roleplay. Give the role clear responsibilities and deliverables.

Example:

```markdown
# Level Design Agent

## Responsibility

Design engaging stages for a 3D rail shooter.

## Priorities

1. Player experience
2. Encounter pacing
3. Clarity of objectives
4. Gameplay variety
5. Environmental storytelling
6. Replayability
7. Technical feasibility

## Required Analysis

Before proposing an encounter, consider:
- Player movement constraints
- Camera orientation
- Enemy visibility
- Available reaction time
- Projectile density
- Collision risks
- Current player abilities
- Difficulty relative to earlier encounters

## Avoid

- Random enemy placement
- Unexplained difficulty spikes
- Unnecessary mechanics
- Repetitive encounters
- Unimplementable cinematic sequences
- Bosses that rely only on increased health

## Output

Provide:
1. Design intent
2. Encounter flow
3. Player experience
4. Implementation requirements
5. Potential problems
```

You can reference this document when asking agy to design a stage.

---

# 9. Make the AI Critique Its Own Designs

This is essential.

An AI that only generates designs will often accept its first idea as good enough.

Ask it to critique the design separately.

### Example Prompt

```text
Review the proposed Stage 01 design as a critical
gameplay designer.

Do not rewrite the design immediately.

Analyze:

1. Is the opening engaging?
2. Is the gameplay progression clear?
3. Are encounters repetitive?
4. Does the difficulty increase appropriately?
5. Are environmental hazards readable?
6. Does the rescue encounter have meaningful gameplay?
7. Is the boss mechanically distinct?
8. Are there unnecessary mechanics?
9. Could the stage be implemented using existing systems?
10. What would make the stage feel generic?

For every issue:
- Identify the problem.
- Explain why it matters.
- Propose a specific improvement.
```

Then you can ask it to revise the design.

### Even Better: Use Separate Contexts

If your workflow supports multiple agent sessions, use separate sessions for:

* Design
* Architecture
* Implementation
* Review

This can reduce the tendency to defend earlier decisions.

It's not a guarantee of unbiased critique, but separating responsibilities can make the review process more useful.

---

# 10. Use a Planner → Coder → Reviewer Workflow

You previously mentioned wanting a planner → coder → reviewer workflow for your projects.

That structure fits this game extremely well.

I'd extend it into:

```text
                 USER IDEA
                     │
                     ▼
              DESIGN AGENT
                     │
                     ▼
            DESIGN DOCUMENT
                     │
                     ▼
             ARCHITECT AGENT
                     │
                     ▼
             TECHNICAL PLAN
                     │
                     ▼
             IMPLEMENTATION
                     │
                     ▼
              CODE REVIEW
                     │
                     ▼
             PLAYTEST / QA
                     │
                     ▼
              USER FEEDBACK
                     │
                     └──────────► IMPROVEMENT
```

### Important

Don't make every stage require all five agents.

For a small change, you might only need:

```text
Planner → Coder → Tests
```

For a new boss or major stage:

```text
Designer → Architect → Coder → Reviewer → Playtest
```

The workflow should scale with the task.

---

# 11. Give the AI Actual Gameplay Feedback

This is where many AI game development workflows fall short.

An AI can write a level that looks logically correct in the code but feels awful when played.

For example:

```text
Enemy Spawn Time: 10.0s
Enemy Spawn Time: 12.0s
Enemy Spawn Time: 14.0s
Enemy Spawn Time: 16.0s
```

That might seem reasonable.

But if each enemy fires five projectiles, the player could be overwhelmed.

The AI needs feedback about actual gameplay.

## Build Debugging and Playtest Tools

Useful features include:

* Restart stage from a specific section.
* Skip to a particular encounter.
* Display current stage event.
* Show enemy spawn positions.
* Show player hitbox.
* Display event timeline.
* Log damage taken.
* Record player deaths.
* Show boss phase transitions.
* Toggle invulnerability during testing.

Example:

```text
[DEBUG MODE]

Current Stage: Stage 01
Current Section: City Assault
Current Event: Enemy Formation 03

Player Health: 75
Enemies Active: 8
Projectiles Active: 12
Boss Phase: N/A

[Restart Section]
[Skip Event]
[Spawn Test Enemy]
[Toggle Invincibility]
```

These tools help you and agy diagnose problems.

---

# 12. Use Playtest Metrics

You can also record structured gameplay data.

For example:

```typescript
interface EncounterTelemetry {
  stageId: string;
  encounterId: string;
  duration: number;
  playerDamageTaken: number;
  playerDeaths: number;
  enemiesDefeated: number;
  objectivesCompleted: boolean;
}
```

Then your AI can analyze the data.

Example:

```text
Stage 01 - Encounter 04

Average Duration: 38 seconds
Average Damage Taken: 42%
Deaths: 7 / 20 attempts
Objective Completion: 65%
```

You could ask:

> Analyze this encounter's telemetry and suggest potential difficulty or pacing issues.

**Be careful with interpretation:** telemetry can identify potential problems, but it cannot automatically determine what makes an encounter fun. Human playtesting and qualitative feedback remain important.

---

# 13. Build a Design Evaluation Rubric

You can ask agy to evaluate a level against consistent criteria.

However, I would avoid treating an LLM-generated score as an objective measure of quality.

Use the rubric as a checklist for discussion, not as a definitive quality rating.

Example:

```markdown
# Level Design Review Checklist

## Clarity
- Is the objective understandable?
- Are threats visually readable?
- Are route changes communicated?

## Pacing
- Are there meaningful changes in intensity?
- Are encounters given enough breathing room?
- Does the climax feel earned?

## Gameplay
- Does the player have interesting decisions?
- Are mechanics introduced progressively?
- Are obstacles avoidable through skill?

## Narrative
- Does the environment communicate the setting?
- Do dialogue events support the action?
- Do characters have meaningful interactions?

## Replayability
- Are there optional challenges?
- Do alternate routes have a purpose?
- Can the player improve through mastery?

## Technical Feasibility
- Can the current systems support this?
- Are the required assets available?
- Are the event dependencies clear?
```

This is much more useful than asking:

> "Rate this level from 1 to 10."

---

# 14. Don't Let AI Generate Content Without a Design Library

This is a more advanced improvement.

Create reusable design patterns.

For example:

```text
docs/game-design/patterns/
├── enemy-formation.md
├── environmental-hazard.md
├── wingmate-rescue.md
├── alternate-route.md
├── escort-encounter.md
├── boss-introduction.md
└── arena-transition.md
```

Each pattern describes:

* Purpose
* Player experience
* Requirements
* Common failure modes
* Implementation considerations
* Example configurations

### Example: Wingmate Rescue Pattern

```markdown
# Wingmate Rescue

## Purpose

Create urgency and reinforce the player's relationship
with a squadmate.

## Structure

1. Ally enters danger.
2. Threat becomes visible.
3. Player receives objective.
4. Player intervenes.
5. Success or failure is resolved.

## Design Constraints

- Threats must be readable.
- Player should have sufficient reaction time.
- Failure should have a defined consequence.
- Avoid making the ally feel like an invulnerable prop.

## Required Systems

- Ally controller
- Enemy targeting
- Rescue objective
- Dialogue trigger
- Success/failure event
```

Now agy has a library of patterns to draw from.

It can combine them to create new levels without inventing everything from scratch.

---

# 15. Give the AI a Clear Asset and System Inventory

This is particularly important if you want agy to create levels that are actually implementable.

Maintain a document like:

```markdown
# Available Game Systems

## Player
- Arwing movement
- Barrel roll
- Boost
- Brake
- Laser shooting

## Enemies
- Basic fighter
- Interceptor
- Turret
- Heavy ship

## Environment
- Space background
- Asteroids
- Space station
- Destructible debris

## Gameplay Events
- Spawn enemy
- Spawn formation
- Dialogue
- Camera transition
- Boss trigger

## Bosses
- None implemented yet
```

When you ask agy to design a stage, it knows what is available.

Otherwise, it may propose:

> "Add a gravitational anomaly that reverses the player's controls."

But you don't have that mechanic, and it might take a week to implement.

You want the AI to distinguish between:

1. Features already available.
2. Features that can be reused.
3. Features that need to be implemented.
4. Features that are outside the current scope.

---

# 16. How I'd Organize Your Actual Development Workflow

Given your software engineering background, I would structure the project around a **vertical-slice-first approach**.

Don't ask agy to build an entire game engine and then create ten levels.

Build one complete playable experience.

### Milestone 1: Core Flight

```text
Player Flight
      ↓
Camera
      ↓
Shooting
      ↓
Basic Enemy
      ↓
Collision
```

### Milestone 2: One Complete Encounter

```text
Enemy Formation
      ↓
Player Combat
      ↓
Encounter Completion
      ↓
Reward / Progression
```

### Milestone 3: One Complete Stage

```text
Opening
      ↓
Combat
      ↓
Environmental Hazard
      ↓
Dialogue
      ↓
Escalation
      ↓
Boss
      ↓
Stage Completion
```

### Milestone 4: Authoring Tools

```text
Stage Data
      ↓
Level Validation
      ↓
Timeline Editing
      ↓
Spawn Configuration
      ↓
Boss Configuration
```

Only after you have a functional stage would I invest heavily in a custom editor.

---

# 17. The Prompt I'd Actually Give agy

Here's a more comprehensive prompt you can adapt.

```text
You are working on a Star Fox-inspired 3D rail shooter.

Your role is to assist with game design, architecture,
implementation, and code review.

Before performing a task, determine which role is required.

PROJECT CONTEXT:
- Read AGENTS.md.
- Read relevant game design documentation.
- Read relevant architecture documentation.
- Inspect the existing codebase.
- Identify reusable systems before proposing new ones.

GAME DESIGN EXPECTATIONS:
- Prioritize player experience over feature quantity.
- Design stages around deliberate gameplay beats.
- Consider pacing, difficulty, readability, and progression.
- Avoid repetitive enemy waves.
- Make environmental objects serve a gameplay purpose.
- Ensure boss encounters have distinct mechanics.
- Consider how dialogue affects gameplay flow.

ARCHITECTURE EXPECTATIONS:
- Prefer data-driven level content.
- Separate stage orchestration from gameplay behavior.
- Reuse existing systems.
- Keep level-specific logic out of generic systems.
- Use explicit data models and validation.
- Avoid unnecessary abstractions.

WORKFLOW:
1. Understand the task.
2. Inspect relevant project files.
3. Identify constraints and dependencies.
4. Propose a plan.
5. Ask for clarification if an important decision
   cannot be inferred safely.
6. Implement the smallest complete increment.
7. Run relevant tests and validation.
8. Review the result for regressions.
9. Summarize changes and remaining limitations.

IMPORTANT:
Do not implement an entire game feature based on
assumptions about undocumented systems.

Do not modify unrelated code.

Do not claim a feature works without testing it
or clearly explaining what was not tested.
```

---

# 18. What About Your Current agy Setup?

You previously mentioned that you're using **agy 1.2.2**, the ordinary agy setup rather than OmniRouter, and that you had an issue where model selection showed nothing and produced:

> `failed to construct executor: neither PlanModel nor RequestedModel specified`

That matters because before optimizing your agent workflow, you should make sure the underlying model configuration is working correctly.

A sophisticated workflow won't help much if agy cannot consistently select and execute the intended model.

I wouldn't assume that the same configuration or model availability still applies today, though. Your current version and backend configuration should be checked before making specific configuration recommendations.

---

# My Recommendation: Build a Game-Specific AI Development System

If I were in your position, I would prioritize these **five things**:

| Priority | System                         | Purpose                                          |
| -------- | ------------------------------ | ------------------------------------------------ |
| 1        | Project instructions           | Consistent coding and design rules               |
| 2        | Game design documentation      | Persistent understanding of your game's identity |
| 3        | Data-driven level architecture | Easier content generation and iteration          |
| 4        | Planner → Coder → Reviewer     | Controlled implementation workflow               |
| 5        | Debugging and playtest tools   | Feedback for improving actual gameplay           |

The most important distinction is this:

**You don't want agy to simply generate game code. You want it to understand your game's design language and produce content that fits your existing systems.**

And that requires more than a better prompt. It requires a project structure that makes good decisions easier for the AI to make.