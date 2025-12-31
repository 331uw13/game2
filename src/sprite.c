#include <dirent.h>
#include <stddef.h>
#include <stdio.h>


#define STB_DS_IMPLEMENTATION
#include "thirdparty/stb_ds.h"


#include "sprite.h"
#include "string.h"
#include "errmsg.h"




static
void load_animation(struct sprite* sprite, const char* folder, const char* name) {

    struct string_t path = string_create(0);
    string_append(&path, folder, -1);
    if(string_lastbyte(&path) != '/') {
        string_pushbyte(&path, '/');
    }
    string_append(&path, name, -1);
    
    DIR* dir = opendir(path.bytes);
    struct dirent* ent = NULL;


    int num_textures = 0;
    while((ent = readdir(dir))) {
        if(ent->d_name[0] == '.') {
            continue;
        }

        if(ent->d_type == DT_REG) {
            num_textures++;
            //printf("%s - %s\n", folder, ent->d_name);
        }
    }


    struct animation* anim = malloc(sizeof *anim);
    anim->num_textures = num_textures;
    anim->curr_texture = 0;
    anim->timer = 0.0f;
    anim->timer_interval = 0.2f;

    struct string_t tmp = string_create(0);

    for(int i = 0; i < num_textures; i++) {
        string_clear(&tmp);
        string_append(&tmp, path.bytes, path.size);

        char numbuf[8] = { 0 };
        ssize_t numbuf_len = snprintf(numbuf, sizeof(numbuf)-1, "/%i.png", i);
        string_append(&tmp, numbuf, numbuf_len);
        

        Texture* tex = &anim->textures[i];
        *tex = LoadTexture(tmp.bytes);

        if(tex->width > sprite->width) {
            sprite->width = tex->width;
        }
        if(tex->height > sprite->height) {
            sprite->height = tex->height;
        }
    }


    shput(sprite->animations, strdup(name), anim);

    free_string(&path);
    free_string(&tmp);
    closedir(dir);
}

void load_sprite(struct sprite* sprite, const char* folder) {
    DIR* dir = opendir(folder);
    struct dirent* ent = NULL;

    sprite->flags = 0;
    sprite->curr_anim = NULL;
    sprite->animations = NULL;
    sprite->width = 0;
    sprite->height = 0;
    while((ent = readdir(dir))) {
        if(ent->d_name[0] == '.') {
            continue;
        }

        if(ent->d_type == DT_DIR) {
            load_animation(sprite, folder, ent->d_name);
        }
        //printf("%s - %s\n", folder, ent->d_name);
    }

    closedir(dir);
}

void free_sprite(struct sprite* sprite) {
    size_t map_len = hmlen(sprite->animations);
    for(size_t i = 0; i < map_len; i++) {
        struct animation* anim = (struct animation*)sprite->animations[i].value;
        for(uint16_t n = 0; n < anim->num_textures; n++) {
            UnloadTexture(anim->textures[n]);
        }
        free(anim);
        free(sprite->animations[i].key);
    }
}

void sprite_set_anim(struct sprite* sprite, const char* name) {
    ptrdiff_t index = shgeti(sprite->animations, name);
    if(index < 0) {
        errmsg("There is no animation named '%s'", name);
        return;
    }

    sprite->curr_anim = sprite->animations[index].value;
}

void sprite_update_anim(struct sprite* sprite, float frametime) {
    if(sprite->curr_anim == NULL) {
        return;
    }

    sprite->curr_anim->timer += frametime;
    if(sprite->curr_anim->timer > sprite->curr_anim->timer_interval) {
        sprite->curr_anim->timer = 0.0f;

        sprite->curr_anim->curr_texture++;
        sprite->curr_anim->curr_texture %= sprite->curr_anim->num_textures;
    }
}

void render_sprite(struct sprite* sprite, Vector2 pos) {
    if(sprite->curr_anim == NULL) {
        return;
    }

    Texture* tex = &sprite->curr_anim->textures[ sprite->curr_anim->curr_texture ];
    //float scale = 1.0f;
    float rotation = 0.0f;

    pos.x = -pos.x;
    pos.y = -pos.y;


    pos.x += sprite->width / 2;
    pos.y += sprite->height / 2;

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

