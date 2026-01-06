#include <stdio.h>

#include "entity.h"
#include "../state.h"
#include "../errmsg.h"


struct entity null_entity() {
    return (struct entity) {
        .want_pos = (Vector2) { 0, 0 },
        .pos = (Vector2){ 0, 0 },
        .vel = (Vector2){ 0, 0 },
        .world = NULL,
        .sprite = null_sprite(),
        .health = 100,
        .max_health = 100,
        .num_hitareas = 0,
        .num_movement_mods = 0,
        .parent_chunk_x = 0,
        .parent_chunk_y = 0,
        .type = ENTITY_NONE,
        .spawn_event = false
    };
}

void entity_add_movement_mod(struct entity* entity, entity_move_mod_fn_t* mod) {
    if(entity->num_movement_mods+1 >= ENTITY_MOVE_MODS_MAX) {
        errmsg("Entity has already maximum amount of movement modifiers(%i).",
                ENTITY_MOVE_MODS_MAX);
        return;
    }


    entity->movement_mods[entity->num_movement_mods] = mod;
    entity->num_movement_mods++;
}

void entity_update_movement_mods(struct gstate* gst, struct entity* entity) {
    for(uint32_t i = 0; i < entity->num_movement_mods; i++) {
        entity->movement_mods[i](gst, (void*)entity);
        entity->spawn_event = false;
    }
}


void update_entity_animation(struct gstate* gst, struct entity* entity) {

    if(entity->type != ENTITY_ENEMY) {
        return; // For now, but more entity types will be added.
    }

    switch(entity->enemy.type) {

        case ENEMY_BAT:
            if(entity->sprite.animptr == NULL) {
                sprite_set_animation(&entity->sprite, &gst->animations[ANIM_ENEMY_BAT_FLY]);
            }
            break;

        case ENEMY_ZOMBIE:
            if(entity->sprite.animptr == NULL) {
                sprite_set_animation(&entity->sprite, &gst->animations[ANIM_ENEMY_ZOMBIE_WALK]);
            }
            break;



        default:
            errmsg("Unhandled enemy type %i", entity->enemy.type);
            return;
    }



    update_sprite_animation(&entity->sprite, gst->frametime);
}

void entity_add_hitarea(struct entity* entity, struct hitarea area) {
    if(entity->num_hitareas+1 >= ENTITY_HITAREAS_MAX) {
        errmsg("Entity already has maximum amount of hitareas.");
        return;
    }

    entity->hitareas[entity->num_hitareas] = area;
    entity->num_hitareas++;
}

struct hitarea* get_entity_hitarea(struct entity* entity, Vector2 point) {
    for(uint32_t i = 0; i < entity->num_hitareas; i++) {
        struct hitarea* area = &entity->hitareas[i];
        if(Vector2Distance(point, Vector2Add(entity->pos, area->offset)) < area->radius) {
            return area;
        }
    }
    return NULL;
}


void damage_entity(struct entity* entity, int damage) {
    entity->health -= damage;
    entity->sprite.blink_timer = 0.3f;
    
}

void entity_render(struct gstate* gst, struct entity* entity) {
    render_sprite(gst, &entity->sprite, entity->pos);

    if(gst->flags & STATEFLG_SHOW_ENTITY_HITAREAS) {
        for(uint32_t i = 0; i < entity->num_hitareas; i++) {
            struct hitarea* area = &entity->hitareas[i];
            
            Vector2 area_center = Vector2Add(entity->pos, area->offset);
            DrawCircleLines(area_center.x, area_center.y, area->radius, 
                    area->impact_damage_mult < 0 ? BLUE : RED);
        }
    }
}

int entity_world_collision_adjust(struct entity* entity, float frametime) {
    int adjusted_directions = 0;

    if(entity->num_hitareas == 0) {
        errmsg("Entity requires at least one hitarea for world collision to work.");
        return 0;
    }

    struct hitarea* hitarea = &entity->hitareas[0];


    // Check if there is a surface between
    // current position and wanted position.
    if(raycast_world(
                entity->world,
                entity->pos,
                Vector2Normalize(Vector2Subtract(entity->want_pos, entity->pos)),
                Vector2Distance(entity->pos, entity->want_pos) / WORLD_RAY_STEP_SIZE,
                NULL)) {
        entity->want_pos = entity->pos; // Invalid movement. Do not accept wanted position.
        errmsg("Entity noclipped, but adjusted.");
        return -1;
    }


    // Passing NULL to last argument to ignore hit normal.
    bool allow_move_up    = can_move_up    (entity->world, entity->want_pos, hitarea->radius, NULL);
    bool allow_move_down  = can_move_down  (entity->world, entity->want_pos, hitarea->radius, NULL);
    bool allow_move_left  = can_move_left  (entity->world, entity->want_pos, hitarea->radius, NULL);
    bool allow_move_right = can_move_right (entity->world, entity->want_pos, hitarea->radius, NULL);


    bool want_move_up     = (entity->want_pos.y < entity->pos.y);
    bool want_move_down   = (entity->want_pos.y > entity->pos.y);
    bool want_move_left   = (entity->want_pos.x < entity->pos.x);
    bool want_move_right  = (entity->want_pos.x > entity->pos.x);


    Vector2 surface;
    bool got_surface = get_surface(entity->world, entity->want_pos, NV_DOWN, &surface, NULL);
    

    // Handle up movement.
    if(want_move_up && allow_move_up) {
        entity->pos.y = entity->want_pos.y;
    }
    else
    if(!allow_move_up) {
        entity->vel.y = 0;
        adjusted_directions |= ENTITY_UP_ADJUSTED;
    }

    // Handle down movement.
    if(want_move_down && allow_move_down) {
        entity->pos.y = entity->want_pos.y;
    }
    else
    if(!allow_move_down) {
        adjusted_directions |= ENTITY_DOWN_ADJUSTED;
    }


    // Handle left movement.
    if(want_move_left && allow_move_left) {
        entity->pos.x = entity->want_pos.x;
    }
     
    if(!allow_move_left) {
        entity->vel.x = -entity->vel.x * 0.7;
        entity->want_pos = entity->pos;

        adjusted_directions |= ENTITY_LEFT_ADJUSTED;
    }
    
    // Handle right movement.
    if(want_move_right && allow_move_right) {
        entity->pos.x = entity->want_pos.x;
    }
    
    if(!allow_move_right) {
        entity->vel.x = -entity->vel.x * 0.7;
        entity->want_pos = entity->pos;
        adjusted_directions |= ENTITY_RIGHT_ADJUSTED;
    }


    // Handle 'on_ground'
    // Also we will set the entity position to surface level
    // to avoid "noclipping"
    // And that also allows the entity to move up slopes with just moving left/right.

    if(got_surface) {
        entity->on_ground = (entity->pos.y + hitarea->radius > surface.y - 1.0f);
        if(entity->on_ground && want_move_down) {
            entity->vel.y = 0;
            entity->pos.y = (surface.y - hitarea->radius) - 0.1f;
        }

        if(!entity->on_ground && (!allow_move_left || !allow_move_right)) {
            entity->vel.x = 0;
        }
    }
    else {
        entity->on_ground = false; // Didnt find surface.
    }



    entity->want_pos = entity->pos;
    return adjusted_directions;
}

