#ifndef PLASMA_PARTICLE_MOD_H
#define PLASMA_PARTICLE_MOD_H


#include <stdlib.h>
#include <stdio.h>

#include "particle_mod_funcs.h"
#include "../perlin_noise.h"
#include "../common.h"
#include "../state.h"


float fract(float x) {
    return x - floor(x);
}

float hash(float x) {
    x = fract(x * 0.3183099f + 0.1f);
    x *= 17.0f;
    return fract(x * (x + 0.33f) * 1372.57125f);
}

Vector2 hashv2(Vector2 p) {
 
    return (Vector2) {
        hash(p.x),
        hash(p.y)
    };

    /*
    float x = Vector2DotProduct(p, (Vector2){ 95.32f, 12.58f });
    float y = Vector2DotProduct(p, (Vector2){ 85.23f, 32.23f });

    const float m = 43758.5453;

    return (Vector2) {
        fract(sin(x) * m),
        fract(sin(y) * m)
    };
    */
}


Vector2 voronoi_closest(Vector2 p, float freq) {
   
    p.x *= freq;
    p.y *= freq;

    Vector2 t = (Vector2) {
        floor(p.x),
        floor(p.y)
    };
    Vector2 f = (Vector2) {
        fract(p.x),
        fract(p.y)
    };


    float result = 9999999.0f;

    Vector2 closest = p;

    int s = 1;
    for(int y = -s; y <= s; y++) {
        for(int x = -s; x <= s; x++) {
        
            Vector2 neighbor = (Vector2) { (float)x, (float)y };
            Vector2 point = hashv2(Vector2Add(t, neighbor));
            Vector2 difference = Vector2Subtract(Vector2Add(neighbor, point), f);
        

            float dist = Vector2LengthSqr(difference);
            if(dist < result) {
                result = dist;
                closest = difference;
            }
        }
    }

    closest.x = pow(closest.x, 3.0f);
    closest.y = pow(closest.y, 3.0f);
    return closest;
}


void plasma_particle_update(PARTICLE_MOD_FUNC_ARGS) {
    float time = GetTime() * 2.0;
  
    

    Vector2 noise_p = (Vector2) {
        part->pos.x + GetTime() * 10,
        part->pos.y + GetTime() * 10
    };

  
    Vector2 closest = part->lazy_update ? (Vector2){0} : voronoi_closest(noise_p, 0.08f);
    Vector2 direction = Vector2Normalize(closest);


    direction.x *= 80;
    direction.y *= 80;

    direction = Vector2Lerp(direction, part->vel, 0.8f);

    part->vel.x += direction.x * gst->frametime;
    part->vel.y += direction.y * gst->frametime;

    part->pos.y += part->vel.y * (gst->frametime * 10);
    part->pos.x += part->vel.x * (gst->frametime * 10);
}


void plasma_particle_death(PARTICLE_MOD_FUNC_ARGS) {
}

void plasma_particle_spawn(PARTICLE_MOD_FUNC_ARGS) { 
    float color_t = drand48();
    Color color = ColorLerp(
            (Color){ 255, 80, 255, 255 },
            (Color){ 100, 255, 255, 255},  color_t * color_t);
    

    part->color = ColorLerp(part->color, color, 0.5f);
    part->lifetime += drand48() * 3.0f + 0.1f;
}

void PMOD_plasma_particle(PARTICLE_MOD_FUNC_ARGS) {
    switch(context) {
        case PMODCTX_FRAME_UPDATE:
            plasma_particle_update(context, gst, emitter, part);
            break;
        
        case PMODCTX_PARTICLE_SPAWN:
            plasma_particle_spawn(context, gst, emitter, part);
            break;
    
        case PMODCTX_PARTICLE_DEATH:
            plasma_particle_death(context, gst, emitter, part);
            break;
    }
}



#endif
