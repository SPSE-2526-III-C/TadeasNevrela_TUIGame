#include <ncurses.h>
#include <string.h>
#include "ui/menu.h"

/* Menu labels shown to the player. */
static const char *menu_items[] = {
    "Cameras",
    "Doors",
    "Security Logs",
    "Systems",
    "Emergency Protocols"
};

/* Target screen for each menu item, in the same order as menu_items. */
static const Screen menu_targets[] = {
    SCREEN_CAMERAS,
    SCREEN_DOORS,
    SCREEN_LOGS,
    SCREEN_SYSTEMS,
    SCREEN_EMERGENCY
};

void menu_handle_input(GameState *state, InputAction action) {
    /* Update the cursor position or open the selected screen. */
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

    /* Get the current terminal size so the layout stays centered. */
    getmaxyx(stdscr, rows, cols);

    /* Position the title near the top-center of the screen. */
    const char *title = "SECURITY TERMINAL v1.0";
    int title_x = (cols - (int)strlen(title)) / 2;
    int start_y = rows / 4;

    attron(A_BOLD);
    mvprintw(start_y, title_x, "%s", title);
    attroff(A_BOLD);

    mvprintw(start_y + 1, title_x, "----------------------");

    /* Draw each menu entry, highlighting the currently selected one. */
    for (int i = 0; i < MENU_ITEM_COUNT; i++) {
        int item_x = (cols - (int)strlen(menu_items[i]) - 4) / 2;

        if (i == state->menu_cursor) {
            attron(A_REVERSE);
            mvprintw(start_y + 3 + i, item_x, "  %s  ", menu_items[i]);
            attroff(A_REVERSE);
        } else {
            mvprintw(start_y + 3 + i, item_x, "  %s  ", menu_items[i]);
        }
    }

    /* Show controls in the footer. */
    const char *footer = "/\\ \\/ move   ENTER select   Q quit";
    mvprintw(rows - 1, (cols - (int)strlen(footer)) / 2, "%s", footer);

    /* Display the current system integrity in the top-right corner. */
    mvprintw(0, cols - 20, "INTEGRITY: %3d%%", state->system_integrity);
}
