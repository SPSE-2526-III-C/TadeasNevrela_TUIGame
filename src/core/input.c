#include <ncurses.h>
#include "core/input.h"

/* Translate raw ncurses key codes into the game-specific action enum. */
InputAction input_read_action(void) {
    int ch = getch();

    switch (ch) {
        case KEY_UP:
            return ACTION_UP;
        case KEY_DOWN:
            return ACTION_DOWN;
        case '\n':
            return ACTION_SELECT;
        case 27:
            return ACTION_BACK;  /* ESC */
        case 'q':
            return ACTION_QUIT;
        default:
            return ACTION_NONE;
    }
}
