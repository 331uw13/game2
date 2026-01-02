
#include <raymath.h>

#include "perlin_noise.h"


static int p[512] = { 0 };

static float grad2d(int hash, float x, float y) {
    switch(hash & 7) {
        case 0: return  x + y;
        case 1: return  x;
        case 2: return  x - y;
        case 3: return -y;
        case 4: return -x - y;
        case 5: return -x;
        case 6: return -x + y;
        case 7: return y;
        default: return 0;
    }
}

static float fade(float t) {
    return (t * t * t * (t * (t * 6.0 - 15.0) + 10.0));
}

float perlin_noise_2D(float x, float y) {

    int X = (int)floor(x) & 255;
    int Y = (int)floor(y) & 255;

    x -= floor(x);
    y -= floor(y);

    float u = fade(x);
    float v = fade(y);
    int A = (p[X  ] + Y) & 255;
    int B = (p[X+1] + Y) & 255;

    return Lerp(Lerp(grad2d(p[A],   x, y), 
                           grad2d(p[B  ], x-1, y), v),
                   Lerp(grad2d(p[A+1], x, y-1), 
                           grad2d(p[B+1], x-1, y-1), v), v);

                           
}

void init_perlin_noise() {
    for(int i = 0; i < 256; i++) {
        p[i+256] = p[i] = permutation[i];
    }
}

