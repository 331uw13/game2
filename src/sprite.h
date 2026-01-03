#ifndef SPRITE_H
#define SPRITE_H

#include <stdint.h>
#include <raylib.h>






/*
struct animation {
    Texture textures [ANIM_TEXTURES_MAX];
    uint16_t num_textures;
    uint16_t curr_texture;
    float timer;
    float timer_interval;
};

struct anim_map {
    char* key;
    struct animation* value;
};

struct sprite {
    struct anim_map*  animations;
    struct animation* curr_anim;
    int flags;

    int width;
    int height;
};

#define SPRITE_FLIP_VERTICAL (1 << 0)
#define SPRITE_FLIP_HORIZONTAL (1 << 1)

void load_sprite(struct sprite* sprite, const char* folder);
void free_sprite(struct sprite* sprite);

void sprite_set_anim(struct sprite* sprite, const char* name);
void sprite_update_anim(struct sprite* sprite, float frametime);
void render_sprite(struct sprite* sprite, Vector2 pos, Color tint);
*/

#endif
