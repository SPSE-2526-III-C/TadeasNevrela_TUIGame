#include <stdio.h>
#include <string.h>

#include "core/game_state.h"

void game_state_init(GameState *game) {
    memset(game, 0, sizeof(*game));
    game->running = true;
    game->menu.current = SCREEN_MAIN_MENU;
    game->integrity.integrity = 100;
}

int main(void) {
    GameState game;

    game_state_init(&game);
    puts("TUI Game scaffold is ready.");
    puts("Read GUIDE.md for the build order and module tutorial.");
    return 0;
}
