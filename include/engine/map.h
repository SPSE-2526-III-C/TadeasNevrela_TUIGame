#ifndef MAP_H
#define MAP_H

#include "core/types.h"

Map map_make_default(void);
int map_get_tile(const Map *map, int x, int y);

#endif
