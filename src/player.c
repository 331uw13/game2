#include <raymath.h>
#include <stdio.h>

#include "player.h"
#include "common.h"



void create_player(struct player* pl, Vector2 spawn_pos) {
    
    pl->cam = (Camera2D) { 0 };
    pl->cam.target = (Vector2){ spawn_pos.x, spawn_pos.y };

    pl->cam.rotation = 0.0f;
    pl->cam.zoom = 1.0f;
    pl->pos = spawn_pos;

}


static
void update_camera(struct player* pl) {
}

static
void update_movement(struct player* pl, float frametime) {

    float speed = 70;

    if(IsKeyDown(KEY_A)) {
        pl->pos.x -= frametime * speed;
    }
    else
    if(IsKeyDown(KEY_D)) {
        pl->pos.x += frametime * speed;
    }

    if(IsKeyDown(KEY_W)) {
        pl->pos.y -= frametime * speed;
    }
    else
    if(IsKeyDown(KEY_S)) {
        pl->pos.y += frametime * speed;
    }
}


void update_player(struct player* pl, float frametime) {

    update_movement(pl, frametime);
}



