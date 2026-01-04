#ifndef PHYSICAL_PARTICLE_MOD_H
#define PHYSICAL_PARTICLE_MOD_H


#include <stdlib.h>

#include "particle_mod_funcs.h"
#include "../perlin_noise.h"
#include "../common.h"
#include "../state.h"


void PMOD_physical_particle(PARTICLE_MOD_FUNC_ARGS) {
    if(context != PMODCTX_FRAME_UPDATE) {
        return;
    }


    part->vel.x = CLAMP(part->vel.x, -30.0f, 30.0f);
    part->vel.y = CLAMP(part->vel.y, -30.0f, 30.0f);

    float radius = part->scale;

    Vector2 uphit_normal, downhit_normal, lefthit_normal, righthit_normal;

    bool allow_up   = can_move_up(emitter->psystem->world, part->pos, radius, &uphit_normal);
    bool allow_down = can_move_down(emitter->psystem->world, part->pos, radius, &downhit_normal);
    bool allow_left = can_move_left(emitter->psystem->world, part->pos, radius, &lefthit_normal);
    bool allow_right = can_move_right(emitter->psystem->world, part->pos, radius, &righthit_normal);
   

    Vector2 nvel = Vector2Normalize(part->vel);


    if(!allow_up) {
        part->pos.y += 1.0f;
        part->vel = Vector2Reflect(part->vel, uphit_normal);
    }
    
    if(!allow_down) {
        part->pos.y -= 1.0f;
        part->vel = Vector2Reflect(part->vel, downhit_normal); 
    }

    if(!allow_left) {
        part->pos.x += 1.0f;
        part->vel = Vector2Reflect(part->vel, lefthit_normal);
    }

    if(!allow_right) {
        part->pos.x -= 1.0f;
        part->vel = Vector2Reflect(part->vel, righthit_normal);
    }

    /*
    if(!allow_up || !allow_right || !allow_left || !allow_down) {
        emitter->psystem->world->fire_emitter->cfg.spawn_rect.x = part->pos.x;
        emitter->psystem->world->fire_emitter->cfg.spawn_rect.y = part->pos.y;
        add_particles(gst, emitter->psystem->world->fire_psystem, 1);
    }
    */
}




#endif
