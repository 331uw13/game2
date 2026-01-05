#ifndef ENTITY_H
#define ENTITY_H


#include <raylib.h>
#include <stdint.h>

#include "../sprite.h"
#include "../entity_move_mods/entity_move_mod_funcs.h"


#define ENTITY_MOVE_MODS_MAX 4


#include "enemy.h"



struct gstate;
struct world;


enum entity_type {
    ENTITY_PLAYER,
    ENTITY_ENEMY,
    ENTITY_NPC // (not implemented)
};

struct entity {

    Vector2          pos;
    Vector2          vel;
    struct world*    world;
    struct sprite    sprite;

    int              health;
    int              max_health;

    float            collision_radius;

    entity_move_mod_fn_t* movement_mods [ENTITY_MOVE_MODS_MAX];
    uint32_t              num_movement_mods;

    int      parent_chunk_x;
    int      parent_chunk_y;
    uint32_t chunk_entity_index; // Index to 'chunk->entities[]'

    enum entity_type type;

    // Set to 'true' for first modifier update when entity spawns.
    // So that first entity move modifier can set the initial velocity if needed.
    bool spawn_event;

    union {

        struct enemy enemy;

        //  ... More stuff may be added here later ...
    };
};


void entity_add_movement_mod(struct entity* entity, entity_move_mod_fn_t* mod);
void entity_update_movement_mods(struct gstate* gst, struct entity* entity);


void update_entity_animation(struct gstate* gst, struct entity* entity);
void damage_entity(struct entity* entity, int damage);


#endif
