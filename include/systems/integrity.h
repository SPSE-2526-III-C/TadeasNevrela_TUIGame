#ifndef INTEGRITY_H
#define INTEGRITY_H

#include "core/game_state.h"

void integrity_init(GameState *game);
void integrity_apply_decay(GameState *game, int amount);

#endif
