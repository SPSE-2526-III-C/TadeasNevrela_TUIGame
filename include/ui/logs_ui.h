#ifndef LOGS_UI_H
#define LOGS_UI_H

#include "core/game_state.h"

void logs_ui_push(GameState *game, const char *text);
void logs_ui_draw(const GameState *game);

#endif
