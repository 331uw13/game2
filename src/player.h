#ifndef PLAYER_H
#define PLAYER_H

#include <raylib.h>
#include <rcamera.h>

#include "sprite.h"


struct player {

    Camera2D cam;
    Vector2 pos;


    struct sprite sprite;
    bool moving;
};


void create_player(struct player* pl, Vector2 spawn_pos);
void update_player(struct player* pl, float frametime);
void render_player(struct player* pl);

void free_player(struct player* pl);




#endif
