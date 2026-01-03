#ifndef ANIMATION_H
#define ANIMATION_H

#include <stdint.h>
#include <raylib.h>

#define ANIM_TEXTURES_MAX 32


struct animation {
    Texture  textures [ANIM_TEXTURES_MAX];
    uint16_t num_textures;
};


bool load_animation(struct animation* anim, const char* path);
void free_animation(struct animation* anim);


#endif
