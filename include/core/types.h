#ifndef TYPES_H
#define TYPES_H

typedef enum {
    SCREEN_MENU,
    SCREEN_CAMERAS,
    SCREEN_DOORS,
    SCREEN_LOGS,
    SCREEN_SYSTEMS,
    SCREEN_EMERGENCY
} Screen;

typedef enum {
    ACTION_NONE,
    ACTION_UP,
    ACTION_DOWN,
    ACTION_SELECT,
    ACTION_BACK,
    ACTION_QUIT
} InputAction;

/* Door states — keep as enum so displayed state can differ from real */
typedef enum {
    DOOR_LOCKED,
    DOOR_UNLOCKED,
    DOOR_JAMMED,
    DOOR_UNKNOWN
} DoorState;

/* Integrity thresholds — horror escalation stages */
typedef enum {
    STAGE_NORMAL      = 100,
    STAGE_SUSPICION   = 75,
    STAGE_CONTRADICTION = 50,
    STAGE_CONTROL_LOSS  = 25,
    STAGE_FAILURE       = 0
} IntegrityThreshold;

#endif