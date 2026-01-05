#include <dirent.h>
#include <stddef.h>
#include <stdio.h>


/*
#define STB_DS_IMPLEMENTATION
#include "thirdparty/stb_ds.h"
*/

#include "sprite.h"
#include "state.h"
//#include "common.h"
//#include "string.h"
//#include "errmsg.h"


#define BLINK_INTERVAL 0.1f


struct sprite null_sprite() {
    return (struct sprite) {
        .flags = 0,
        .color_tint = WHITE,
        .animptr = NULL,
        .max_width = 0,
        .max_height = 0,
        .anim_timer = 0.0f,
        .anim_timer_interval = 0.175f,
        .anim_texture_index = 0,
        .blink_timer = 0.0f,
        .blink_speed = 15.0f
    };
}


void sprite_set_animation(struct sprite* sprite, struct animation* animptr) {
    if(sprite->animptr == animptr) {
        return;
    }

    sprite->max_width = 0;
    sprite->max_height = 0;
    sprite->animptr = animptr;
    sprite->anim_texture_index = 0;
    //sprite->anim_timer = 0.0f;

    if(animptr != NULL) {
        for(uint32_t i = 0; i < animptr->num_textures; i++) {
            Texture* animtex = &animptr->textures[i];

            if(sprite->max_width < animtex->width) {
                sprite->max_width = animtex->width;
            }

            if(sprite->max_height < animtex->height) {
                sprite->max_height = animtex->height;
            }
        }
    }
}


void update_sprite_animation(struct sprite* sprite, float frametime) {
    if(sprite->animptr == NULL) {
        return;
    }

    sprite->anim_timer += frametime;
    if(sprite->anim_timer >= sprite->anim_timer_interval) {
        sprite->anim_timer = 0.0f;
        sprite->anim_texture_index++;
        sprite->anim_texture_index %= sprite->animptr->num_textures;
    }

}

#include <math.h>

void render_sprite(struct gstate* gst, struct sprite* sprite, Vector2 pos) {
    if(sprite->animptr == NULL) {
        return;
    }

    Color color = sprite->color_tint;

    if(sprite->blink_timer > 0.0f) {
        sprite->blink_timer -= gst->frametime;
        int itimer = (int)floor(sprite->blink_timer * sprite->blink_speed) % 3;
        if(itimer < 2) {
            color.r *= 0.06;
            color.g *= 0.06;
            color.b *= 0.06;
        }
    }

    float rotation = 0;    
    Texture* tex = &sprite->animptr->textures[sprite->anim_texture_index];

    pos.x = -pos.x;
    pos.y = -pos.y;
    pos.x += tex->width / 2;
    pos.y += tex->height / 2;

    DrawTexturePro(*tex,
            (Rectangle){
                0, 0,
                (sprite->flags & SPRITE_FLIP_HORIZONTAL) ? -tex->width : tex->width,
                (sprite->flags & SPRITE_FLIP_VERTICAL) ? -tex->height : tex->height
            },
            (Rectangle) {
                0, 0,
                tex->width,
                tex->height
            },
            pos,
            rotation,
            color);
}

