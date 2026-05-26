#include "engine/map.h"

Map map_make_default(void) {
    static const int kDefaultMap[] = {1};

    Map map = {
        .width = 1,
        .height = 1,
        .tiles = kDefaultMap
    };

    return map;
}

int map_get_tile(const Map *map, int x, int y) {
    if (x < 0 || y < 0 || x >= map->width || y >= map->height) {
        return -1;
    }
    return map->tiles[(y * map->width) + x];
}
