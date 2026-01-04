#ifndef GAME_STATE_H
#define GAME_STATE_H

#include <stdint.h>
#include <raylib.h>

#include "animation.h"
#include "player.h"
#include "world/world.h"
#include "psystem.h"


enum shaders : uint32_t {
    SHADER_NONE = 0,
    SHADER_POSTPROCESS,

    SHADERS_COUNT
};

struct gstate {
    int flags;
    Font      font;
    Shader    shaders   [SHADERS_COUNT];

    RenderTexture2D render_target;
    RenderTexture2D bloom_result;

    int screen_width;
    int screen_height;
    float frametime;

    Vector2 world_mouse_pos;
    struct world  world;
    struct player player;

    struct animation animations       [ANIMATIONS_COUNT];

    Texture item_textures             [ITEM_TYPES_COUNT];
    char* item_descs                  [ITEM_TYPES_COUNT];
    enum item_rarity item_rarities    [ITEM_TYPES_COUNT];
};



struct gstate* gstate_init();
void           gstate_rungame (struct gstate* gst);
void           free_gstate    (struct gstate* gst);

uint32_t       create_ssbo(int bind_point, size_t size);
void           free_ssbo(uint32_t ssbo);

const char*    item_rarity_to_str(enum item_rarity rarity);
void           draw_text(struct gstate* gst, const char* text, Vector2 pos, Color color);

Vector2 get_world_coords(struct gstate* gst, Vector2 screen_pos);

#endif
