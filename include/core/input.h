#ifndef INPUT_H
#define INPUT_H

typedef enum {
    INPUT_NONE = 0,
    INPUT_UP,
    INPUT_DOWN,
    INPUT_SELECT,
    INPUT_BACK,
    INPUT_QUIT
} InputAction;

InputAction input_read_action(void);

#endif
