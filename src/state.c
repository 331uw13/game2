#include <stdlib.h>
#include <string.h>
#include <raylib.h>
#include <dirent.h>
#include <stdio.h>
#include <rlgl.h>

#include "thirdparty/glad.h"


#include "state.h"
#include "memory.h"
#include "errmsg.h"
#include "string.h"
#include "common.h"
#include "shader_util.h"
#include "bloom.h"


struct gstate* gstate_init() {
    struct gstate* gst = malloc(sizeof *gst);

    InitWindow(1500, 800, "game2");
    SetTargetFPS(TARGET_FPS);

    gst->screen_width = GetScreenWidth() / 2;
    gst->screen_height = GetScreenHeight() / 2;

    create_player(&gst->player, (Vector2){ 0.0, 0.0 });


    gst->render_target = LoadRenderTexture(gst->screen_width, gst->screen_height);
    gst->bloom_result  = LoadRenderTexture(gst->screen_width, gst->screen_height);


    gst->shaders[SHADER_NONE] = LoadShader(0, 0);
    load_shader(
            "./shaders/postprocess.vs",
            "./shaders/postprocess.fs",
            NO_GEOMETRY_SHADER,
            &gst->shaders[SHADER_POSTPROCESS]);


    init_bloom(gst->screen_width, gst->screen_height);

    return gst;
}

void free_gstate(struct gstate* gst) {

    for(int i = 0; i < SHADERS_COUNT; i++) {
        free_shader(&gst->shaders[i]);
    }

    UnloadRenderTexture(gst->render_target);
    free_bloom();
    CloseWindow();
    freeif(gst);
}



static
void render(struct gstate* gst) {
    DrawCircle(0, 0, 10.0f, ORANGE);
    DrawCircle(10, 10, 5.0f, ORANGE);
    DrawCircle(-10, 30, 5.0f, ORANGE);
    DrawCircle(5, 35, 2.0f, ORANGE);


    DrawRectangle(200, 0, 100, 100, WHITE);
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
        const float frametime = GetFrameTime();

        // Updating...

        update_player(&gst->player, frametime);

        gst->player.cam.offset.x = gst->screen_width / 2;
        gst->player.cam.offset.y = gst->screen_height / 2;
        gst->player.cam.target = gst->player.pos;
       


        // Rendering...

        BeginTextureMode(gst->render_target);
        ClearBackground(BLACK);
        BeginMode2D(gst->player.cam);
       
        render(gst);

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
                gst->screen_width * 2,
                gst->screen_height * 2);

        EndShaderMode();
        DrawFPS(0,0);
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

