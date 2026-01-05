#ifndef COMMON_H
#define COMMON_H

#include <raylib.h>


#define UP_VECTOR ((Vector3){ 0.0f, 1.0f, 0.0f })
#define CLAMP(v, min, max) ((v < min) ? min : (v > max) ? max : v)
#define TARGET_FPS 300
#define ARRAY_LEN(a) (sizeof(a) / sizeof(*a))


void draw_texture(Texture tex, Vector2 pos, Vector2 center_offset, float rotation, float scale, Color tint);

Vector2 random_normal();

float map_fvalue(float t, float src_min, float src_max, float dst_min, float dst_max);


#endif
    
    
