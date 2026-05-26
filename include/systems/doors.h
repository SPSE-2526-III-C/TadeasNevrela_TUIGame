#ifndef DOORS_H
#define DOORS_H

#include "core/game_state.h"

void doors_init(GameState *game);
void doors_toggle_selected(GameState *game);
const char *doors_state_to_string(DoorState state);

#endif
