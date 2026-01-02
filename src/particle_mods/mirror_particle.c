#ifndef MIRROR_PARTICLE_MOD_H
#define MIRROR_PARTICLE_MOD_H


#include <stdlib.h>

#include "functions.h"
#include "../perlin_noise.h"



void mirror_particle_update(PARTICLE_MOD_FUNC_ARGS) {
}


void mirror_particle_death(PARTICLE_MOD_FUNC_ARGS) {
}

void mirror_particle_spawn(PARTICLE_MOD_FUNC_ARGS) {
    if(drand48() > 0.5f) {
        part->vel.x = -part->vel.x;
        part->vel.y = -part->vel.y;
    }
}

void PMOD_mirror_particle(PARTICLE_MOD_FUNC_ARGS) {
    switch(context) {
        case PMODCTX_FRAME_UPDATE:
            mirror_particle_update(context, gst, emitter, part);
            break;
        
        case PMODCTX_PARTICLE_SPAWN:
            mirror_particle_spawn(context, gst, emitter, part);
            break;
        
        case PMODCTX_PARTICLE_DEATH:
            mirror_particle_death(context, gst, emitter, part);
            break;
    }
}




#endif
