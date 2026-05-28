# TUI Horror Game Development Roadmap

## Project Goal

This project is not just a terminal UI app.

It is a **system simulation horror game** where:

* the player interacts through terminal panels,
* the game slowly becomes unreliable,
* and the terminal itself becomes part of the horror.

The architecture matters because later systems depend on earlier decisions.

The most important design rule:

> The game must be able to lie to the player.

That means:

* the *real* game state exists internally,
* the *displayed* state may become corrupted later.

Build the project in layers.

---

# Core Architecture

The entire game is built around three loops:

```text
INPUT  -> UPDATE -> RENDER
```

## INPUT

Reads keyboard input and converts it into abstract actions.

Example:

```text
Arrow Up -> ACTION_UP
Enter    -> ACTION_SELECT
Q        -> ACTION_QUIT
```

The input system should NEVER directly modify game state.

It only translates keys into actions.

---

## UPDATE

The update phase changes the actual game state.

Examples:

* move menu cursor,
* open a door,
* trigger an event,
* reduce integrity,
* add log entries.

Only update systems should mutate data.

---

## RENDER

The renderer only displays information.

It should:

* never change game state,
* never run gameplay logic,
* never decide events.

It only reads data and draws it.

---

# High-Level System Layout

```text
main.c
│
├── input
├── update systems
│   ├── menu
│   ├── doors
│   ├── events
│   ├── integrity
│   └── logs
│
└── renderer
    ├── menu UI
    ├── camera UI
    ├── logs UI
    ├── doors UI
    └── systems UI
```

---

# Recommended Development Order

DO NOT jump randomly between systems.

The order below minimizes debugging pain.

---

# STEP 0 — Project Skeleton

## Goal

Get a stable executable that:

* opens ncurses,
* runs a loop,
* exits cleanly.

## Files

```text
src/main.c
include/core/game_state.h
```

## Needed Systems

### GameState

Central structure that owns all game data.

Example:

```c
typedef struct {
    Screen current_screen;
    int menu_cursor;
    int should_quit;

    int system_integrity;
    int tick;
} GameState;
```

## Why This Exists

You need one place where the entire game lives.

Avoid:

* global variables,
* disconnected state,
* random static variables across files.

Everything should eventually belong to GameState.

---

# STEP 1 — Input System

## Goal

Translate keyboard input into abstract actions.

## Files

```text
src/core/input.c
include/core/input.h
include/core/types.h
```

## Needed Systems

### InputAction enum

Example:

```c
typedef enum {
    ACTION_NONE,
    ACTION_UP,
    ACTION_DOWN,
    ACTION_SELECT,
    ACTION_BACK,
    ACTION_QUIT
} InputAction;
```

## Why This Exists

The rest of the game should not care about raw keys.

Good:

```text
menu_handle_input(state, ACTION_UP)
```

Bad:

```text
if (key == KEY_UP)
```

inside every module.

---

# STEP 2 — Menu System

## Goal

Navigate between panels.

## Files

```text
src/ui/menu.c
include/ui/menu.h
```

## Needed Systems

### Screen enum

```c
typedef enum {
    SCREEN_MENU,
    SCREEN_CAMERAS,
    SCREEN_DOORS,
    SCREEN_LOGS,
    SCREEN_SYSTEMS,
    SCREEN_EMERGENCY
} Screen;
```

### Menu Cursor

Tracks selected item.

Example:

```c
state->menu_cursor
```

## Why This Exists

The menu becomes the player's central interaction point.

It should:

* feel responsive,
* support future corruption,
* remain isolated from gameplay logic.

---

# STEP 3 — Renderer Architecture

## Goal

Separate rendering from game logic.

## Files

```text
src/engine/renderer.c
src/ui/*
```

## Needed Systems

### Renderer Dispatcher

One function:

```c
renderer_draw(GameState *state)
```

This chooses WHICH panel to draw.

Example:

```c
switch(state->current_screen)
```

## Important Rule

Renderer should NEVER:

* modify state,
* trigger events,
* process gameplay.

Only display data.

---

# STEP 4 — Logs System

## Goal

Create a central event history.

## Files

```text
src/ui/logs_ui.c
include/ui/logs_ui.h
```

## Needed Systems

### Ring Buffer

This is critical.

You do NOT want dynamically growing logs forever.

Use fixed capacity:

```c
#define LOG_MAX 64
```

## Data Structure

```c
typedef struct {
    char lines[LOG_MAX][80];
    int head;
    int count;
} LogBuffer;
```

## Core Concept

When full:

* overwrite oldest entry.

Core operation:

```c
head = (head + 1) % LOG_MAX;
```

## Why Logs Matter

Every future system writes here:

* doors,
* integrity warnings,
* fake events,
* hallucinations,
* emergency protocols.

The logs make the world feel alive.

---

# STEP 5 — Doors System

## Goal

Create interactable world state.

## Files

```text
src/systems/doors.c
include/systems/doors.h
```

## Needed Systems

### Door Structure

VERY IMPORTANT:
Separate real and displayed state.

```c
typedef struct {
    int id;

    DoorState real_state;
    DoorState displayed_state;

    char label[32];
} Door;
```

## Why Two States Exist

Later:

* the UI may say a door is locked,
* while the real door is open.

This is the core horror mechanic.

Never merge them.

---

# STEP 6 — Map System

## Goal

Represent physical world layout.

## Files

```text
src/engine/map.c
include/engine/map.h
```

## Needed Systems

### Tile Grid

```c
#define MAP_W 24
#define MAP_H 24
```

```c
typedef struct {
    int tiles[MAP_H][MAP_W];
} Map;
```

## Tile Types

```text
0 = empty
1 = wall
2 = door
```

## Why This Exists

The map becomes:

* camera source,
* navigation space,
* event location system,
* door placement system.

---

# STEP 7 — Camera System

## Goal

Display surveillance feeds.

## Files

```text
src/ui/camera_ui.c
src/engine/raycaster.c
```

---

# PHASE 1 — Fake Camera

Before real raycasting:

* just draw the map as ASCII.

Example:

```text
########
#......#
#..D...#
########
```

This verifies:

* map logic,
* camera ownership,
* rendering flow.

DO THIS FIRST.

---

# PHASE 2 — Raycaster

Only after map works.

## Core Concept

For each terminal column:

* cast a ray,
* measure wall distance,
* draw vertical slice height.

Example:

```c
wall_height = screen_height / distance;
```

## Why Raycasting Exists

It creates:

* depth,
* atmosphere,
* distortion opportunities,
* camera corruption effects.

---

# STEP 8 — Integrity System

## Goal

Drive horror escalation.

## Files

```text
src/systems/integrity.c
include/systems/integrity.h
```

## Needed Systems

### Integrity Value

```c
0 -> 100
```

### Thresholds

```text
100-75 normal
75-50 suspicious
50-25 contradictory
25-0 unstable
0 failure
```

## Effects By Stage

### Stage 1

Minor fake logs.

### Stage 2

Displayed door states desync.

### Stage 3

Camera glitches.

### Stage 4

Input ignored occasionally.

### Stage 5

Terminal failure.

---

# STEP 9 — Event System

## Goal

Create timed behavior.

## Files

```text
src/systems/events.c
include/systems/events.h
```

## Needed Systems

### Timers

Use:

```c
state->tick
```

Avoid:

```c
sleep()
```

The game loop should always continue running.

---

# STEP 10 — Horror Layer

This is NOT one file.

It is modifications added everywhere.

---

# HORROR DESIGN RULES

## 1. Contradiction Creates Fear

Examples:

* logs disagree with cameras,
* doors disagree with UI,
* systems contradict each other.

---

## 2. The Terminal Becomes Untrustworthy

Examples:

* flickering labels,
* corrupted log lines,
* false warnings,
* menu movement,
* delayed input.

---

## 3. Never Fully Break The Game

The player should:

* doubt the terminal,
* not fight the controls constantly.

Small corruption > chaos.

---

# System Relationships

## Logs

Receives:

* door actions,
* event alerts,
* integrity warnings.

---

## Integrity

Modifies:

* renderer reliability,
* displayed states,
* input consistency.

---

## Doors

Affects:

* cameras,
* logs,
* events.

---

## Cameras

Reads:

* map,
* door state,
* integrity corruption.

---

## Events

Controls:

* pacing,
* escalation,
* scripted incidents.

---

# Main Architectural Rules

## Rule 1

Update mutates data.

Render never mutates data.

---

## Rule 2

Keep modules isolated.

Good:

```text
doors.c owns door logic
```

Bad:

```text
renderer changes door states
```

---

## Rule 3

Every meaningful action writes to logs.

---

## Rule 4

Compile constantly.

Never write huge amounts before testing.

---

# Suggested Milestones

## Milestone 1

Stable menu system.

## Milestone 2

Working logs ring buffer.

## Milestone 3

Doors interact and write logs.

## Milestone 4

ASCII camera map.

## Milestone 5

Real raycaster.

## Milestone 6

Integrity corruption.

## Milestone 7

Full horror session.

---

# Suggested Folder Responsibilities

```text
core/
    engine-independent fundamentals

engine/
    rendering + world simulation

ui/
    terminal presentation only

systems/
    gameplay logic and state mutation
```

---

# Final Advice

Do NOT chase polish early.

Your priorities should be:

1. architecture,
2. clean state flow,
3. reliable update/render separation,
4. extensibility.

A stable ugly system is better than a pretty tangled one.

Most horror atmosphere will come naturally once:

* logs exist,
* integrity affects systems,
* displayed state diverges from reality.

That is the heart of the game.
