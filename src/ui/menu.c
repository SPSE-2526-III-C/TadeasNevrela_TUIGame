#include "ui/menu.h"

const char *menu_screen_name(Screen screen) {
    switch (screen) {
        case SCREEN_CAMERAS:
            return "Cameras";
        case SCREEN_DOORS:
            return "Doors";
        case SCREEN_LOGS:
            return "Security Logs";
        case SCREEN_SYSTEMS:
            return "Systems";
        case SCREEN_EMERGENCY:
            return "Emergency Protocols";
        case SCREEN_MAIN_MENU:
        default:
            return "Main Menu";
    }
}

void menu_handle_input(GameState *game, InputAction action) {
    (void)game;
    (void)action;
    /* TODO: implement menu navigation and panel transitions. */
}
