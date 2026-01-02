#ifndef ITEM_H
#define ITEM_H

#include <raylib.h>


enum item_type : int {
    ITEM_WAND = 0,
    ITEM_FIREBEND,


    ITEM_TYPES_COUNT
};


struct item {
    enum item_type type;
    Vector2        world_pos;
    bool           in_inventory;

};






#endif
