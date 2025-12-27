#ifndef PLAYER_H
#define PLAYER_H

#include <raylib.h>
#include <rcamera.h>




struct player {


    Camera3D cam;
    Vector3  pos;
    Vector3  vel;
    Vector3  looking_at;
    float    cam_yaw;
    float    cam_pitch;

    float    current_speed;
    float    walk_speed;
    float    run_speed;
};


void create_player(struct player* pl, Vector3 spawn_pos);
void update_player(struct player* pl, float frametime);






#endif
