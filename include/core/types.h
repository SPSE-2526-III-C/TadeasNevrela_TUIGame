#ifndef TYPES_H
#define TYPES_H

/* High-level screens the terminal can display. */
typedef enum {
    SCREEN_MENU,
    SCREEN_CAMERAS,
    SCREEN_DOORS,
    SCREEN_LOGS,
    SCREEN_SYSTEMS,
    SCREEN_EMERGENCY
} Screen;

/* Normalized input actions produced from raw keyboard input. */
typedef enum {
    ACTION_NONE,
    ACTION_UP,
    ACTION_DOWN,
    ACTION_SELECT,
    ACTION_BACK,
    ACTION_QUIT
} InputAction;

/* Door states keep display logic separate from internal state. */
typedef enum {
    DOOR_LOCKED,
    DOOR_UNLOCKED,
    DOOR_JAMMED,
    DOOR_UNKNOWN
} DoorState;

/* Integrity thresholds mark the different escalation stages. */
typedef enum {
    STAGE_NORMAL         = 100,
    STAGE_SUSPICION      = 75,
    STAGE_CONTRADICTION  = 50,
    STAGE_CONTROL_LOSS   = 25,
    STAGE_FAILURE        = 0
} IntegrityThreshold;

#endif
