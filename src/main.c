#define _POSIX_C_SOURCE 200809L
#include <ncurses.h>
#include <unistd.h>
#include "core/game_state.h"
#include "core/input.h"
#include "ui/menu.h"

/* Keep the main loop focused by splitting update and render work. */
void update_game(GameState *state, InputAction action);
void render_game(GameState *state);

int main(void) {
    /* Switch the terminal into ncurses mode and configure interactive input. */
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    nodelay(stdscr, TRUE);
    curs_set(0);

    /* Set up the game state before the first frame. */
    GameState state;
    game_state_init(&state);

    /* Run until the user requests quit. */
    while (!state.should_quit) {
        InputAction action = input_read_action();
        update_game(&state, action);
        render_game(&state);

        /* Advance the frame counter and throttle the loop to ~60 FPS. */
        state.tick++;
        napms(16);
    }

    /* Restore the terminal so the shell behaves normally again. */
    endwin();
    return 0;
}

void update_game(GameState *state, InputAction action) {
    /* Quit is handled globally so it works from any screen. */
    if (action == ACTION_QUIT) {
        state->should_quit = 1;
        return;
    }

    /* The menu owns navigation and selection; other screens only handle back. */
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
    /* Clear the previous frame so the new frame can be redrawn cleanly. */
    erase();

    /* Draw the active screen. */
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

    /* Send the drawn frame to the terminal. */
    refresh();
}

/* Initialize every field with a predictable starting value. */
void game_state_init(GameState *state) {
    state->current_screen   = SCREEN_MENU;
    state->menu_cursor      = 0;
    state->should_quit      = 0;
    state->system_integrity = 100;
    state->tick             = 0;
}
