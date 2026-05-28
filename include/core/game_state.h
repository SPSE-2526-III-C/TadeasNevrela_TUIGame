#ifndef GAME_STATE_H
#define GAME_STATE_H

#include "types.h"

typedef struct {
    Screen current_screen;
    int    menu_cursor;
    int    should_quit;
} GameState;

void game_state_init(GameState *state);

#endif