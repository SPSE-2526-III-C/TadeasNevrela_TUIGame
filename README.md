# TUI SECURITY HORROR RAYCASTING GAME (C) — FULL DESIGN SPEC

## OVERVIEW

You are building a terminal-based security operator simulation horror game in C.

The player does NOT directly move in a world. Instead, they operate a facility through a multi-panel security terminal interface.

Core pillars:

* Cursor-based TUI navigation system
* Menu-driven “security OS” interface
* Wolfenstein-style raycasting cameras rendered in ASCII
* System simulation (doors, logs, alarms, power, integrity)
* Horror emerges from system corruption, contradictions, and unreliable reality

---

## CORE GAME LOOP

The game runs continuously:

1. Handle keyboard input
2. Update system state
3. Process timed/event anomalies
4. Render full TUI interface

```c
while (running) {
    handle_input();
    update_state();
    generate_events();
    render_ui();
}
```

---

## INPUT & CONTROL SYSTEM

### Navigation

* UP / DOWN → move cursor
* ENTER → select menu item
* ESC / BACKSPACE → back / exit menu
* TAB → optional panel switching (advanced)

### Core interaction style

* Cursor-driven selection (no typing required for MVP)
* Optional command input later for advanced systems

---

## UI ARCHITECTURE (SECURITY TERMINAL)

Main interface is a structured menu system:

```
SECURITY TERMINAL v1.0

> Cameras        <--
  Doors
  Security Logs
  Reports
  Systems
  Emergency Protocols
```

### UI principles

* Everything is menu-driven
* Panels represent subsystems
* UI behaves like real monitoring software
* Stack-based navigation (menus push/pop state)

---

## MENU SYSTEM DESIGN

### Structure

* Tree-based menu hierarchy
* Stack navigation model

Example:

MAIN MENU
├── Cameras
│    ├── CAM 01
│    ├── CAM 02
├── Doors
│    ├── Floor 1
│    ├── Floor 2
├── Logs
├── Systems

---

## CAMERA SYSTEM (CORE FEATURE)

Each camera is a **raycasting pseudo-3D renderer** (Wolfenstein-style).

Each camera behaves like a real surveillance feed.

### Camera structure

```c
typedef struct {
    float x;
    float y;
    float angle;
    float fov;
} Camera;
```

---

## WORLD REPRESENTATION

The facility is a 2D grid map.

### Tile types

* 0 = empty space
* 1 = wall
* 2 = door
* 3 = anomaly/entity

```c
#define MAP_W 24
#define MAP_H 24
int map[MAP_W][MAP_H];
```

---

## RAYCASTING ENGINE

Each camera frame is generated using raycasting:

### Process per screen column

1. Compute ray angle from camera direction + FOV
2. Step through grid until collision
3. Measure distance to wall
4. Convert distance into vertical ASCII slice

### Core formula

```c
wall_height = screen_height / distance;
```

---

## ASCII CAMERA RENDERING

Example output:

```
████████████████
██████    ██████
████        ████
██            ██
```

### Visual rules

* Close objects → tall, solid walls
* Far objects → short, thin walls
* Empty space → gaps

---

## PSEUDO-3D BEHAVIOR RULES

To simulate depth:

* Perspective narrowing toward center
* Distance-based scaling
* Fog effects (optional)
* Light intensity falloff

---

## CAMERA HORROR LAYER

Cameras are not fully reliable.

They may show:

* Objects that are not in the map
* Missing geometry
* Frame-to-frame inconsistencies
* Entities appearing in some rays only

Example:

* Column 12 shows entity
* Column 13 does not
* Column 14 shows entity again

---

## DOOR SYSTEM

Doors are interactive environmental tiles.

### States

* LOCKED
* UNLOCKED
* JAMMED
* UNKNOWN (corrupted state)

### Behavior

* Player can toggle doors (if permitted)
* System events can override state
* Some doors change without input (horror mechanic)

---

## LOG SYSTEM

Security logs show system events in real time.

Example:

```
[02:14] Motion detected CAM 03
[02:15] Door 2 opened manually
[02:15] No operator detected
```

### Horror behaviors

* Logs contradict camera feeds
* Logs rewrite themselves
* Logs reference impossible player actions
* Logs appear before events happen

---

## SYSTEMS PANEL

Displays subsystem health:

* Camera network
* Door control
* Logging system
* Power grid
* External connection

### Important rule:

System status becomes unreliable over time.

---

## SYSTEM INTEGRITY (NO HP SYSTEM)

There is no player health.

Instead:

```c
int system_integrity = 100;
```

---

## FAILURE STATES (DEATH SYSTEM)

### 1. LOCKOUT

* Player loses control of UI
* Input disabled or ignored

```
SYSTEM LOCKED
ACCESS REVOKED
```

---

### 2. OVERRIDE

* UI responds incorrectly
* Inputs trigger wrong actions
* Menu selections lie to the player

---

### 3. INTEGRITY ZERO

* Full system collapse
* UI corruption spreads
* Game ends or transitions into horror state

```
ALL MODULES COMPROMISED
```

---

### 4. REPLACEMENT ENDING

```
WELCOME BACK, OPERATOR
```

System resets with subtle changes (looping horror state).

---

## HORROR DESIGN PRINCIPLES

Horror comes from SYSTEM FAILURE, not monsters.

Core techniques:

* UI inconsistency
* Contradicting information layers
* Reality desync between cameras
* Cursor misalignment
* Menu restructuring over time

Player should start trusting the system… then lose that trust.

---

## CAMERA HORROR MECHANICS

Allowed distortions:

* Impossible geometry
* Broken perspective rules
* Flickering entities
* Missing or extra walls
* Different cameras showing different realities

---

## ARCHITECTURE (C PROJECT STRUCTURE)

```
src/
 ├── main.c
 ├── engine/
 │    ├── raycaster.c
 │    ├── map.c
 │    ├── renderer.c
 ├── ui/
 │    ├── menu.c
 │    ├── camera_ui.c
 │    ├── logs_ui.c
 ├── systems/
 │    ├── doors.c
 │    ├── events.c
 │    ├── integrity.c
```

---

## MINIMUM VIABLE PRODUCT (MVP)

Start with:

1. Cursor-based menu system
2. Single camera raycasting view
3. Simple hallway rendering
4. Door toggle system
5. Basic log system

Then expand into:

* Multiple cameras
* Entity/anomaly system
* System corruption mechanics
* Full multi-panel UI
* Horror-driven rule breaking

---

## FINAL DESIGN SUMMARY

This is a terminal-based security simulation horror game where:

* Cameras are raycasted pseudo-3D surveillance feeds
* UI behaves like real security software
* Player interacts via structured menus
* Horror emerges from system corruption and contradictory reality
* The system itself becomes the antagonist over time
