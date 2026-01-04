#ifndef GRAVITY_PARTICLE_MOD_H
#define GRAVITY_PARTICLE_MOD_H


#include <stdlib.h>


#include "particle_mod_funcs.h"
#include "../perlin_noise.h"
#include "../common.h"
#include "../state.h"


void gravity_particle_update(PARTICLE_MOD_FUNC_ARGS) {
  

    Vector2 target = gst->world_mouse_pos;

    Vector2 direction = Vector2Normalize(Vector2Subtract(part->pos, target));


    float distance = Vector2Distance(part->pos, target);

    distance = CLAMP(distance, 0.0f, 150.0f);
    direction.x *= distance;
    direction.y *= distance;

    /*
    direction.x *= 80;
    direction.y *= 80;
    */
    direction = Vector2Negate(direction);
   

    part->vel.x += direction.x * (gst->frametime * 1.0f);
    part->vel.y += direction.y * (gst->frametime * 1.0f);

    part->pos.y += part->vel.y * (gst->frametime * 5);
    part->pos.x += part->vel.x * (gst->frametime * 5);

    
    //part->vel.y -= gst->frametime * 6.0f;
}


void gravity_particle_death(PARTICLE_MOD_FUNC_ARGS) {
}

void gravity_particle_spawn(PARTICLE_MOD_FUNC_ARGS) { 
    float color_t = drand48();
    part->color = ColorLerp((Color){ 125, 40, 80, 255 }, PINK, color_t * color_t);
    part->lifetime += drand48() * 10.0f;
}

void PMOD_gravity_particle(PARTICLE_MOD_FUNC_ARGS) {
    switch(context) {
        case PMODCTX_FRAME_UPDATE:
            gravity_particle_update(context, gst, emitter, part);
            break;
        
        case PMODCTX_PARTICLE_SPAWN:
            gravity_particle_spawn(context, gst, emitter, part);
            break;
        
        case PMODCTX_PARTICLE_DEATH:
            gravity_particle_death(context, gst, emitter, part);
            break;
    }
}




#endif
