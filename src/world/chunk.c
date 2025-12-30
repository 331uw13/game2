#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include <raymath.h>

#include "chunk.h"

#define STB_PERLIN_IMPLEMENTATION
#include "../thirdparty/stb_perlin.h"


#include "../memory.h"
#include "../common.h"




static
float perlin_noise(Vector2 p, float freq) {
    return stb_perlin_noise3(
            (p.x * freq) / ((float)CHUNK_SIZE / 10),
            (p.y * freq) / ((float)CHUNK_SIZE / 10),
            1.0f, 0, 0, 0);
}

static
float get_noise(Vector2 p) {
    float n = perlin_noise(p, 0.1);

    return n;
}


const int cases[16][5] = {
    {-1},           // 0
    {3,0,-1},       // 1
    {0,1,-1},       // 2
    {3,1,-1},       // 3
    {1,2,-1},       // 4
    {3,2,0,1,-1},   // 5 (ambiguous)
    {0,2,-1},       // 6
    {3,2,-1},       // 7
    {2,3,-1},       // 8
    {0,2,-1},       // 9
    {1,3,0,2,-1},   // 10 (ambiguous)
    {1,2,-1},       // 11
    {1,3,-1},       // 12
    {0,1,-1},       // 13
    {3,0,-1},       // 14
    {-1}            // 15
};


void load_chunk(struct chunk* chunk, int col, int row) {
    
    chunk->row = row;
    chunk->col = col;

    chunk->cells = calloc(CHUNK_SIZE * CHUNK_SIZE,
        sizeof *chunk->cells);


    const float scale = 10.0f;
    const float isolevel = 0.1f;

    chunk->scale = scale;


    for(int y = 0; y < CHUNK_SIZE; y++) {
        for(int x = 0; x < CHUNK_SIZE; x++) {


            Vector2 p = (Vector2){ // Point world position.
                col * CHUNK_SIZE * scale + x,
                row * CHUNK_SIZE * scale + y
            };

            p = Vector2Scale(p, scale);


            Vector2 points[4] = {
                (Vector2){ p.x,         p.y },
                (Vector2){ p.x + scale, p.y },
                (Vector2){ p.x + scale, p.y + scale },
                (Vector2){ p.x,         p.y + scale }
            };

            float sq[4] = {
                get_noise(points[0]),
                get_noise(points[1]),
                get_noise(points[2]),
                get_noise(points[3])
            };

            int case_index = 0;
            if(sq[0] > isolevel) { case_index |= 1; }
            if(sq[1] > isolevel) { case_index |= 2; }
            if(sq[2] > isolevel) { case_index |= 4; }
            if(sq[3] > isolevel) { case_index |= 8; }

            Vector2 edge_points[4] = {
                Vector2Lerp(points[0], points[1], 0.5f),
                Vector2Lerp(points[1], points[2], 0.5f),
                Vector2Lerp(points[2], points[3], 0.5f),
                Vector2Lerp(points[3], points[0], 0.5f)
            };


            size_t cell_index = y * CHUNK_SIZE + x;

            if(case_index == 0) {
                chunk->cells[cell_index].id = S_ID_AIR;
                continue;
            }
            else
            if(case_index == 15) {
                chunk->cells[cell_index].id = S_ID_FULL;
                continue;
            }


            for(int i = 0; i < 5; i += 2) {
                if(cases[case_index][i] == -1) {
                    break;
                }
                
                Vector2 va = edge_points[cases[case_index][i]];
                Vector2 vb = edge_points[cases[case_index][i+1]];
                
                struct chunk_cell* s = &chunk->cells[y * CHUNK_SIZE + x];
                s->va = va;
                s->vb = vb;
                s->id = S_ID_SURFACE;

                chunk->num_cells++;
            }
        }
    }
}

void free_chunk(struct chunk* chunk) {
    freeif(chunk->cells);
}

#include <stdio.h>
void render_chunk(struct chunk* chunk) {
    
    for(uint32_t i = 0; i < CHUNK_SIZE*CHUNK_SIZE; i++) {
        struct chunk_cell* s = &chunk->cells[i];
        if(s->id != S_ID_SURFACE) { continue; }
        DrawLine(s->va.x, s->va.y,
                 s->vb.x, s->vb.y, GREEN);

    }
}


void get_chunk_local_coords(Vector2 p, struct chunk* chunk, int* col, int* row) {
    *row = (int)floor(p.y / chunk->scale);
    *col = (int)floor(p.x / chunk->scale);
}

void get_chunk_coords(Vector2 p, struct chunk* chunk, int* col, int* row) {
    *row = (int)floor(p.y / (chunk->scale * CHUNK_SIZE));
    *col = (int)floor(p.x / (chunk->scale * CHUNK_SIZE));
}




struct chunk_cell* get_chunk_cell_at(struct chunk* chunk, Vector2 p) {
 
    int pchunk_col;
    int pchunk_row;
    int local_x;
    int local_y; 

    get_chunk_coords(p, chunk, &pchunk_col, &pchunk_row);
    if(pchunk_col != chunk->col
    || pchunk_row != chunk->row) {
        return NULL;
    }
   
    get_chunk_local_coords(p, chunk, &local_x, &local_y);


    return &chunk->cells[ local_y * CHUNK_SIZE + local_x ];
}

