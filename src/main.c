#include <ncurses.h>

#include "core/game_state.h"
#include "core/input.h"

/* Forward declarations — implement these as you fill in each module */
void update_game(GameState *state, InputAction action);
void render_game(GameState *state);

int main(void) {
    /* ncurses setup */
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    nodelay(stdscr, TRUE);   /* non-blocking input so loop runs freely */
    curs_set(0);

    GameState state;
    game_state_init(&state);

    while (!state.should_quit) {
        InputAction action = input_read_action();
        update_game(&state, action);
        render_game(&state);
    }

    endwin();   /* restore terminal */
    return 0;
}

/* ------------------------------------------------------------------ */
/* Stub implementations — replace these as you build each module       */
/* ------------------------------------------------------------------ */

void update_game(GameState *state, InputAction action) {
    if (action == ACTION_QUIT) {
        state->should_quit = 1;
    }
    /* TODO: call menu_handle_input, doors_update, events_update, etc. */
}

void render_game(GameState *state) {
    clear();
    /* TODO: call renderer_draw(state) once renderer.c is implemented  */

    /* Temporary: just show current screen name so you know it works   */
    const char *names[] = {"MENU","CAMERAS","DOORS","LOGS","SYSTEMS"};
    mvprintw(0, 0, "Screen: %s", names[state->current_screen]);
    mvprintw(1, 0, "Press q to quit");
    refresh();
}



















/* bottom of main.c or in a new src/core/game_state.c */
#include "core/game_state.h"

void game_state_init(GameState *state) {
    state->current_screen = SCREEN_MENU;
    state->menu_cursor    = 0;
    state->should_quit    = 0;
}