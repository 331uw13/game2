#include <stdlib.h>
#include <string.h>
#include <raylib.h>
#include <dirent.h>
#include <stdio.h>
#include <rlgl.h>
#include <time.h>

#include "thirdparty/glad.h"


#include "state.h"
#include "memory.h"
#include "errmsg.h"
#include "string.h"
#include "common.h"
#include "shader_util.h"
#include "bloom.h"
#include "perlin_noise.h"

#define RESOLUTION_DIV  2.0

static
void load_item_textures(struct gstate* gst) {
   
    // This will not be the final item loading "system"
    // just for testing...

    gst->item_textures[ITEM_WAND] = LoadTexture("./textures/wand.png");
    gst->item_textures[ITEM_FIREBEND] = LoadTexture("./textures/firebend.png");
    gst->item_textures[ITEM_PLASMABEND] = LoadTexture("./textures/plasmabend.png");
    gst->item_textures[ITEM_PARTICLE_GROWTH] = LoadTexture("./textures/particle_growth.png");
    gst->item_textures[ITEM_MIRROR_PARTICLE] = LoadTexture("./textures/mirror_particle.png");
    gst->item_textures[ITEM_GRAVITYBEND] = LoadTexture("./textures/gravitybend.png");

    gst->item_descs[ITEM_WAND] = strdup("Many centuries old staff.");
    gst->item_descs[ITEM_FIREBEND] = strdup("Firebend spell modifier.");
    gst->item_descs[ITEM_PLASMABEND] = strdup("Plasmabend spell modifier.");
    gst->item_descs[ITEM_GRAVITYBEND] = strdup("Gravitybend spell modifier.");
    gst->item_descs[ITEM_PARTICLE_GROWTH] = strdup("Particle growth spell modifier.");
    gst->item_descs[ITEM_MIRROR_PARTICLE] = strdup("Mirror particle spell modifier.");

    gst->item_rarities[ITEM_WAND] = MYTHICAL_ITEM;
    
    gst->item_rarities[ITEM_PARTICLE_GROWTH] = COMMON_ITEM;
    gst->item_rarities[ITEM_FIREBEND] = RARE_ITEM;
    gst->item_rarities[ITEM_PLASMABEND] = RARE_ITEM;
    gst->item_rarities[ITEM_GRAVITYBEND] = EPIC_ITEM;
    gst->item_rarities[ITEM_MIRROR_PARTICLE] = COMMON_ITEM;

}

static
void load_animations(struct gstate* gst) {

    load_animation(&gst->animations[ANIM_PLAYER_IDLE], "./animations/player/idle");
    load_animation(&gst->animations[ANIM_PLAYER_WALK], "./animations/player/walk");
    
    load_animation(&gst->animations[ANIM_ENEMY_BAT_FLY], "./animations/enemy_bat/fly");

}

const char* item_rarity_to_str(enum item_rarity rarity) {
    switch(rarity) {
        case COMMON_ITEM: return "Common";
        case RARE_ITEM: return "Rare";
        case EPIC_ITEM: return "Epic";
        case MYTHICAL_ITEM: return "Mythical";
    }
    return "Unknown";
}

static
void spawn_starting_spells(struct gstate* gst, Vector2 pos) {

    Vector2 origin = pos;

    for(int n = 0; n < 3; n++) {
        for(uint32_t i = 0; i < ITEM_TYPES_COUNT; i++) {
            spawn_item(gst->player.world, pos, i);
        
            pos.x += 32;
        }
        pos.y += 32;
        pos.x = origin.x;
    }
}


struct gstate* gstate_init() {
    struct gstate* gst = malloc(sizeof *gst);

    InitWindow(1200, 800, "game2");
   
    ToggleBorderlessWindowed();
    int monitor = GetCurrentMonitor();
    int mon_width = GetMonitorWidth(monitor);
    int mon_height = GetMonitorHeight(monitor);

    Vector2 monitor_pos = GetMonitorPosition(monitor);
    SetWindowPosition(monitor_pos.x, monitor_pos.y);
    SetWindowSize(mon_width, mon_height);

    SetTargetFPS(TARGET_FPS);

    gst->font = LoadFont("./Rainbow100.ttf");
    SetTextureFilter(gst->font.texture, RL_TEXTURE_FILTER_NEAREST);

    gst->screen_width = GetScreenWidth() / RESOLUTION_DIV;
    gst->screen_height = GetScreenHeight() / RESOLUTION_DIV;


    gst->render_target = LoadRenderTexture(gst->screen_width, gst->screen_height);
    gst->bloom_result  = LoadRenderTexture(gst->screen_width, gst->screen_height);

    gst->shaders[SHADER_NONE] = LoadShader(0, 0);
    load_shader(
            "./shaders/postprocess.vs",
            "./shaders/postprocess.fs",
            NO_GEOMETRY_SHADER,
            &gst->shaders[SHADER_POSTPROCESS]);

    load_item_textures(gst);
    load_animations(gst);

    init_bloom(gst->screen_width, gst->screen_height);


    srand(time(NULL));

    load_world(&gst->world, 4, 4);
    
    //create_player(gst, &gst->world, &gst->player, (Vector2){ CHUNK_SIZE * 8, CHUNK_SIZE * 8 });
    create_player(gst, &gst->world, &gst->player, (Vector2){ 0, 0 });



    init_perlin_noise();
    srand48(time(NULL));


       


    // Spawn first spell options.
    // TODO: Add more and balance these better.

    spawn_starting_spells(gst, gst->player.pos);


    return gst;
}

void free_gstate(struct gstate* gst) {

    for(uint32_t i = 0; i < SHADERS_COUNT; i++) {
        free_shader(&gst->shaders[i]);
    }
    for(uint32_t i = 0; i < ITEM_TYPES_COUNT; i++) {
        UnloadTexture(gst->item_textures[i]);
        freeif(gst->item_descs[i]);
    }
    for(uint32_t i = 0; i < ANIMATIONS_COUNT; i++) {
        free_animation(&gst->animations[i]);
    }

    free_player(&gst->player);
    free_world(&gst->world);
    free_bloom();
   
    UnloadFont(gst->font);
    UnloadRenderTexture(gst->render_target);
    CloseWindow();
    
    freeif(gst);
}



static
void render(struct gstate* gst) {
    render_player(gst, &gst->player);
    render_world(gst, &gst->world);
}

static
void draw_tex(RenderTexture t, int w, int h) {
    DrawTexturePro(
                t.texture,
                (Rectangle){
                    0, 0,
                    t.texture.width,
                    -t.texture.height
                },
                (Rectangle){
                    0, 0,
                    w,
                    h
                },
                (Vector2){ 0, 0 }, // Origin
                0.0f, // Rotation
                WHITE);
}

void gstate_rungame(struct gstate* gst) {
    while(!WindowShouldClose()) {
        gst->frametime = GetFrameTime();

        BeginTextureMode(gst->render_target);
        ClearBackground(BLACK);
        BeginMode2D(gst->player.cam);
       


        /*
        if(IsKeyDown(KEY_T)) {
            add_particles(gst, test_psys, 1);
        }
        */
        //update_psystem(gst, test_psys);

        //render_psystem(test_psys);


        update_player(gst, &gst->player);

        gst->world_mouse_pos = get_world_coords(gst, GetMousePosition());
        /*gst->world_mouse_pos.x = mouse.x / RESOLUTION_DIV - gst->screen_width / 2;
        gst->world_mouse_pos.y = mouse.y / RESOLUTION_DIV - gst->screen_height / 2;
        gst->world_mouse_pos.x += gst->player.pos.x;
        gst->world_mouse_pos.y += gst->player.pos.y;
        */
        gst->player.cam.offset.x = gst->screen_width / 2;
        gst->player.cam.offset.y = gst->screen_height / 2;
        gst->player.cam.target.x = gst->player.pos.x;
        gst->player.cam.target.y = gst->player.pos.y;


       
        render(gst);
     
        /*
        // FOR TESTING
        {

            Vector2 player_feet = 
             (Vector2){ gst->player.pos.x + 16, gst->player.pos.y + 33 };



            static Vector2 dir = NV_DOWN;
            if(IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
                dir.x += GetMouseDelta().x;
                dir.y += GetMouseDelta().y;
            }


            Vector2 surface;
            if(get_surface(&gst->world, player_feet, dir, &surface)) {
                DrawCircle(surface.x, surface.y, 2.0f, BLUE);
            
                if(IsKeyDown(KEY_LEFT_ALT)) {
                    gst->player.pos.y = surface.y - 34;
                }
            }


        }
        */
        /*
        // FOR TESTING
        {

            struct chunk_cell* chunk_cell = get_chunk_cell_at(&test_chunk,
                    (Vector2){ gst->player.pos.x + 16, gst->player.pos.y + 33 });

            if(chunk_cell) {
                switch(chunk_cell->id) {
                    case S_ID_SURFACE:
                        DrawLine(
                                chunk_cell->segment.va.x,
                                chunk_cell->segment.va.y,
                                chunk_cell->segment.vb.x,
                                chunk_cell->segment.vb.y,
                                BLUE);
                        printf("S_ID_SURFACE\n");
                        break;

                    case S_ID_AIR:
                        printf("S_ID_AIR\n");
                        break;

                    case S_ID_FULL:
                        printf("S_ID_FULL\n");
                        break;
                }
            }
        }
        */

        EndMode2D();
        EndTextureMode();

        

        render_bloom(gst->render_target, &gst->bloom_result);


        
        BeginDrawing();
        ClearBackground(BLACK);
        
        //draw_tex(gst->bloom_result, screen_width, screen_height);
        BeginShaderMode(gst->shaders[ SHADER_POSTPROCESS ]);

        uniform1f(gst->shaders[ SHADER_POSTPROCESS ], "time", GetTime());

        SetShaderValueTexture(gst->shaders[ SHADER_POSTPROCESS ],
                GetShaderLocation(gst->shaders[ SHADER_POSTPROCESS ], "texture_bloom"),
                gst->bloom_result.texture);

        draw_tex(gst->render_target, 
                gst->screen_width * RESOLUTION_DIV,
                gst->screen_height * RESOLUTION_DIV);

    
        EndShaderMode();
        DrawFPS(5, GetScreenHeight()-25);
        DrawText(TextFormat("X: %i, Y: %i onground: %s | jumps: %i | moving: %s",
                    (int)gst->player.pos.x,
                    (int)gst->player.pos.y,
                    gst->player.onground ? "yes" : "no",
                    gst->player.jump_counter,
                    gst->player.moving ? "yes" : "no"
                    ),
                120,
                GetScreenHeight()-25,
                20, GREEN);
        EndDrawing();
    }
}

uint32_t create_ssbo(int bind_point, size_t size) {
    uint32_t ssbo = 0;
    glGenBuffers(1, &ssbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
    glBufferData(GL_SHADER_STORAGE_BUFFER, size, NULL, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, bind_point, ssbo);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    return ssbo;
}

void free_ssbo(uint32_t ssbo) {
    glDeleteBuffers(1, &ssbo);
}

void draw_text(struct gstate* gst, const char* text, Vector2 pos, Color color) {
    DrawTextEx(gst->font, text, pos, 10, 1.0f, color);
}


Vector2 get_world_coords(struct gstate* gst, Vector2 screen_pos) {

    screen_pos.x /= RESOLUTION_DIV;
    screen_pos.y /= RESOLUTION_DIV;
    screen_pos.x -= gst->screen_width / 2;
    screen_pos.y -= gst->screen_height / 2;
    screen_pos.x += gst->player.pos.x;
    screen_pos.y += gst->player.pos.y;

    return screen_pos;
}
