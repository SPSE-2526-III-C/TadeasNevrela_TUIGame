#ifndef MENU_H
#define MENU_H

#include "core/game_state.h"
#include "core/types.h"

/* The menu currently exposes five entries. */
#define MENU_ITEM_COUNT 5

/* Handle up/down/select input while the menu is active. */
void menu_handle_input(GameState *state, InputAction action);

/* Draw the main menu and status text. */
void menu_draw(GameState *state);

#endif
