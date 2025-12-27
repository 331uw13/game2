#ifndef GAME_STATE_H
#define GAME_STATE_H


#include <raylib.h>

#include "player.h"
#include "world/world.h"


#define FLG_DISABLE_PLAYER_CTRL (1 << 0)


enum shader_indices : int {
    SHADER_NONE = 0,
    SHADER_DEFAULT,

    SHADERS_COUNT
};

enum material_indices : int {
    MATERIAL_DEFAULT = 0, 
    MATERIALS_COUNT
};


struct gstate {
    int flags;

    Shader    shaders   [SHADERS_COUNT];
    Material  materials [MATERIALS_COUNT];
   

    struct player player;
    int exit_code;
};



struct gstate* gstate_init();
void           gstate_rungame (struct gstate* gst);
void           free_gstate    (struct gstate* gst);


#endif
