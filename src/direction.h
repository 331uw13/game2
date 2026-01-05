#ifndef DIRECTION_H
#define DIRECTION_H


#include <raymath.h> 


// NV here stands for Normalized Vector.


#define NV_LEFT  ((Vector2){ -1.0f,  0.0f })
#define NV_RIGHT ((Vector2){  1.0f,  0.0f })
#define NV_DOWN  ((Vector2){  0.0f,  1.0f })
#define NV_UP    ((Vector2){  0.0f, -1.0f })


enum direction {
    D_LEFT,
    D_RIGHT,
    D_UP,
    D_DOWN
};




#endif
