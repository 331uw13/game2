#include <stdlib.h>
#include <raylib.h>
#include <math.h>

#include "common.h"


void draw_texture(Texture tex, Vector2 pos, Vector2 center_offset, float rotation, float scale, Color tint) {
    DrawTexturePro(tex, 
            (Rectangle){
                0,
                0,
                tex.width,
                tex.height
            },
            (Rectangle) {
                pos.x,
                pos.y,
                tex.width,
                tex.height
            },
            (Vector2) {
                tex.width / 2 + center_offset.x,
                tex.height / 2 + center_offset.y
            },
            rotation,
            tint);
}
