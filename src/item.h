#ifndef ITEM_H
#define ITEM_H

#include <raylib.h>


enum item_type : int {
    ITEM_WAND = 0,
    ITEM_FIREBEND,
    ITEM_PARTICLE_GROWTH,
    ITEM_MIRROR_PARTICLE,
    ITEM_PLASMABEND,
    ITEM_GRAVITYBEND,

    ITEM_TYPES_COUNT,
    ITEM_NONE
};

enum item_rarity : int {
    COMMON_ITEM,
    RARE_ITEM,
    EPIC_ITEM,
    MYTHICAL_ITEM
};



struct item {
    enum item_type type;
    Vector2        pos;
    bool           in_inventory;
};






#endif
