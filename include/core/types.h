#ifndef TYPES_H
#define TYPES_H

typedef enum {
    SCREEN_MENU,
    SCREEN_CAMERAS,
    SCREEN_DOORS,
    SCREEN_LOGS,
    SCREEN_SYSTEMS
} Screen;

typedef enum {
    ACTION_NONE,
    ACTION_UP,
    ACTION_DOWN,
    ACTION_SELECT,
    ACTION_BACK,
    ACTION_QUIT
} InputAction;

#endif