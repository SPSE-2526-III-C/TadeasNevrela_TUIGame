#ifndef MENU_H
#define MENU_H

#include "core/game_state.h"
#include "core/input.h"

void menu_handle_input(GameState *game, InputAction action);
const char *menu_screen_name(Screen screen);

#endif
