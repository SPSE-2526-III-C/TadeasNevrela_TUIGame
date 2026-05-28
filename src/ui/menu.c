#include <ncurses.h>
#include <string.h>
#include "ui/menu.h"

static const char *menu_items[] = {
    "Cameras",
    "Doors",
    "Security Logs",
    "Systems",
    "Emergency Protocols"
};

static const Screen menu_targets[] = {
    SCREEN_CAMERAS,
    SCREEN_DOORS,
    SCREEN_LOGS,
    SCREEN_SYSTEMS,
    SCREEN_EMERGENCY
};

void menu_handle_input(GameState *state, InputAction action) {
    switch (action) {
        case ACTION_UP:
            if (state->menu_cursor > 0)
                state->menu_cursor--;
            break;
        case ACTION_DOWN:
            if (state->menu_cursor < MENU_ITEM_COUNT - 1)
                state->menu_cursor++;
            break;
        case ACTION_SELECT:
            state->current_screen = menu_targets[state->menu_cursor];
            break;
        default:
            break;
    }
}

void menu_draw(GameState *state) {
    int rows, cols;
    getmaxyx(stdscr, rows, cols);   /* actual terminal dimensions */

    /* Center the title */
    const char *title = "SECURITY TERMINAL v1.0";
    int title_x = (cols - (int)strlen(title)) / 2;
    int start_y = rows / 4;        /* start a quarter down the screen */

    attron(A_BOLD);
    mvprintw(start_y, title_x, "%s", title);
    attroff(A_BOLD);

    mvprintw(start_y + 1, title_x, "----------------------");

    /* Menu items centered under title */
    for (int i = 0; i < MENU_ITEM_COUNT; i++) {
        int item_x = (cols - (int)strlen(menu_items[i]) - 4) / 2;
        if (i == state->menu_cursor) {
            attron(A_REVERSE);     /* highlight selected row */
            mvprintw(start_y + 3 + i, item_x, "  %s  ", menu_items[i]);
            attroff(A_REVERSE);
        } else {
            mvprintw(start_y + 3 + i, item_x, "  %s  ", menu_items[i]);
        }
    }

    /* Footer at bottom of terminal */
    const char *footer = "↑↓ move   ENTER select   Q quit";
    mvprintw(rows - 1, (cols - (int)strlen(footer)) / 2, "%s", footer);

    /* Integrity bar top-right */
    mvprintw(0, cols - 20, "INTEGRITY: %3d%%", state->system_integrity);
}