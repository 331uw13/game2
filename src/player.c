#include <raymath.h>
#include <stdio.h>

#include "player.h"
#include "common.h"
#include "world/chunk.h"


void create_player(struct player* pl, Vector2 spawn_pos) {
    
    pl->cam = (Camera2D) { 0 };
    pl->cam.target = (Vector2){ spawn_pos.x, spawn_pos.y };

    pl->cam.rotation = 0.0f;
    pl->cam.zoom = 1.0f;
    pl->pos = spawn_pos;
    pl->moving = false;

    load_sprite(&pl->sprite, "./sprites/player");
    sprite_set_anim(&pl->sprite, "idle");
}

void free_player(struct player* pl) {
    free_sprite(&pl->sprite);
}


static
void stopped_moving(struct player* pl) {
    sprite_set_anim(&pl->sprite, "idle");
}

static
void started_moving(struct player* pl) {
    sprite_set_anim(&pl->sprite, "walk");
}

static
void update_movement(struct player* pl, float frametime) {
    float speed = 120;

    Vector2 old_pos = pl->pos;

    if(IsKeyDown(KEY_A)) {
        pl->pos.x -= frametime * speed;
        pl->sprite.flags |= SPRITE_FLIP_HORIZONTAL;
    }
    else
    if(IsKeyDown(KEY_D)) {
        pl->pos.x += frametime * speed;
        pl->sprite.flags &= ~SPRITE_FLIP_HORIZONTAL;
    }
    if(IsKeyDown(KEY_W)) {
        pl->pos.y -= frametime * speed;
    }
    else
    if(IsKeyDown(KEY_S)) {
        pl->pos.y += frametime * speed;
    }

    if(Vector2Distance(old_pos, pl->pos) > 0.1f) {
        if(!pl->moving) {
            started_moving(pl);
        }
        pl->moving = true;
    }
    else {
        if(pl->moving) {
            stopped_moving(pl);
        }
        pl->moving = false;
    }

}



void update_player(struct player* pl, float frametime) {
    update_movement(pl, frametime);

    sprite_update_anim(&pl->sprite, frametime);




}


void render_player(struct player* pl) {
    render_sprite(&pl->sprite, pl->pos);
}


