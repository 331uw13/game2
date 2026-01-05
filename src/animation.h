#ifndef ANIMATION_H
#define ANIMATION_H

#include <stdint.h>
#include <raylib.h>

#define ANIMATION_TEXTURES_MAX 32




enum animations : uint32_t {
    ANIM_PLAYER_IDLE = 0, 
    ANIM_PLAYER_WALK,

    ANIM_ENEMY_BAT_FLY,
    ANIM_ENEMY_ZOMBIE_WALK,


    ANIMATIONS_COUNT
};



struct animation {
    Texture  textures [ANIMATION_TEXTURES_MAX];
    uint32_t num_textures;
};


// load_animation() expects that
// textures are named like this: 0.png, 1.png, 2.png ...
bool load_animation(struct animation* anim, char* directory);
void free_animation(struct animation* anim);



#endif
