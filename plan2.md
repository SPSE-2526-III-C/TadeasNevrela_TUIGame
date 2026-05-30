# TUI Security Horror Game — Full Development Guide

This is your complete solo development roadmap. Every system is explained from
scratch: what it is, why it exists, what data it owns, what functions it needs,
and what "done" looks like. Follow the milestones in order. Each one produces
a runnable build before you move on.

---

## Mental Model — The Three Laws

Before writing any code, internalize these three rules. Every bug you will ever
have in this project will trace back to breaking one of them.

**Law 1 — Input only produces actions.**
`input_read_action()` returns an `InputAction` enum value. It never touches
`GameState`. It never draws anything. One function, one job.

**Law 2 — Update only mutates real state.**
`update_game()` changes what is actually true in the world. Doors open or
close. Integrity drops. The tick counter increments. It never calls any ncurses
function. It never draws anything.

**Law 3 — Render only reads displayed state.**
`render_game()` draws what the system claims is happening. It reads
`displayed_state` on doors, not `real_state`. It never changes anything. If
you find yourself writing `if (action == ...)` inside a render function, stop
and move that logic to update.

These three laws are what make the horror layer possible. When you want a door
to lie about its state, you change `displayed_state` in update. Render just
draws whatever it is told. No special cases needed.

---

## Project Architecture

```
src/
  main.c                   — loop, init, shutdown
  core/
    input.c                — keypress → InputAction
    game_state.c           — GameState init
  engine/
    map.c                  — tile grid, map queries
    raycaster.c            — ray math, produces column data
    renderer.c             — dispatches to the right panel
  ui/
    menu.c                 — cursor movement, screen transitions
    camera_ui.c            — draws raycaster output as ASCII
    logs_ui.c              — ring buffer, log panel rendering
    systems_ui.c           — systems health panel
  systems/
    doors.c                — door state, toggling, door list
    events.c               — timed event scheduling and firing
    integrity.c            — integrity tracking, threshold reactions

include/
  core/
    types.h                — all enums and shared typedefs
    game_state.h           — GameState struct and init declaration
    input.h                — input_read_action declaration
  engine/
    map.h
    raycaster.h
    renderer.h
  ui/
    menu.h
    camera_ui.h
    logs_ui.h
    systems_ui.h
  systems/
    doors.h
    events.h
    integrity.h
```

The rule for headers: every `.c` file has a matching `.h` file. The `.h` file
declares only what other modules are allowed to call. Internal helper functions
stay in the `.c` file and are not declared in the header.

---

## Shared Types — `include/core/types.h`

This file is included by almost everything. Define it once and never
duplicate these enums anywhere else.

```c
#ifndef TYPES_H
#define TYPES_H

/* Which panel is currently visible */
typedef enum {
    SCREEN_MENU,
    SCREEN_CAMERAS,
    SCREEN_DOORS,
    SCREEN_LOGS,
    SCREEN_SYSTEMS,
    SCREEN_EMERGENCY
} Screen;

/* Normalized input — what the player meant, not what key they pressed */
typedef enum {
    ACTION_NONE,
    ACTION_UP,
    ACTION_DOWN,
    ACTION_SELECT,
    ACTION_BACK,
    ACTION_QUIT
} InputAction;

/* Door states — used for both real_state and displayed_state */
typedef enum {
    DOOR_LOCKED,
    DOOR_UNLOCKED,
    DOOR_JAMMED,
    DOOR_UNKNOWN
} DoorState;

/* Named integrity thresholds for readability */
typedef enum {
    INTEGRITY_NORMAL       = 100,
    INTEGRITY_SUSPICION    = 75,
    INTEGRITY_CONTRADICTION = 50,
    INTEGRITY_CONTROL_LOSS = 25,
    INTEGRITY_FAILURE      = 0
} IntegrityThreshold;

/* Map tile values */
typedef enum {
    TILE_EMPTY = 0,
    TILE_WALL  = 1,
    TILE_DOOR  = 2
} TileType;

#endif
```

---

## Milestone 0 — Compile and Run ✓ (already done)

You have this. The loop runs, ncurses is initialised, `q` exits cleanly.
The menu navigates between placeholder panels.

**Your current state:**
- `main.c` has the loop with `napms(16)` frame cap
- `input.c` maps keys to `InputAction`
- `menu.c` draws a centred menu with `A_REVERSE` highlight
- `game_state.h` has `system_integrity` and `tick`
- All five screens exist as placeholder `mvprintw` stubs

---

## Milestone 1 — Logs Ring Buffer

### Why this comes first

Every system you build after this will want to write a log entry. If logs
exist now, doors, events, and integrity can all report what they do the moment
you implement them. This makes every future milestone feel alive immediately.

### What a ring buffer is

A ring buffer is a fixed-size array where new entries overwrite the oldest
ones once the array is full. There is no shifting of memory. One integer
tracks the write position and wraps back to zero when it reaches the end.

```
[ entry0 ][ entry1 ][ entry2 ][ entry3 ][ entry4 ]
                                            ^
                                           head (next write goes here)
```

When head reaches the end it wraps: `head = (head + 1) % LOG_MAX`

### Data structures — `include/ui/logs_ui.h`

```c
#ifndef LOGS_UI_H
#define LOGS_UI_H

#include "core/game_state.h"

#define LOG_MAX      64     /* total entries before oldest is overwritten */
#define LOG_LINE_MAX 72     /* max characters per entry */

typedef struct {
    char lines[LOG_MAX][LOG_LINE_MAX];
    int  head;              /* index of the next write position */
    int  count;             /* how many entries exist, capped at LOG_MAX */
} LogBuffer;

void logs_push(LogBuffer *log, const char *msg);
void logs_draw(LogBuffer *log, int start_y, int start_x, int height, int width);

#endif
```

### Where `LogBuffer` lives

Add it to `GameState`:

```c
#include "ui/logs_ui.h"

typedef struct {
    Screen     current_screen;
    int        menu_cursor;
    int        should_quit;
    int        system_integrity;
    int        tick;
    LogBuffer  logs;             /* add this */
} GameState;
```

And initialise in `game_state_init`:

```c
state->logs.head  = 0;
state->logs.count = 0;
```

### Implementing `logs_push` — `src/ui/logs_ui.c`

```c
#include <string.h>
#include <ncurses.h>
#include "ui/logs_ui.h"

void logs_push(LogBuffer *log, const char *msg) {
    strncpy(log->lines[log->head], msg, LOG_LINE_MAX - 1);
    log->lines[log->head][LOG_LINE_MAX - 1] = '\0';  /* always null-terminate */
    log->head = (log->head + 1) % LOG_MAX;
    if (log->count < LOG_MAX)
        log->count++;
}
```

### Implementing `logs_draw`

The draw function needs to show the most recent entries at the bottom, like
a real terminal log. The oldest entry at the top, newest at the bottom.

```c
void logs_draw(LogBuffer *log, int start_y, int start_x, int height, int width) {
    /* Calculate where the oldest visible entry starts.
       We want to show at most (height) lines, newest at the bottom. */
    int total   = log->count;
    int visible = (total < height) ? total : height;

    /* oldest_idx: walk back from head by (visible) steps */
    int oldest_idx = (log->head - visible + LOG_MAX) % LOG_MAX;

    for (int i = 0; i < visible; i++) {
        int idx = (oldest_idx + i) % LOG_MAX;
        mvprintw(start_y + i, start_x, "%-*.*s", width, width, log->lines[idx]);
    }
}
```

### Connecting to the logs panel in `renderer.c` / `main.c`

In your `render_game` switch, replace the logs placeholder:

```c
case SCREEN_LOGS: {
    int rows, cols;
    getmaxyx(stdscr, rows, cols);
    mvprintw(0, 0, "SECURITY LOGS");
    mvprintw(1, 0, "─────────────────────────────────────────");
    mvprintw(rows - 1, 0, "ESC back");
    logs_draw(&state->logs, 2, 0, rows - 3, cols);
    break;
}
```

### Test it

Add a temporary line in `update_game` to push a log every 60 ticks:

```c
if (state->tick % 60 == 0) {
    logs_push(&state->logs, "[00:00] System nominal");
}
```

**Done when:** logs panel fills with entries, old ones roll off the top as
new ones appear at the bottom.

---

## Milestone 2 — Doors System

### What doors are

A door is a world object that exists at a map position, has a state the
player can query, and in a later milestone can have a displayed state that
disagrees with the real one. Keep both fields from day one.

### Data structures — `include/systems/doors.h`

```c
#ifndef DOORS_H
#define DOORS_H

#include "core/types.h"

#define DOOR_MAX 8

typedef struct {
    int       id;
    int       map_x;            /* tile position on the map */
    int       map_y;
    DoorState real_state;       /* ground truth */
    DoorState displayed_state;  /* what the panel and camera claim */
    char      label[24];        /* e.g. "DOOR 01 - WEST CORRIDOR" */
    int       player_can_control; /* 1 = player may toggle, 0 = locked out */
} Door;

typedef struct {
    Door doors[DOOR_MAX];
    int  count;
    int  selected;              /* cursor index in the doors panel */
} DoorSystem;

void doors_init(DoorSystem *ds);
void doors_handle_input(DoorSystem *ds, LogBuffer *log, InputAction action);
void doors_draw(DoorSystem *ds, int start_y, int start_x, int height, int width);

#endif
```

Note: `LogBuffer` is forward-declared here. Add `#include "ui/logs_ui.h"` at
the top of the doors header. Doors write to logs when toggled.

### Implementing doors — `src/systems/doors.c`

```c
void doors_init(DoorSystem *ds) {
    ds->count    = 4;   /* start with four doors */
    ds->selected = 0;

    /* Set up each door manually for the first build */
    ds->doors[0] = (Door){0, 5, 3, DOOR_LOCKED,   DOOR_LOCKED,   "DOOR 01 - NORTH WING",  1};
    ds->doors[1] = (Door){1, 5, 7, DOOR_UNLOCKED, DOOR_UNLOCKED, "DOOR 02 - EAST HALL",   1};
    ds->doors[2] = (Door){2, 9, 3, DOOR_LOCKED,   DOOR_LOCKED,   "DOOR 03 - SERVER ROOM", 0};
    ds->doors[3] = (Door){3, 2, 9, DOOR_UNLOCKED, DOOR_UNLOCKED, "DOOR 04 - SOUTH EXIT",  1};
}
```

The toggle function only affects `real_state`. Corruption will later desync
`displayed_state` separately. Never write to `displayed_state` here.

```c
static void door_toggle(Door *d, LogBuffer *log) {
    if (!d->player_can_control) {
        logs_push(log, "ACCESS DENIED");
        return;
    }
    if (d->real_state == DOOR_LOCKED) {
        d->real_state      = DOOR_UNLOCKED;
        d->displayed_state = DOOR_UNLOCKED;
        char msg[LOG_LINE_MAX];
        snprintf(msg, sizeof(msg), "[SYS] %s unlocked", d->label);
        logs_push(log, msg);
    } else if (d->real_state == DOOR_UNLOCKED) {
        d->real_state      = DOOR_LOCKED;
        d->displayed_state = DOOR_LOCKED;
        char msg[LOG_LINE_MAX];
        snprintf(msg, sizeof(msg), "[SYS] %s locked", d->label);
        logs_push(log, msg);
    }
}
```

The input handler moves the cursor and calls toggle on select:

```c
void doors_handle_input(DoorSystem *ds, LogBuffer *log, InputAction action) {
    switch (action) {
        case ACTION_UP:
            if (ds->selected > 0) ds->selected--;
            break;
        case ACTION_DOWN:
            if (ds->selected < ds->count - 1) ds->selected++;
            break;
        case ACTION_SELECT:
            door_toggle(&ds->doors[ds->selected], log);
            break;
        default:
            break;
    }
}
```

The draw function uses `displayed_state`, not `real_state`:

```c
static const char *door_state_label(DoorState s) {
    switch (s) {
        case DOOR_LOCKED:   return "LOCKED  ";
        case DOOR_UNLOCKED: return "UNLOCKED";
        case DOOR_JAMMED:   return "JAMMED  ";
        case DOOR_UNKNOWN:  return "UNKNOWN ";
    }
    return "?";
}

void doors_draw(DoorSystem *ds, int start_y, int start_x, int height, int width) {
    (void)height; (void)width;
    mvprintw(start_y, start_x, "DOOR CONTROL");
    mvprintw(start_y + 1, start_x, "────────────────────────────────────");

    for (int i = 0; i < ds->count; i++) {
        Door *d = &ds->doors[i];
        if (i == ds->selected) attron(A_REVERSE);
        mvprintw(start_y + 3 + i, start_x, "  [%s]  %s",
                 door_state_label(d->displayed_state), d->label);
        if (i == ds->selected) attroff(A_REVERSE);
    }

    mvprintw(start_y + 3 + ds->count + 1, start_x,
             "ENTER toggle   ESC back");
}
```

### Add to `GameState`

```c
#include "systems/doors.h"

typedef struct {
    ...
    DoorSystem doors;
} GameState;
```

Initialise: `doors_init(&state->doors);`

Add to `update_game`:

```c
case SCREEN_DOORS:
    doors_handle_input(&state->doors, &state->logs, action);
    break;
```

Add to `render_game`:

```c
case SCREEN_DOORS:
    doors_draw(&state->doors, 0, 0, rows, cols);
    break;
```

**Done when:** doors panel lists doors with states, ENTER toggles a door,
a log entry appears in the logs panel reflecting the change.

---

## Milestone 3 — Map

### What the map is

The map is a 2D grid of integers. Each integer is a `TileType`. The map
is the source of truth for what exists in the world. The raycaster reads
it. Doors have positions on it.

### Data structures — `include/engine/map.h`

```c
#ifndef MAP_H
#define MAP_H

#include "core/types.h"

#define MAP_W 16
#define MAP_H 16

typedef struct {
    int tiles[MAP_H][MAP_W];
} Map;

void    map_init(Map *m);
int     map_get_tile(const Map *m, int x, int y);
int     map_is_wall(const Map *m, int x, int y);   /* 1 = blocks rays */

#endif
```

### Implementing — `src/engine/map.c`

Hardcode a small map for the first version. Use the tile enum values.
`1` is wall, `0` is empty space, `2` is door.

```c
void map_init(Map *m) {
    int layout[MAP_H][MAP_W] = {
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,0,0,1,1,1,2,1,1,0,0,0,0,0,0,1},
        {1,0,0,1,0,0,0,0,1,0,0,0,0,0,0,1},
        {1,0,0,1,0,0,0,0,1,0,0,0,0,0,0,1},
        {1,0,0,2,0,0,0,0,2,0,0,0,0,0,0,1},
        {1,0,0,1,0,0,0,0,1,0,0,0,0,0,0,1},
        {1,0,0,1,1,1,2,1,1,0,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,0,0,1,1,1,0,0,0,1},
        {1,0,0,0,0,0,0,0,0,1,0,1,0,0,0,1},
        {1,0,0,0,0,0,0,0,0,1,0,1,0,0,0,1},
        {1,0,0,0,0,0,0,0,0,1,1,1,0,0,0,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    };
    for (int y = 0; y < MAP_H; y++)
        for (int x = 0; x < MAP_W; x++)
            m->tiles[y][x] = layout[y][x];
}

int map_get_tile(const Map *m, int x, int y) {
    if (x < 0 || x >= MAP_W || y < 0 || y >= MAP_H) return TILE_WALL;
    return m->tiles[y][x];
}

int map_is_wall(const Map *m, int x, int y) {
    int t = map_get_tile(m, x, y);
    return (t == TILE_WALL);
    /* Doors block rays when locked — handled in raycaster using DoorSystem */
}
```

Add `Map map` to `GameState` and call `map_init(&state->map)`.

**Done when:** map compiles and `map_is_wall` returns correct results for
hardcoded positions.

---

## Milestone 4 — Raycaster and Camera

### What raycasting is

Each camera renders a pseudo-3D view by casting one ray per screen column.
The ray travels through the tile grid until it hits a wall. The distance to
the wall determines how tall to draw a vertical stripe on that column. Close
walls produce tall stripes. Distant walls produce short stripes.

This is the same technique as Wolfenstein 3D, simplified for ASCII output.

### Camera data — `include/engine/raycaster.h`

```c
#ifndef RAYCASTER_H
#define RAYCASTER_H

#include "engine/map.h"
#include "systems/doors.h"

#define CAM_MAX 4          /* maximum number of cameras in the facility */

typedef struct {
    float x;               /* position in map units */
    float y;
    float angle;           /* direction in radians */
    float fov;             /* field of view in radians, typically 1.0–1.4 */
    int   active;          /* 1 = feed is live, 0 = feed is dead */
} Camera;

/* RayHit is what the raycaster returns for one column */
typedef struct {
    float    distance;
    TileType tile;         /* what was hit */
} RayHit;

void    cameras_init(Camera cams[CAM_MAX]);
RayHit  cast_ray(const Map *m, const DoorSystem *ds,
                 float ox, float oy, float angle);

#endif
```

### Camera positions

Place cameras so they point at interesting parts of your hardcoded map:

```c
void cameras_init(Camera cams[CAM_MAX]) {
    cams[0] = (Camera){1.5f, 1.5f,  0.0f,       1.2f, 1};  /* north-west */
    cams[1] = (Camera){4.5f, 4.5f,  3.14159f,   1.2f, 1};  /* inner room */
    cams[2] = (Camera){1.5f, 13.5f, -1.5708f,   1.2f, 1};  /* south corridor */
    cams[3] = (Camera){10.5f, 11.5f, 1.5708f,   1.2f, 0};  /* offline */
}
```

### The ray casting algorithm — `src/engine/raycaster.c`

This is a DDA (Digital Differential Analysis) algorithm. It steps through
the grid efficiently without floating point per-step errors.

```c
#include <math.h>
#include "engine/raycaster.h"

RayHit cast_ray(const Map *m, const DoorSystem *ds,
                float ox, float oy, float angle) {
    RayHit hit = {0};

    float ray_dx = cosf(angle);
    float ray_dy = sinf(angle);

    /* Which tile the ray is currently in */
    int map_x = (int)ox;
    int map_y = (int)oy;

    /* How far along the ray to cross one tile boundary in x or y */
    float delta_x = (ray_dx == 0) ? 1e30f : fabsf(1.0f / ray_dx);
    float delta_y = (ray_dy == 0) ? 1e30f : fabsf(1.0f / ray_dy);

    /* Step direction and initial side distances */
    int step_x, step_y;
    float side_x, side_y;

    if (ray_dx < 0) {
        step_x = -1;
        side_x = (ox - map_x) * delta_x;
    } else {
        step_x = 1;
        side_x = (map_x + 1.0f - ox) * delta_x;
    }
    if (ray_dy < 0) {
        step_y = -1;
        side_y = (oy - map_y) * delta_y;
    } else {
        step_y = 1;
        side_y = (map_y + 1.0f - oy) * delta_y;
    }

    /* Walk until wall hit, max 32 steps */
    int hit_side = 0;  /* 0 = x side, 1 = y side */
    for (int i = 0; i < 32; i++) {
        if (side_x < side_y) {
            side_x  += delta_x;
            map_x   += step_x;
            hit_side = 0;
        } else {
            side_y  += delta_y;
            map_y   += step_y;
            hit_side = 1;
        }

        int tile = map_get_tile(m, map_x, map_y);
        if (tile == TILE_WALL) {
            hit.tile = TILE_WALL;
            break;
        }
        if (tile == TILE_DOOR) {
            /* Check if this door is currently blocking */
            for (int d = 0; d < ds->count; d++) {
                if (ds->doors[d].map_x == map_x &&
                    ds->doors[d].map_y == map_y &&
                    ds->doors[d].real_state == DOOR_LOCKED) {
                    hit.tile = TILE_DOOR;
                    goto ray_done;
                }
            }
        }
    }

ray_done:
    /* Correct for fisheye by using perpendicular distance */
    if (hit_side == 0)
        hit.distance = (map_x - ox + (1 - step_x) / 2.0f) / ray_dx;
    else
        hit.distance = (map_y - oy + (1 - step_y) / 2.0f) / ray_dy;

    if (hit.distance < 0.1f) hit.distance = 0.1f;   /* avoid division by zero */

    return hit;
}
```

Add `#include <math.h>` and `-lm` to your Makefile `LIBS`:

```makefile
LIBS := -lncurses -lm
```

### Drawing the camera feed — `src/ui/camera_ui.c`

```c
#include <ncurses.h>
#include <math.h>
#include "ui/camera_ui.h"
#include "engine/raycaster.h"

/* Character set: close → far */
static const char wall_chars[] = {'#', '|', ':', '.', ' '};
#define WALL_CHAR_COUNT 5

static char pick_char(float distance) {
    if (distance < 1.5f) return wall_chars[0];
    if (distance < 3.0f) return wall_chars[1];
    if (distance < 5.0f) return wall_chars[2];
    if (distance < 8.0f) return wall_chars[3];
    return wall_chars[4];
}

void camera_draw_feed(const Map *m, const DoorSystem *ds,
                      const Camera *cam,
                      int start_y, int start_x,
                      int height, int width) {
    if (!cam->active) {
        mvprintw(start_y + height / 2, start_x + width / 2 - 7,
                 "--- NO SIGNAL ---");
        return;
    }

    for (int col = 0; col < width; col++) {
        /* Ray angle for this column */
        float ray_angle = cam->angle
                        - (cam->fov / 2.0f)
                        + (cam->fov * col / (float)width);

        RayHit hit = cast_ray(m, ds, cam->x, cam->y, ray_angle);

        /* Wall height in terminal rows */
        int wall_h = (int)(height / hit.distance);
        if (wall_h > height) wall_h = height;

        int top    = (height - wall_h) / 2;
        int bottom = top + wall_h;
        char ch    = pick_char(hit.distance);

        /* Door gets a different character */
        if (hit.tile == TILE_DOOR) ch = '+';

        for (int row = 0; row < height; row++) {
            if (row >= top && row < bottom)
                mvaddch(start_y + row, start_x + col, ch);
            else
                mvaddch(start_y + row, start_x + col, ' ');
        }
    }
}
```

### Camera panel — `include/ui/camera_ui.h`

```c
#ifndef CAMERA_UI_H
#define CAMERA_UI_H

#include "engine/map.h"
#include "engine/raycaster.h"
#include "systems/doors.h"

void camera_draw_feed(const Map *m, const DoorSystem *ds,
                      const Camera *cam,
                      int start_y, int start_x,
                      int height, int width);

#endif
```

Add `Camera cameras[CAM_MAX]` and `int active_camera` to `GameState`.
Call `cameras_init(state->cameras)` and set `state->active_camera = 0`.

In `render_game` cameras case:

```c
case SCREEN_CAMERAS: {
    int rows, cols;
    getmaxyx(stdscr, rows, cols);
    mvprintw(0, 0, "CAM %02d  |  ESC back  |  </> switch camera",
             state->active_camera + 1);
    camera_draw_feed(&state->map, &state->doors,
                     &state->cameras[state->active_camera],
                     1, 0, rows - 2, cols);
    break;
}
```

Add camera switching to `update_game`:

```c
case SCREEN_CAMERAS:
    if (action == ACTION_UP && state->active_camera > 0)
        state->active_camera--;
    if (action == ACTION_DOWN && state->active_camera < CAM_MAX - 1)
        state->active_camera++;
    break;
```

**Done when:** selecting Cameras shows a stable ASCII corridor. Moving between
cameras shows different views. A locked door tile renders as `+`.

---

## Milestone 5 — Events System

### What events are

Events are things that happen over time without the player doing anything.
An event has a tick timestamp when it fires and a function pointer or type
enum that describes what it does. Simple events just push a log entry.
Later events will corrupt door displayed states or degrade the camera feed.

### Data structures — `include/systems/events.h`

```c
#ifndef EVENTS_H
#define EVENTS_H

#include "core/game_state.h"

#define EVENT_MAX 32

typedef enum {
    EVENT_LOG_MESSAGE,       /* push a string to the log */
    EVENT_DOOR_JAM,          /* jam a door (real_state → JAMMED) */
    EVENT_DOOR_DESYNC,       /* displayed_state disagrees with real_state */
    EVENT_CAMERA_OFFLINE,    /* mark a camera as inactive */
    EVENT_INTEGRITY_HIT,     /* reduce integrity by a fixed amount */
} EventType;

typedef struct {
    int       fire_tick;     /* tick value when this event triggers */
    EventType type;
    int       target_id;     /* door id, camera index, etc. */
    char      message[72];   /* used by EVENT_LOG_MESSAGE */
    int       fired;         /* 1 = already triggered, skip it */
} Event;

typedef struct {
    Event events[EVENT_MAX];
    int   count;
} EventSystem;

void events_init(EventSystem *es);
void events_process(EventSystem *es, GameState *state);

#endif
```

### Implementing — `src/systems/events.c`

```c
void events_init(EventSystem *es) {
    es->count = 0;

    /* Schedule events at specific ticks.
       At 60fps: 60 ticks = 1 second, 3600 ticks = 1 minute */

    /* Early normal-looking messages */
    es->events[es->count++] = (Event){180,  EVENT_LOG_MESSAGE, 0,
        "[02:14] Motion detected - NORTH WING", 0};
    es->events[es->count++] = (Event){600,  EVENT_LOG_MESSAGE, 0,
        "[02:15] Door 02 accessed", 0};

    /* First sign of trouble */
    es->events[es->count++] = (Event){1200, EVENT_LOG_MESSAGE, 0,
        "[02:17] *** SIGNAL ANOMALY - CAM 03 ***", 0};
    es->events[es->count++] = (Event){1500, EVENT_INTEGRITY_HIT, 5, "", 0};

    /* Contradiction — log says something the player didn't do */
    es->events[es->count++] = (Event){2400, EVENT_LOG_MESSAGE, 0,
        "[02:21] Door 03 opened by operator", 0};

    /* Door starts lying */
    es->events[es->count++] = (Event){3000, EVENT_DOOR_DESYNC, 1, "", 0};

    /* Camera goes dark */
    es->events[es->count++] = (Event){3600, EVENT_CAMERA_OFFLINE, 2, "", 0};
    es->events[es->count++] = (Event){3600, EVENT_LOG_MESSAGE, 0,
        "[02:31] CAM 03 feed terminated", 0};
}

void events_process(EventSystem *es, GameState *state) {
    for (int i = 0; i < es->count; i++) {
        Event *e = &es->events[i];
        if (e->fired || state->tick < e->fire_tick)
            continue;

        e->fired = 1;

        switch (e->type) {
            case EVENT_LOG_MESSAGE:
                logs_push(&state->logs, e->message);
                break;

            case EVENT_INTEGRITY_HIT:
                state->system_integrity -= e->target_id;
                if (state->system_integrity < 0)
                    state->system_integrity = 0;
                break;

            case EVENT_DOOR_DESYNC:
                if (e->target_id < state->doors.count) {
                    Door *d = &state->doors.doors[e->target_id];
                    /* displayed_state disagrees with real_state */
                    d->displayed_state = (d->real_state == DOOR_LOCKED)
                                       ? DOOR_UNLOCKED : DOOR_LOCKED;
                }
                break;

            case EVENT_CAMERA_OFFLINE:
                if (e->target_id < CAM_MAX)
                    state->cameras[e->target_id].active = 0;
                break;

            case EVENT_DOOR_JAM:
                if (e->target_id < state->doors.count)
                    state->doors.doors[e->target_id].real_state = DOOR_JAMMED;
                break;
        }
    }
}
```

Add `EventSystem events` to `GameState`, call `events_init`, and call
`events_process(&state->events, state)` inside `update_game` every tick,
outside the input switch.

**Done when:** log entries appear on schedule, a camera eventually dies,
a door panel entry starts showing the wrong state.

---

## Milestone 6 — Integrity and Horror Escalation

### What integrity does

Integrity is a number from 100 to 0. It has no visual health bar. The
player can see it in the top-right corner. What changes is system behaviour.
Each threshold unlocks different corruption effects.

### Implementing — `src/systems/integrity.c`

```c
#include "systems/integrity.h"
#include "core/game_state.h"

void integrity_update(GameState *state) {
    /* Passive decay — one point every 10 seconds at 60fps */
    if (state->tick % 600 == 0 && state->system_integrity > 0)
        state->system_integrity--;
}

int integrity_stage(int integrity) {
    if (integrity >= INTEGRITY_SUSPICION)    return 0;  /* normal */
    if (integrity >= INTEGRITY_CONTRADICTION) return 1; /* suspicion */
    if (integrity >= INTEGRITY_CONTROL_LOSS) return 2;  /* contradiction */
    if (integrity > INTEGRITY_FAILURE)       return 3;  /* control loss */
    return 4;                                            /* failure */
}
```

### Using the stage

Check `integrity_stage` in render functions to alter output:

```c
/* In camera_draw_feed, after picking ch: */
int stage = integrity_stage(integrity);

/* Stage 1: randomly flicker one character */
if (stage >= 1 && (rand() % 100) < 2)
    ch = '?';

/* Stage 2: occasionally replace a wall column with an entity marker */
if (stage >= 2 && col == width / 2 && (rand() % 100) < 5)
    ch = '@';
```

```c
/* In doors_draw, when rendering the state label: */
int stage = integrity_stage(integrity);

/* Stage 2: randomly show wrong state label */
if (stage >= 2 && (rand() % 100) < 10)
    mvprintw(..., "  [UNKNOWN]  %s", d->label);
else
    mvprintw(..., "  [%s]  %s", door_state_label(d->displayed_state), d->label);
```

```c
/* In menu_draw, at stage 3: change a label briefly */
if (integrity_stage(state->system_integrity) >= 3) {
    if ((state->tick / 30) % 2 == 0)
        /* flash a menu item label */
}
```

Pass `state->system_integrity` into draw functions that need it by adding
an `int integrity` parameter.

### Failure state

In `render_game`, check for integrity zero before the normal switch:

```c
void render_game(GameState *state) {
    erase();

    if (state->system_integrity <= 0) {
        int rows, cols;
        getmaxyx(stdscr, rows, cols);
        attron(A_BOLD);
        mvprintw(rows/2 - 1, (cols - 22) / 2, "ALL MODULES COMPROMISED");
        mvprintw(rows/2,     (cols - 24) / 2, "TERMINAL SESSION TERMINATED");
        attroff(A_BOLD);
        refresh();
        return;
    }

    /* normal switch ... */
}
```

**Done when:** a session that runs long enough visibly shifts through normal,
flickering, contradicting, and collapsing states ending in the failure screen.

---

## Milestone 7 — Polish Pass

Once all systems work, make the whole thing feel like a real terminal:

**Colours** — ncurses supports colour pairs. Add them via `start_color()` and
`init_pair()`. Use green on black for normal log text, yellow for warnings,
red for critical. Keep the menu white on black.

```c
start_color();
init_pair(1, COLOR_GREEN,  COLOR_BLACK);  /* normal */
init_pair(2, COLOR_YELLOW, COLOR_BLACK);  /* warning */
init_pair(3, COLOR_RED,    COLOR_BLACK);  /* critical */
```

Apply with `attron(COLOR_PAIR(1))` and `attroff(COLOR_PAIR(1))`.

**Timestamp logs** — use `state->tick` to generate fake timestamps:

```c
int seconds = state->tick / 60;
int minutes = seconds / 60;
snprintf(msg, LOG_LINE_MAX, "[%02d:%02d] %s", minutes, seconds % 60, text);
```

**Window borders** — draw panel borders with `mvhline` and `mvvline` for a
more terminal-OS feel. ncurses provides these as part of the standard library.

**Bootup sequence** — before entering the main loop, display a short
fake boot text:

```c
mvprintw(0, 0, "SENTINEL SECURITY OS v2.3.1");
mvprintw(1, 0, "Initialising modules...");
refresh();
napms(800);
mvprintw(2, 0, "Camera subsystem........OK");
refresh();
napms(400);
/* etc. */
```

---

## Key Rules to Keep While Coding Solo

- **Compile after every function.** Not every file, every function. Catching
  errors early is faster than debugging 200 lines.

- **Never skip the header.** Every new function gets declared in its module
  header the same session you write it.

- **Render reads displayed state.** If a render function ever reads
  `real_state` directly, stop and ask whether that is intentional.

- **Events own the horror timeline.** Do not hardcode corruption into update
  logic based on tick comparisons scattered around. Keep the event schedule
  in `events_init` where you can read the whole story in one place.

- **One module per session.** Finish the logs buffer completely before
  starting doors. Finish doors before touching the raycaster. Partial systems
  that compile are better than two half-working ones.

---

## Completion Checklist

- [ ] Milestone 0 — loop runs, menu navigates, q exits
- [ ] Milestone 1 — logs ring buffer, panel fills and rolls
- [ ] Milestone 2 — doors panel, toggle, log entry on change
- [ ] Milestone 3 — map compiles, map_is_wall correct
- [ ] Milestone 4 — camera feed renders ASCII corridor
- [ ] Milestone 5 — events fire on schedule, camera dies, door desyncs
- [ ] Milestone 6 — integrity decays, failure screen triggers
- [ ] Milestone 7 — colours, timestamps, boot sequence

---

## Quick Reference — Data Flow

```
input_read_action()
        │
        ▼
update_game(state, action)
  ├── menu_handle_input
  ├── doors_handle_input
  ├── events_process         ← runs every tick regardless of screen
  └── integrity_update       ← runs every tick regardless of screen
        │
        ▼
render_game(state)
  ├── menu_draw
  ├── camera_draw_feed       ← reads displayed door state via DoorSystem
  ├── doors_draw             ← reads displayed_state only
  └── logs_draw              ← reads LogBuffer
```

Everything flows downward. Nothing in render touches real state.
Nothing in input touches GameState.