#include <stdlib.h>
#include <string.h>
#include <raylib.h>
#include <dirent.h>
#include <stdio.h>

#include "state.h"
#include "memory.h"
#include "errmsg.h"
#include "string.h"
#include "common.h"
#include "shader_util.h"


struct world_chunk test_chunk = { 0 };


struct gstate* gstate_init() {
    struct gstate* gst = malloc(sizeof *gst);

    InitWindow(800, 600, "game2");
    SetTargetFPS(TARGET_FPS);

    DisableCursor();

    create_player(&gst->player, (Vector3){ 0.2, 0.3, -3.0f});

    generate_chunk(&test_chunk, 0, 0, 0);


    gst->shaders[SHADER_NONE] = LoadShader(0, 0);
    load_shader(
            "./shaders/default.vs",
            "./shaders/default.fs",
            NO_GEOMETRY_SHADER,
            &gst->shaders[SHADER_DEFAULT]);

    gst->materials[MATERIAL_DEFAULT] = LoadMaterialDefault();
    gst->materials[MATERIAL_DEFAULT].shader = gst->shaders[SHADER_DEFAULT];
    gst->materials[MATERIAL_DEFAULT].maps[0].color = GREEN;

    return gst;
}

void free_gstate(struct gstate* gst) {
    free_chunk(&test_chunk);

    for(int i = 0; i < SHADERS_COUNT; i++) {
        free_shader(&gst->shaders[i]);
    }

    CloseWindow();
    freeif(gst);
}


void gstate_rungame(struct gstate* gst) {
    while(!WindowShouldClose()) {
        //UpdateCamera(&gst->player.cam, CAMERA_FIRST_PERSON);
       
        if(IsKeyPressed(KEY_TAB)) {
            if(gst->flags & FLG_DISABLE_PLAYER_CTRL) {
                gst->flags &= ~FLG_DISABLE_PLAYER_CTRL;
                DisableCursor();
            }
            else {
                gst->flags |= FLG_DISABLE_PLAYER_CTRL;
                EnableCursor();
            }
        }

        const float frametime = GetFrameTime();

        BeginDrawing();
        ClearBackground(BLACK);
        BeginMode3D(gst->player.cam);


        if(!(gst->flags & FLG_DISABLE_PLAYER_CTRL)) {
            update_player(&gst->player, frametime);
        }



        uniformv3(gst->shaders[SHADER_DEFAULT], "sun", (Vector3){ 0.3f, -1.0, 0.3f });
        BeginShaderMode(gst->shaders[SHADER_DEFAULT]);

        render_chunk(&test_chunk, gst->materials[MATERIAL_DEFAULT]);

        
        EndShaderMode();
        EndMode3D();

        DrawText(TextFormat("(FPS=%i) (POSITION={ %0.1f, %0.1f, %0.1f })",
                    GetFPS(),
                    gst->player.pos.x,
                    gst->player.pos.y,
                    gst->player.pos.z
                    ), 0, 0, 20, ORANGE);
        EndDrawing();
    }
}


