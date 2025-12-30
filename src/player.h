#ifndef PLAYER_H
#define PLAYER_H

#include <raylib.h>
#include <rcamera.h>



struct player {

    Camera2D cam;
    Vector2 pos;


};


void create_player(struct player* pl, Vector2 spawn_pos);
void update_player(struct player* pl, float frametime);






#endif
