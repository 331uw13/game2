#ifndef ENEMY_ENTITY_H
#define ENEMY_ENTITY_H

#include "../direction.h"

struct gstate;


enum enemy_type : uint32_t {
    ENEMY_BAT = 0,
    ENEMY_ZOMBIE,
    // ...

    ENEMY_TYPES_COUNT
};


struct enemy {
    enum enemy_type type;

    // Controlled by entity vision modifier './src/entity_mods/vision.c'
    bool can_see_player;
     
    enum direction walk_direction;
    float move_speed;
   
    union {
        struct {
        }
        bat;

        struct {
        }
        zombie;

        // ... More enemies will be added later ...
    };
};




#endif
