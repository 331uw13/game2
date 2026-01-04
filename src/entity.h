#ifndef ENTITY_H
#define ENTITY_H


#include <raylib.h>

#include "sprite.h"
#include "world/world.h"



// Entity is just general thing
// for any living creature.


struct entity {

    Vector2          pos;
    Vector2          vel;
    struct world*    world;
    struct sprite    sprite;

    int              health;
    int              max_health;

    float            collision_radius;
};






#endif
