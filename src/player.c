#include <raymath.h>
#include <stdio.h>

#include "player.h"
#include "common.h"



void create_player(struct player* pl, Vector3 spawn_pos) {
    pl->cam = (Camera3D) { 0 };
    pl->cam.up = UP_VECTOR;
    pl->cam.position = spawn_pos;
    pl->cam.target = (Vector3){ 0.0f, 2.0f, 0.0f };
    pl->cam.projection = CAMERA_PERSPECTIVE;
    pl->cam.fovy = 60.0f;
   
    pl->walk_speed = 3.0f;
    pl->run_speed = 6.0;
    pl->current_speed = pl->walk_speed;
}




static
void update_camera(struct player* pl) {

    float sensetivity = 0.0035f;

    const Vector2 md = GetMouseDelta();
    pl->cam_pitch = (-md.y * sensetivity);
    pl->cam_yaw   = (-md.x * sensetivity);

    pl->looking_at 
        = Vector3Normalize( Vector3Subtract(pl->cam.target, pl->cam.position) );


    CameraYaw   (&pl->cam, pl->cam_yaw, 0);
    CameraPitch (&pl->cam, pl->cam_pitch, 1, 0, 0);
    
}



static
void update_movement(struct player* pl, float frametime) {

    if(IsKeyDown(KEY_LEFT_CONTROL)) {
        pl->current_speed = pl->run_speed;
    }
    else {
        pl->current_speed = pl->walk_speed;
    }


    if(IsKeyDown(KEY_W)) {
        pl->vel.z += pl->current_speed * frametime;
    }
    else
    if(IsKeyDown(KEY_S)) {
        pl->vel.z -= pl->current_speed * frametime;
    }

    if(IsKeyDown(KEY_A)) {
        pl->vel.x -= pl->current_speed * frametime;
    }
    else
    if(IsKeyDown(KEY_D)) {
        pl->vel.x += pl->current_speed * frametime;
    }

    if(IsKeyDown(KEY_SPACE)) {
        pl->vel.y += 0.1 * frametime;
    }
    else
    if(IsKeyDown(KEY_LEFT_SHIFT)) {
        pl->vel.y -= 0.1 * frametime;
    }

    const float vmax = 3.0f;
    pl->vel.x = CLAMP(pl->vel.x, -vmax, vmax);
    pl->vel.y = CLAMP(pl->vel.y, -vmax, vmax);
    pl->vel.z = CLAMP(pl->vel.z, -vmax, vmax);


    CameraMoveForward (&pl->cam, (pl->current_speed * pl->vel.z) * frametime, 1);
    CameraMoveRight   (&pl->cam, (pl->current_speed * pl->vel.x) * frametime, 1);

    pl->cam.position.y += pl->vel.y;

    float friction = pow(1.0f - 0.008f, frametime * TARGET_FPS);
    pl->vel.x *= friction;
    pl->vel.y *= friction;
    pl->vel.z *= friction;


    pl->cam.target.y -= (pl->pos.y - pl->cam.position.y);
    pl->pos = pl->cam.position;
}


void update_player(struct player* pl, float frametime) {

    update_camera(pl);
    update_movement(pl, frametime);

}



