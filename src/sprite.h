#ifndef SPRITE_H
#define SPRITE_H

#include <stdint.h>
#include <raylib.h>


#include "animation.h"


#define SPRITE_FLIP_VERTICAL (1 << 0)
#define SPRITE_FLIP_HORIZONTAL (1 << 1)


struct gstate;

struct sprite {
    int flags;
    struct animation* animptr;

    // Max animation texture width and height.
    int max_width;
    int max_height;

    float    anim_timer;
    float    anim_timer_interval;
    uint32_t anim_texture_index;

    float    blink_timer;
    float    blink_speed;
    Color    color_tint;
};


struct sprite null_sprite();
void sprite_set_animation(struct sprite* sprite, struct animation* animptr);

void update_sprite_animation(struct sprite* sprite, float frametime);
void render_sprite(struct gstate* gst, struct sprite* sprite, Vector2 pos);



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
