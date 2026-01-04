#include <dirent.h>
#include <stddef.h>
#include <stdio.h>


/*
#define STB_DS_IMPLEMENTATION
#include "thirdparty/stb_ds.h"
*/

#include "sprite.h"
//#include "common.h"
//#include "string.h"
//#include "errmsg.h"



struct sprite null_sprite() {
    return (struct sprite) {
        .flags = 0,
        .animptr = NULL,
        .max_width = 0,
        .max_height = 0,
        .anim_timer = 0.0f,
        .anim_timer_interval = 0.175f,
        .anim_texture_index = 0
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


void render_sprite(struct sprite* sprite, Vector2 pos) {
    if(sprite->animptr == NULL) {
        return;
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
            WHITE);
}

