#ifndef GAME_STATE_H
#define GAME_STATE_H

#include "core/types.h"

/* All mutable state that drives the current frame and UI flow. */
typedef struct {
    Screen current_screen;   /* Which screen is currently visible. */
    int menu_cursor;         /* Highlighted menu item index. */
    int should_quit;         /* Nonzero when the main loop should exit. */

    int system_integrity;    /* 100 = healthy, 0 = collapsed. */
    int tick;                /* Frame counter used for timed events. */
} GameState;

/* Initialize a GameState with safe default values. */
void game_state_init(GameState *state);

#endif
