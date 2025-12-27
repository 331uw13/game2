#ifndef WORLD_CHUNK_H
#define WORLD_CHUNK_H



#define CHUNK_SIZE 32
#include <raylib.h>
#include <stdint.h>



struct triangle {
    Vector3 a;
    Vector3 b;
    Vector3 c;
};


struct world_chunk {
    Mesh mesh;
    
    Vector3 world_pos;
    int grid_x;
    int grid_y;
    int grid_z;
};


void generate_chunk(struct world_chunk* chunk, int chunk_x, int chunk_y, int chunk_z);
void render_chunk(struct world_chunk* chunk, Material mat);
void free_chunk(struct world_chunk* chunk);

#endif
