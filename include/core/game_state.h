#ifndef GAME_STATE_H
#define GAME_STATE_H

#include "core/types.h"

typedef struct {
    Screen  current_screen;
    int     menu_cursor;
    int     should_quit;

    int     system_integrity;   /* 100 = healthy, 0 = collapsed */
    int     tick;               /* frame counter for timed events */
} GameState;

void game_state_init(GameState *state);

#endif