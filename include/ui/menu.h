#ifndef MENU_H
#define MENU_H

#include "core/game_state.h"
#include "core/types.h"

#define MENU_ITEM_COUNT 4

void menu_handle_input(GameState *state, InputAction action);
void menu_draw(GameState *state);

#endif