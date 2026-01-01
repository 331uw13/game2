#ifndef PLAYER_H
#define PLAYER_H

#include <raylib.h>
#include <rcamera.h>

#include "sprite.h"
#include "world/world.h"



struct player {
    Camera2D cam;
    Vector2 pos;
    Vector2 want_pos;
    Vector2 old_pos;
    Vector2 vel;
    Vector2 feet_pos;
    Vector2 head_pos;
    Vector2 body_pos;
    
    struct sprite sprite;
    bool moving;
    bool onground;
    int jump_counter;
    bool jumped;

    struct world* world;
};


void create_player(struct player* pl, Vector2 spawn_pos);
void update_player(struct player* pl, float frametime);
void render_player(struct player* pl);
void player_jump(struct player* pl);
void free_player(struct player* pl);


#endif
