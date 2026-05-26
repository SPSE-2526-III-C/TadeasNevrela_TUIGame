#ifndef TYPES_H
#define TYPES_H

#include <stdbool.h>
#include <stddef.h>

#define LOG_CAPACITY 64
#define MAX_DOORS 32

typedef enum {
    SCREEN_MAIN_MENU = 0,
    SCREEN_CAMERAS,
    SCREEN_DOORS,
    SCREEN_LOGS,
    SCREEN_SYSTEMS,
    SCREEN_EMERGENCY
} Screen;

typedef enum {
    DOOR_LOCKED = 0,
    DOOR_UNLOCKED,
    DOOR_JAMMED,
    DOOR_UNKNOWN
} DoorState;

typedef struct {
    int id;
    int map_x;
    int map_y;
    DoorState state;
} Door;

typedef struct {
    int x;
    int y;
    float angle;
    float fov;
} Camera;

typedef struct {
    int width;
    int height;
    const int *tiles;
} Map;

typedef struct {
    int integrity;
} IntegrityState;

typedef struct {
    char text[96];
} LogEntry;

typedef struct {
    LogEntry entries[LOG_CAPACITY];
    size_t count;
} LogBuffer;

typedef struct {
    Screen current;
    size_t cursor;
} MenuState;

#endif
