#include <math.h>
#include <stdlib.h>
#include <stdio.h>

#include "entity_move_mod_funcs.h"
#include "../state.h"
#include "../common.h"
#include "../errmsg.h"


// This modifier will update enemy 'can_see_player' variable


void ENTMOVMOD_enemy_vision(struct gstate* gst, struct entity* entity) {
    
    struct enemy* enemy = &entity->enemy;

    // TODO: Dont hardcode values here because different enemies can have different abilities...

    Vector2 dir_to_player = Vector2Normalize(Vector2Subtract(gst->player.entity.pos, entity->pos));
    float dist_to_player  = Vector2Distance(entity->pos, gst->player.entity.pos);

    Vector2 ray_pos = (Vector2){ 0, 0 };
    const int ray_max_len = dist_to_player / WORLD_RAY_STEP_SIZE;
    const float ray_near_bias = 30.0f;


    if(enemy->type == ENEMY_ZOMBIE) {
        enemy->can_see_player = dist_to_player < 600.0f;
        return;
    }

    if(raycast_world(entity->world, entity->pos, dir_to_player, ray_max_len, &ray_pos)) {
        enemy->can_see_player = false;
    }
    else {
        enemy->can_see_player = (Vector2Distance(ray_pos, gst->player.entity.pos) < ray_near_bias);
    }
}

