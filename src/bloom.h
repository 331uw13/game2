#ifndef BLOOM_H
#define BLOOM_H


#include <raylib.h>


void init_bloom(int screen_width, int screen_height);    
void free_bloom();



void render_bloom(RenderTexture2D scene, RenderTexture2D* result_out);





#endif
