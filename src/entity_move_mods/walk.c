#include <math.h>
#include <stdlib.h>
#include <stdio.h>

#include "entity_move_mod_funcs.h"
#include "../state.h"
#include "../common.h"



void ENTMOVMOD_enemy_walking(struct gstate* gst, struct entity* entity) {
   
    struct enemy* enemy = &entity->enemy;


    if(enemy->walk_direction == D_LEFT) {
        entity->vel.x = - (entity->enemy.move_speed * 100) * gst->frametime;
        entity->sprite.flags &= ~SPRITE_FLIP_HORIZONTAL;
    }
    else
    if(enemy->walk_direction = D_RIGHT) {
        entity->vel.x = (entity->enemy.move_speed * 100) * gst->frametime;
        entity->sprite.flags |= SPRITE_FLIP_HORIZONTAL;
    }

    entity->vel.y += 1.0f * (gst->frametime * 50.0f);
    entity->want_pos.x += entity->vel.x * (gst->frametime * 30.0f);    
    entity->want_pos.y += entity->vel.y * (gst->frametime * 30.0f);


    if(entity->spawn_event) {
        enemy->walk_direction = (drand48() < 0.5f) ? D_LEFT : D_RIGHT;
    }
    

    if(enemy->can_see_player) {
        if(entity->pos.x > gst->player.entity.pos.x) {
            enemy->walk_direction = D_LEFT;
        }
        else
        if(entity->pos.x < gst->player.entity.pos.x) {
            enemy->walk_direction = D_RIGHT;
        }
    }


    entity_world_collision_adjust(entity, gst->frametime);

}

