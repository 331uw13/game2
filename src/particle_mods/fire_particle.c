#ifndef FIRE_PARTICLE_MOD_H
#define FIRE_PARTICLE_MOD_H


#include <stdlib.h>

#include "particle_mod_funcs.h"
#include "../perlin_noise.h"
#include "../state.h"




void fire_particle_update(PARTICLE_MOD_FUNC_ARGS) {
    float time = GetTime() * 2.0;
   
    float noise = 0.0f;

    if(!part->lazy_update) {
        noise = 8.0f * perlin_noise_2D(
                part->pos.x * 0.02,
                part->pos.y * 0.02 + time);
    }
    
    float ax = (cos(noise) / M_PI);
    float ay = (sin(noise) / M_PI);
    part->vel.x += ax * (gst->frametime * 15.0f);
    part->vel.y += ay * (gst->frametime * 15.0f);

    part->pos.y += part->vel.y * (gst->frametime * 20);
    part->pos.x += part->vel.x * (gst->frametime * 20);

    //part->vel.y -= gst->frametime * 6.0f;
}


void fire_particle_death(PARTICLE_MOD_FUNC_ARGS) {
}

void fire_particle_spawn(PARTICLE_MOD_FUNC_ARGS) { 
    float color_t = drand48();
    part->color = ColorLerp(RED, ORANGE, color_t * color_t);
    part->lifetime += drand48() * 3.0f + 0.1f;
}

void PMOD_fire_particle(PARTICLE_MOD_FUNC_ARGS) {
    switch(context) {
        case PMODCTX_FRAME_UPDATE:
            fire_particle_update(context, gst, emitter, part);
            break;
        
        case PMODCTX_PARTICLE_SPAWN:
            fire_particle_spawn(context, gst, emitter, part);
            break;
        
        case PMODCTX_PARTICLE_DEATH:
            fire_particle_death(context, gst, emitter, part);
            break;
    }
}




#endif
