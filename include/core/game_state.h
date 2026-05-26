#ifndef GAME_STATE_H
#define GAME_STATE_H

#include "core/types.h"

typedef struct {
    int tick;
    int event_timer;
    bool running;
    MenuState menu;
    Camera camera;
    Map map;
    Door doors[MAX_DOORS];
    size_t door_count;
    LogBuffer logs;
    IntegrityState integrity;
} GameState;

void game_state_init(GameState *game);

#endif
