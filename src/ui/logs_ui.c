#include <stdio.h>

#include "ui/logs_ui.h"

void logs_ui_push(GameState *game, const char *text) {
    (void)game;
    (void)text;
    /* TODO: implement ring buffer append for log entries. */
}

void logs_ui_draw(const GameState *game) {
    (void)game;
    puts("[TODO] logs_ui_draw");
}
