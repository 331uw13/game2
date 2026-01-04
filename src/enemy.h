#ifndef ENEMY_H
#define ENEMY_H

#include "entity.h"


struct gstate;


enum enemy_type {
    ENEMY_BAT,
    // ...

    ENEMY_TYPES_COUNT
};



struct enemy {
    struct entity   entity;
    enum enemy_type type;
    
    /*union {
        struct {
            
        }
        bat;
    };*/

};


void update_enemy_animation(struct gstate* gst, struct enemy* enemy);




#endif
