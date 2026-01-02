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

#define RESOLUTION_DIV  2


/*
static struct psystem*  test_psys;
static struct ps_emitter*  test_emitter_A;
static struct ps_emitter*  test_emitter_B;
*/

static
void load_item_textures(struct gstate* gst) {
    
    gst->item_textures[ITEM_WAND] = LoadTexture("./textures/wand.png");
    gst->item_textures[ITEM_FIREBEND] = LoadTexture("./textures/firebend.png");


}


struct gstate* gstate_init() {
    struct gstate* gst = malloc(sizeof *gst);

    InitWindow(1200, 800, "game2");
    SetTargetFPS(TARGET_FPS);

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
    init_bloom(gst->screen_width, gst->screen_height);


    load_world(&gst->world, 4, 4);
    create_player(&gst->world, &gst->player, (Vector2){ CHUNK_SIZE * 8, CHUNK_SIZE * 8 });
    //create_player(&gst->player, (Vector2){ 0, 0 });


    srand48(time(NULL));

    // For testing

    /*{

        test_psys = new_psystem("test_psystem");
        test_emitter_A = add_particle_emitter(test_psys, 300, (Rectangle){
                    gst->player.pos.x + 30,
                    gst->player.pos.y - 10,
                    10,
                    10
                });

      
        add_particle_mod(test_psys, PMOD_fire_particle);


    }*/

    init_perlin_noise();


    return gst;
}

void free_gstate(struct gstate* gst) {

    for(int i = 0; i < SHADERS_COUNT; i++) {
        free_shader(&gst->shaders[i]);
    }
    for(int i = 0; i < ITEM_TYPES_COUNT; i++) {
        UnloadTexture(gst->item_textures[i]);
    }
 
    free_player(&gst->player);
    free_world(&gst->world);
    free_bloom();
    
    UnloadRenderTexture(gst->render_target);
    CloseWindow();
    
    freeif(gst);
}



static
void render(struct gstate* gst) {
    /*DrawCircle(0, 0, 10.0f, ORANGE);
    DrawCircle(20, 10, 5.0f, RED);
    DrawCircle(-10, 30, 5.0f, BLUE);
    DrawCircle(8, 35, 8.0f, GREEN);
    DrawRectangle(200, 0, 10, 10, WHITE);
    */


    render_player(gst, &gst->player);
    render_world(&gst->world);
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
        DrawFPS(0,0);
        DrawText(TextFormat("X: %i, Y: %i | onground: %s | jumps: %i",
                    (int)gst->player.pos.x,
                    (int)gst->player.pos.y,
                    gst->player.onground ? "yes" : "no",
                    gst->player.jump_counter
                    ),
                0, 20, 20, GREEN);
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

