#ifndef ENTITY_H
#define ENTITY_H


#include <raylib.h>
#include <stdint.h>

#include "../sprite.h"
#include "../entity_move_mods/entity_move_mod_funcs.h"


#define ENTITY_MOVE_MODS_MAX 4
#define ENTITY_HITAREAS_MAX 6

#include "enemy.h"



struct gstate;
struct world;


enum entity_type {
    ENTITY_NONE,
    ENTITY_PLAYER,
    ENTITY_ENEMY,
    ENTITY_NPC // (not implemented)
};



struct hitarea {
    float   impact_damage_mult; // Damage to entity gets multiplied with this value if hit.
    float   radius;
    Vector2 offset;
};


struct entity {

    // Always use 'entity.want_pos' if 'entity_world_collision_adjust'
    // is used, it will avoid "noclipping". 
    // You can think of it as "i want to move this thing here but adjust this for me later"

    Vector2          want_pos;
    Vector2          pos;
    Vector2          vel;
    bool             on_ground;

    struct world*    world;
    struct sprite    sprite;

    int              health;
    int              max_health;

    struct hitarea   hitareas [ENTITY_HITAREAS_MAX];
    uint32_t         num_hitareas;

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


struct entity null_entity();

void entity_add_movement_mod(struct entity* entity, entity_move_mod_fn_t* mod);
void entity_update_movement_mods(struct gstate* gst, struct entity* entity);


void entity_add_hitarea(struct entity* entity, struct hitarea area);

// Returns pointer to hitarea if point is inside of it.
struct hitarea* get_entity_hitarea(struct entity* entity, Vector2 point);

void update_entity_animation(struct gstate* gst, struct entity* entity);
void damage_entity(struct entity* entity, int damage);

void entity_render(struct gstate* gst, struct entity* entity);

// 'entity_world_collision_adjust()' Returns 0 when no adjustment was done
// or one or combination of these flags if the entity experiencd adjustment.
// -1 is returned if entity noclipped but it was adjusted back to old position.
#define ENTITY_UP_ADJUSTED    (1 << 0)
#define ENTITY_DOWN_ADJUSTED  (1 << 1)
#define ENTITY_LEFT_ADJUSTED  (1 << 2)
#define ENTITY_RIGHT_ADJUSTED (1 << 3)

// This function will provide world collision,
// but in order it to work  the entity must have at least one hitarea.
int entity_world_collision_adjust(struct entity* entity, float frametime);


#endif
