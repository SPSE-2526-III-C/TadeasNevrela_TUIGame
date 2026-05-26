# Decision Memory

This file tracks project decisions so future work can preserve intent instead of re-litigating the same choices.

## How to Use

Add a new entry whenever a design, architecture, or gameplay choice affects future implementation.

Each decision should include:

- Date: When the decision was made.
- Status: `Proposed`, `Accepted`, `Changed`, or `Rejected`.
- Decision: The choice that was made.
- Reasoning: Why this direction was chosen.
- Alternatives: Other options considered.
- Follow-up: Anything that should be revisited later.

## Decision Log

### 2026-05-26 - Security Terminal Is the Main Player Interface

Status: Accepted

Decision:
The player interacts with the game through a structured security terminal UI rather than directly walking around the world.

Reasoning:
This supports the core horror premise: the player is a security operator whose information comes through unreliable systems, cameras, logs, doors, and integrity panels.

Alternatives:
Direct first-person movement, command-line typing as the main interface, or a hybrid terminal plus movement system.

Follow-up:
Keep new features menu-driven unless there is a strong reason to add direct command input.

### 2026-05-26 - Cursor Menus Are the MVP Input Model

Status: Accepted

Decision:
The MVP uses cursor-based menu navigation with keys such as up, down, enter, back, and escape.

Reasoning:
Cursor menus are simple to implement in C, easy to test, and fit the security OS style. They avoid parser complexity while the core systems are still forming.

Alternatives:
Typed commands, mouse input, hotkey-heavy navigation, or free-form text entry.

Follow-up:
Typed commands can be added later as an advanced system once the base menu loop is stable.

### 2026-05-26 - Cameras Use ASCII Raycasting

Status: Accepted

Decision:
Camera feeds are rendered with Wolfenstein-style raycasting into ASCII output.

Reasoning:
Raycasting gives the game a pseudo-3D surveillance look while staying compatible with a terminal UI. It also gives horror events a clear place to distort perception.

Alternatives:
Static ASCII images, tile-map views, pre-rendered scenes, or full graphical rendering.

Follow-up:
Keep the raycaster independent enough that anomaly effects can alter camera output without corrupting the world map directly.

### 2026-05-26 - Horror Comes From System Unreliability

Status: Accepted

Decision:
The system itself becomes unreliable through corrupted logs, inconsistent camera feeds, suspicious door states, and integrity changes.

Reasoning:
This matches the operator fantasy and avoids relying only on visible enemies or jump scares. It also gives every subsystem a gameplay role.

Alternatives:
Monster-chase horror, combat encounters, or scripted cutscenes as the main fear source.

Follow-up:
When adding a new subsystem, define how it can fail, lie, or contradict another subsystem.

## Open Questions

- Should the game eventually support typed commands, or should it remain purely menu-driven?
- How much of the world state should be real simulation versus camera-only illusion?
- Should anomalies be entities in the map, event scripts, rendering artifacts, or a mix of all three?
