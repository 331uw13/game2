#ifndef COMMON_H
#define COMMON_H

#include <raylib.h>


#define UP_VECTOR ((Vector3){ 0.0f, 1.0f, 0.0f })
#define CLAMP(v, min, max) ((v < min) ? min : (v > max) ? max : v)
#define TARGET_FPS 300




#endif
