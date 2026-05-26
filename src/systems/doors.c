#include "systems/doors.h"

const char *doors_state_to_string(DoorState state) {
    switch (state) {
        case DOOR_LOCKED:
            return "LOCKED";
        case DOOR_UNLOCKED:
            return "UNLOCKED";
        case DOOR_JAMMED:
            return "JAMMED";
        case DOOR_UNKNOWN:
        default:
            return "UNKNOWN";
    }
}

void doors_init(GameState *game) {
    (void)game;
    /* TODO: initialize door array from map or static config. */
}

void doors_toggle_selected(GameState *game) {
    (void)game;
    /* TODO: toggle selected door with permission and state checks. */
}
