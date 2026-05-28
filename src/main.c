#define _POSIX_C_SOURCE 200809L
#include <ncurses.h>
#include <unistd.h>
#include "core/game_state.h"
#include "core/input.h"
#include "ui/menu.h"

void update_game(GameState *state, InputAction action);
void render_game(GameState *state);

int main(void) {
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    nodelay(stdscr, TRUE);
    curs_set(0);

    GameState state;
    game_state_init(&state);

    while (!state.should_quit) {
        InputAction action = input_read_action();
        update_game(&state, action);
        render_game(&state);

        state.tick++;
        napms(16);   /* ~60fps cap — eliminates pointless redraws */
    }

    endwin();
    return 0;
}

void update_game(GameState *state, InputAction action) {
    if (action == ACTION_QUIT) {
        state->should_quit = 1;
        return;
    }
    switch (state->current_screen) {
        case SCREEN_MENU:
            menu_handle_input(state, action);
            break;
        default:
            if (action == ACTION_BACK)
                state->current_screen = SCREEN_MENU;
            break;
    }
}

void render_game(GameState *state) {
    erase();
    switch (state->current_screen) {
        case SCREEN_MENU:
            menu_draw(state);
            break;
        case SCREEN_CAMERAS:
            mvprintw(0, 0, "[CAMERAS]  ESC to go back");
            break;
        case SCREEN_DOORS:
            mvprintw(0, 0, "[DOORS]    ESC to go back");
            break;
        case SCREEN_LOGS:
            mvprintw(0, 0, "[LOGS]     ESC to go back");
            break;
        case SCREEN_SYSTEMS:
            mvprintw(0, 0, "[SYSTEMS]  ESC to go back");
            break;
        case SCREEN_EMERGENCY:
            mvprintw(0, 0, "[EMERGENCY PROTOCOLS]  ESC to go back");
            break;
    }
    refresh();
}


















/* bottom of main.c or in a new src/core/game_state.c */
#include "core/game_state.h"

void game_state_init(GameState *state) {
    state->current_screen     = SCREEN_MENU;
    state->menu_cursor        = 0;
    state->should_quit        = 0;
    state->system_integrity   = 100;
}