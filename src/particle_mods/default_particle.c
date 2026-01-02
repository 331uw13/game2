#ifndef DEFAULT_PARTICLE_MOD_H
#define DEFAULT_PARTICLE_MOD_H


#include <stdlib.h>

#include "functions.h"
#include "../perlin_noise.h"



void default_particle_update(PARTICLE_MOD_FUNC_ARGS) {
    part->pos.y += part->vel.y * (gst->frametime * 15);
    part->pos.x += part->vel.x * (gst->frametime * 15);
    part->vel.y += gst->frametime * 7;
}


void default_particle_death(PARTICLE_MOD_FUNC_ARGS) {
}

void default_particle_spawn(PARTICLE_MOD_FUNC_ARGS) { 
    float color_t = drand48();
    part->color = ColorLerp(
            (Color) {
                200, 20, 255, 255
            },
            (Color) {
                100, 180, 200, 255
            }, 
            color_t * color_t);

    part->lifetime += (drand48() * 2.0 + 0.5f);
}

void PMOD_default_particle(PARTICLE_MOD_FUNC_ARGS) {
    switch(context) {
        case PMODCTX_FRAME_UPDATE:
            default_particle_update(context, gst, emitter, part);
            break;
        
        case PMODCTX_PARTICLE_SPAWN:
            default_particle_spawn(context, gst, emitter, part);
            break;
        
        case PMODCTX_PARTICLE_DEATH:
            default_particle_death(context, gst, emitter, part);
            break;
    }
}




#endif
