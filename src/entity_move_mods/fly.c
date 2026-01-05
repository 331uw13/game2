#include <math.h>
#include <stdlib.h>
#include <stdio.h>

#include "entity_move_mod_funcs.h"
#include "../state.h"
#include "../common.h"



void ENTMOVMOD_enemy_flying(struct gstate* gst, struct entity* entity) {
   
    struct enemy* enemy = &entity->enemy;

    bool allow_up    = can_move_up(entity->world, entity->pos, entity->hitareas[0].radius, NULL);
    bool allow_down  = can_move_down(entity->world, entity->pos, entity->hitareas[0].radius, NULL);
    bool allow_left  = can_move_left(entity->world, entity->pos, entity->hitareas[0].radius, NULL);
    bool allow_right = can_move_right(entity->world, entity->pos, entity->hitareas[0].radius, NULL);

    if(!allow_up) {
        entity->pos.y += 1.0f;
    }
    if(!allow_down) {
        entity->pos.y -= 1.0f;
    }
    if(!allow_left) {
        entity->pos.x += 1.0f;
    }
    if(!allow_right) {
        entity->pos.x -= 1.0f;
    }

    if(enemy->can_see_player) {
        Vector2 dir_to_player = Vector2Normalize(Vector2Subtract(entity->pos, gst->player.entity.pos));
        float dist_to_player = Vector2Distance(entity->pos, gst->player.entity.pos);
        dist_to_player = CLAMP(dist_to_player, 0.0f, 100.0f) / 100.0f;

        float v_mult = Lerp(60.0f, 10.0f, dist_to_player);

        if(!allow_up || !allow_down) {
            entity->vel.y = -entity->vel.y;
        }
        if(!allow_right || !allow_left) {
            entity->vel.x = -entity->vel.x;
        }
    
        entity->vel.x -= dir_to_player.x * (gst->frametime * v_mult);
        entity->vel.y -= dir_to_player.y * (gst->frametime * v_mult);

        const float vmax = 5.0f;
        entity->vel.x = CLAMP(entity->vel.x, -vmax, vmax);
        entity->vel.y = CLAMP(entity->vel.y, -vmax, vmax);

    }
    else {
        if(!allow_up || !allow_down || entity->spawn_event) {
            entity->vel.y = (drand48() * 2.0 - 1.0) * 10;
        }
        if(!allow_right || !allow_left || entity->spawn_event) {
            entity->vel.x = (drand48() * 2.0 - 1.0) * 10;
        }
    }
        
    entity->pos.x += entity->vel.x * (gst->frametime * 30.0f);    
    entity->pos.y += entity->vel.y * (gst->frametime * 30.0f);
}

