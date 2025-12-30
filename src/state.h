#ifndef GAME_STATE_H
#define GAME_STATE_H

#include <stdint.h>
#include <raylib.h>

#include "player.h"
#include "world/world.h"


enum shader_indices : int {
    SHADER_NONE = 0,
    SHADER_POSTPROCESS,

    SHADERS_COUNT
};

struct gstate {
    int flags;

    Shader    shaders   [SHADERS_COUNT];

    RenderTexture2D render_target;
    RenderTexture2D bloom_result;

    int screen_width;
    int screen_height;

    struct world  world;
    struct player player;
};



struct gstate* gstate_init();
void           gstate_rungame (struct gstate* gst);
void           free_gstate    (struct gstate* gst);

uint32_t       create_ssbo(int bind_point, size_t size);
void           free_ssbo(uint32_t ssbo);

#endif
