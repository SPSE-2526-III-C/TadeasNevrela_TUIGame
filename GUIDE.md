# TUI Game Development Guide

This repo is now a scaffold. Most functions are intentionally stubs with `TODO` comments so you can implement each subsystem step by step.

## 1) What You Have

Project layout:

- `src/main.c`: entry point and `GameState` initialization.
- `src/core/input.c`: input mapping stub.
- `src/engine/*`: map/raycaster/renderer stubs.
- `src/ui/*`: menu/camera/logs stubs.
- `src/systems/*`: doors/events/integrity stubs.
- `include/**`: public headers and shared types.
- `Makefile`: build/run/clean.

Build commands:

```bash
make
make run
make clean
```

## 2) Development Order (Recommended)

Implement in this order to keep scope under control and always have a runnable build.

### Step 0: Main Loop Skeleton

Files:

- `src/main.c`
- `include/core/game_state.h`

Do:

- Keep `game_state_init`.
- Add a loop that calls input, update, render.
- Exit cleanly on quit action.

Check:

- Program starts, loops, exits without crash.

### Step 1: Input Mapping

Files:

- `src/core/input.c`
- `include/core/input.h`

Do:

- Convert key presses into `InputAction`.
- Support at least: up, down, select, back, quit.
- Keep input handling isolated in this module.

Check:

- Print action debug output from `main.c` while testing.

### Step 2: Menu Navigation

Files:

- `src/ui/menu.c`
- `include/ui/menu.h`

Do:

- Implement menu cursor movement.
- Add transitions from main menu to panels.
- Implement back behavior to return to main menu.

Check:

- You can enter/exit Cameras, Doors, Logs, Systems.

### Step 3: Renderer Split

Files:

- `src/engine/renderer.c`
- `src/ui/camera_ui.c`
- `src/ui/logs_ui.c`

Do:

- `renderer_draw` dispatches by current screen.
- Panel drawing stays inside UI modules.
- Keep renderer free from system business logic.

Check:

- Each screen prints meaningful content.

### Step 4: Map + Camera Baseline

Files:

- `src/engine/map.c`
- `src/engine/raycaster.c`
- `include/engine/map.h`
- `include/engine/raycaster.h`

Do:

- Build a small static map.
- Implement simple camera feed output first.
- Later, replace with column-based raycasting.

Check:

- Camera screen shows deterministic output tied to map.

### Step 5: Doors System

Files:

- `src/systems/doors.c`
- `include/systems/doors.h`

Do:

- Initialize doors.
- Toggle selected door state.
- Reflect door state in Doors screen and camera view.

Check:

- Door changes are visible and consistent.

### Step 6: Logs System

Files:

- `src/ui/logs_ui.c`
- `include/ui/logs_ui.h`

Do:

- Implement fixed-size ring buffer append.
- Add log writes from input/menu/doors/events.
- Keep log text formatting simple.

Check:

- Logs roll correctly after capacity is reached.

### Step 7: Events + Integrity

Files:

- `src/systems/events.c`
- `src/systems/integrity.c`
- `include/systems/events.h`
- `include/systems/integrity.h`

Do:

- Add timed events.
- Reduce integrity over time or by event.
- Trigger behavior changes at thresholds.

Check:

- Session escalates from normal to unreliable behavior.

## 3) Coding Rules for This Project

- Keep each module focused and small.
- Put shared structs/enums in headers under `include/`.
- Keep UI rendering separate from simulation updates.
- Prefer explicit state changes over hidden side effects.
- Keep all new functions declared in headers.

## 4) First Concrete TODOs

If you want a direct starting punch list:

1. Implement `input_read_action`.
2. Add `update_game` and `render_game` calls in `main.c`.
3. Implement `menu_handle_input`.
4. Implement `renderer_draw` dispatch with text panels.
5. Add `logs_ui_push` ring buffer.

## 5) How We Can Work Together

When you finish a step, ask for a focused review like:

- "review Step 2 menu handling"
- "help implement ring buffer logs"
- "debug raycaster edge cases"

I can then give targeted changes and tests without rewriting your architecture.
