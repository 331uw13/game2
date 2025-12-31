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
    pl->want_pos = pl->pos;
    pl->moving = false;
    pl->world = NULL;

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
void get_movement_input(struct player* pl, float frametime) {
    float speed = 300;
    pl->want_pos = pl->pos;

    if(IsKeyDown(KEY_LEFT_CONTROL)) {
        speed *= 2;
    }
    else
    if(IsKeyDown(KEY_LEFT_SHIFT)) {
        speed *= 0.5;
    }

    Vector2 old_vel = pl->vel;

    if(IsKeyDown(KEY_A)) {
        pl->vel.x -= frametime * speed;
        pl->sprite.flags |= SPRITE_FLIP_HORIZONTAL;
    }
    else
    if(IsKeyDown(KEY_D)) {
        pl->vel.x += frametime * speed;
        pl->sprite.flags &= ~SPRITE_FLIP_HORIZONTAL;
    }

    if(IsKeyDown(KEY_S)) {
        pl->vel.y += frametime * speed;
    }
    if(IsKeyDown(KEY_W)) {
        pl->vel.y -= frametime * speed;
    }
    
    if(IsKeyPressed(KEY_SPACE)) {
        player_jump(pl);
    }
    

    if(Vector2Distance(old_vel, pl->vel) > 0.1f) {
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

void player_jump(struct player* pl) {
    pl->vel.y = -200.0;
}



static
void update_position(struct player* pl, float frametime) {
    pl->head_pos.x = pl->pos.x + pl->sprite.width / 2;
    pl->head_pos.y = pl->pos.y + 10;
    
    pl->body_pos.x = pl->pos.x + pl->sprite.width / 2;
    pl->body_pos.y = pl->pos.y + pl->sprite.height / 2 + 5;
    
    pl->feet_pos.x = pl->pos.x + pl->sprite.width / 2;
    pl->feet_pos.y = pl->pos.y + pl->sprite.height - 8;




    const float chunk_scale = pl->world->chunks[0].scale;
   


    pl->want_pos.x += pl->vel.x * frametime;
    pl->want_pos.y += pl->vel.y * frametime;

    
    float friction = pow(1.0f - 0.007f, 500.0f * frametime);
    pl->vel.x *= friction;
    
    pl->vel.y += 10.0f * (frametime * 60.0f);


    float radius = 10.0f;
    Vector2 center = (Vector2){
        pl->pos.x,
        pl->pos.y + 6
    };


    DrawCircleLines(center.x, center.y, radius, RED);


    Vector2 surface;
    bool got_surface = get_surface(pl->world, center, NV_DOWN, &surface, NULL);

    // Do final checking.

    bool allow_move_up     = can_move_up(pl->world, center, radius);
    bool allow_move_down   = can_move_down(pl->world, center, radius);
    bool allow_move_left   = can_move_left(pl->world, center, radius);
    bool allow_move_right  = can_move_right(pl->world, center, radius);

    bool want_move_left  = (pl->want_pos.x < pl->pos.x);
    bool want_move_right = (pl->want_pos.x > pl->pos.x);
    bool want_move_down  = (pl->want_pos.y > pl->pos.y);
    bool want_move_up    = (pl->want_pos.y < pl->pos.y);


    if(want_move_up && allow_move_up) {
        pl->pos.y = pl->want_pos.y;
    }

    if(want_move_down && allow_move_down) {
        pl->pos.y = pl->want_pos.y;
    }
    else 
    if(want_move_down) {
        pl->vel.y = 0;
    }


    if(want_move_left && allow_move_left) {
        pl->pos.x = pl->want_pos.x;
    }
    else
    if(want_move_left && allow_move_up) {
        center.y = pl->want_pos.y;
        if(can_move_left(pl->world, center, radius)) {
            pl->pos.x = pl->want_pos.x;
            if(!allow_move_down) {
                pl->pos.y = surface.y - pl->sprite.height / 2;
            }
        }
    }

    if(want_move_right && allow_move_right) {
        pl->pos.x = pl->want_pos.x;
    }
    else
    if(want_move_right && allow_move_up) {
        center.y = pl->want_pos.y;
        if(can_move_right(pl->world, center, radius)) {
            pl->pos.x = pl->want_pos.x;
            if(!allow_move_down) {
                pl->pos.y = surface.y - pl->sprite.height / 2;
            }
        }
    }



    /*
    if(pl->want_pos.y < pl->pos.y && allow_move_up) {
        pl->pos.y = pl->want_pos.y;
    }

    if(pl->want_pos.y > pl->pos.y && allow_move_down) {
        pl->pos.y = pl->want_pos.y;
    }

    if(pl->want_pos.x < pl->pos.x && allow_move_left) {
        pl->pos.x = pl->want_pos.x;
    }

    if(pl->want_pos.x > pl->pos.x && allow_move_right) {
        pl->pos.x = pl->want_pos.x;
    }
    */





        /*
    Rectangle player_box = (Rectangle) {
        .x = pl->pos.x - pl->sprite.width / 4 + 2,
        .y = pl->pos.y - pl->sprite.height / 4 + 8,
        .width = 16,
        .height = 24
    };

       
    Vector2 surface;
    if(get_surface(pl->world, (Vector2) {
                player_box.x + player_box.width / 2, 
                player_box.y + player_box.height / 2 
                }, 
                NV_DOWN, &surface, NULL)) {
        pl->onground = (surface.y <= player_box.y + player_box.height + 1);
    }
    else {
        // Player cant be on ground
        // because the surface was not found.
        pl->onground = false; 
    }


    // Moving left.
    if(pl->want_pos.x < pl->pos.x) {
        if(can_move_left(pl->world, player_box)) {
            pl->pos.x = pl->want_pos.x;
        }
    }
    
    // Moving right.
    if(pl->want_pos.x > pl->pos.x) {
        if(can_move_right(pl->world, player_box)) {
            pl->pos.x = pl->want_pos.x;
        }
    }
 
    if(pl->onground && pl->vel.y >= 0.0f) {
        pl->want_pos.y = surface.y - pl->sprite.height / 2 - 1;
        pl->vel.y = 0;
    }
    else {
        pl->vel.y += 0.3f;
        pl->vel.y = CLAMP(pl->vel.y, -50.0f, 50.0f);
        pl->want_pos.y += pl->vel.y * (frametime * 8.0f);
    }

    if(!pl->onground && pl->want_pos.y < pl->pos.y) {
        Vector2 ceiling;
        if(can_move_up(pl->world, player_box)) {
            pl->pos.y = pl->want_pos.y;
        }
        else {
            pl->vel.y = 1.0f;
        }
    }
    else {
        // Moving down so surface detection handled that.
        pl->pos.y = pl->want_pos.y;
    }


    if(IsKeyDown(KEY_LEFT_ALT)) {
        pl->pos = pl->want_pos;
        pl->vel.y = 0;
    }
    */
}



void update_player(struct player* pl, float frametime) {
    get_movement_input(pl, frametime);
    update_position(pl, frametime);
    sprite_update_anim(&pl->sprite, frametime);




}


void render_player(struct player* pl) {
    render_sprite(&pl->sprite, pl->pos);
}


