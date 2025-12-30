#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include <raymath.h>
#include <float.h>

#include "chunk.h"

#define STB_PERLIN_IMPLEMENTATION
#include "../thirdparty/stb_perlin.h"


#include "../memory.h"
#include "../common.h"


#include <stdio.h>


static
float perlin_noise(Vector2 p, float freq) {
    return stb_perlin_noise3(
            (p.x * freq) / ((float)CHUNK_SIZE / 10),
            (p.y * freq) / ((float)CHUNK_SIZE / 10),
            1.0f, 0, 0, 0);
}

static
float get_noise(Vector2 p) {
    float n = perlin_noise(p, 0.8);

    return n;
}


const int cases[16][5] = {
    { -1 },             // 0
    { 3, 0, -1 },       // 1
    { 0, 1, -1 },       // 2
    { 3, 1, -1 },       // 3
    { 1, 2, -1 },       // 4
    { 3, 2, 0,1,-1 },   // 5
    { 0, 2, -1 },       // 6
    { 3, 2, -1 },       // 7
    { 2, 3, -1 },       // 8
    { 0, 2, -1 },       // 9
    { 1, 3, 0,2,-1 },   // 10
    { 1, 2, -1 },       // 11
    { 1, 3, -1 },       // 12
    { 0, 1, -1 },       // 13
    { 3, 0, -1 },       // 14
    { -1 }              // 15
};

const Vector2 cases_normals[16][2] = {
    {{0,0}, {0,0}}, // 0
    
    {{  0.5,  0.5 }, { 0, 0 }},  // 1
    {{ -0.5,  0.5 }, { 0, 0 }},  // 2
    {{  0.0,  0.5 }, { 0, 0 }},  // 3
    {{ -0.5, -0.5 }, { 0, 0 }},  // 4
    {{ -0.5,  0.5 }, { 0.5, -0.5 }}, // 5
    {{ -0.5,  0.0 }, { 0, 0 }},  // 6
    {{ -0.5,  0.5 }, { 0, 0 }},  // 7
    {{  0.5, -0.5 }, { 0, 0 }},  // 8
    {{  0.5,  0.0 }, { 0, 0 }},  // 9
    {{ -0.5, -0.5 }, { 0.5, 0.5 }}, // 10
    {{  0.5,  0.5 }, { 0, 0 }},  // 11
    {{  0.0, -0.5 }, { 0, 0 }},  // 12
    {{  0.5, -0.5 }, { 0, 0 }},  // 13
    {{ -0.5, -0.5 }, { 0, 0}},   // 14
   
    {{0,0}, {0,0}}  // 15
};


void load_chunk(struct chunk* chunk, int col, int row) {
    
    chunk->row = row;
    chunk->col = col;

    chunk->cells = calloc(CHUNK_SIZE * CHUNK_SIZE,
        sizeof *chunk->cells);


    const float isolevel = 0.1f;

    chunk->scale = 10;


    for(int y = 0; y < CHUNK_SIZE; y++) {
        for(int x = 0; x < CHUNK_SIZE; x++) {


            Vector2 p = (Vector2){ // Point world position.
                col * CHUNK_SIZE + x,
                row * CHUNK_SIZE + y
            };


            Vector2 points[4] = {
                (Vector2){ p.x,     p.y },
                (Vector2){ p.x + 1, p.y },
                (Vector2){ p.x + 1, p.y + 1 },
                (Vector2){ p.x,     p.y + 1 }
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


            int k = 0;
            for(int i = 0; i < 5; i += 2) {
                if(cases[case_index][i] == -1) {
                    break;
                }
                
                struct chunk_cell* cell = &chunk->cells[y * CHUNK_SIZE + x];
                
                cell->segment.va = Vector2Scale(edge_points[cases[case_index][i]], chunk->scale);
                cell->segment.vb = Vector2Scale(edge_points[cases[case_index][i+1]], chunk->scale);
                cell->segment.normal = cases_normals[case_index][k];
                cell->id = S_ID_SURFACE;
                chunk->num_cells++;
                k++;
            }
        }
    }
}

void free_chunk(struct chunk* chunk) {
    freeif(chunk->cells);
}

void render_chunk(struct chunk* chunk) {

    DrawRectangleLines(
                chunk->row * CHUNK_SIZE * chunk->scale,
                chunk->col * CHUNK_SIZE * chunk->scale,
                CHUNK_SIZE * chunk->scale,
                CHUNK_SIZE * chunk->scale,
                BLUE);

    for(uint32_t i = 0; i < CHUNK_SIZE*CHUNK_SIZE; i++) {
        struct chunk_cell* s = &chunk->cells[i];
        if(s->id != S_ID_SURFACE) { continue; }
        DrawLine(s->segment.va.x, s->segment.va.y,
                 s->segment.vb.x, s->segment.vb.y, 
                 GRAY);
                 //NORMAL_UP(s->segment.normal.y) ? GREEN : RED);


        /*
        Vector2 c = Vector2Lerp(s->segment.va, s->segment.vb, 0.5);
        DrawLine(
                c.x,
                c.y,
                c.x + s->segment.normal.x * 8,
                c.y + s->segment.normal.y * 8,
                RED);
                */

    }
}


void get_chunk_local_coords(Vector2 p, struct chunk* chunk, int* col, int* row) {
    *row = (int)floor((p.y - (chunk->row * CHUNK_SIZE * chunk->scale)) / chunk->scale);
    *col = (int)floor((p.x - (chunk->col * CHUNK_SIZE * chunk->scale)) / chunk->scale);
}

void get_chunk_coords(Vector2 p, float chunk_scale, int* col, int* row) {
    *row = (int)floor(p.y / (chunk_scale * CHUNK_SIZE));
    *col = (int)floor(p.x / (chunk_scale * CHUNK_SIZE));
}




struct chunk_cell* get_chunk_cell_at(struct chunk* chunk, Vector2 p) {
 
    int pchunk_col;
    int pchunk_row;
    int local_x;
    int local_y; 

    get_chunk_coords(p, chunk->scale, &pchunk_col, &pchunk_row);
    if(pchunk_col != chunk->col
    || pchunk_row != chunk->row) {
        return NULL;
    }
   
    get_chunk_local_coords(p, chunk, &local_x, &local_y);
    return &chunk->cells[ local_y * CHUNK_SIZE + local_x ];
}

/*
static
float distance_to_surface_cell(struct chunk_cell* cell, Vector2 p) {
    Vector2 left;
    Vector2 right;

    if(cell->segment.va.x < cell->segment.vb.x) {
        left = cell->segment.va;
        right = cell->segment.vb;
    }
    else {
        left = cell->segment.vb;
        right = cell->segment.va;
    }

    if(FloatEquals(right.x, left.x)) {
        return INF_DISTANCE;
    }

    float d = (p.x - left.x) / (right.x - left.x);

    float left_dist = Vector2Distance(p, left);
    float right_dist = Vector2Distance(p, right);
    return Lerp(left_dist, right_dist, d);
}
*/

/*
bool get_surface(struct chunk* chunk, Vector2 from, Vector2* surface) {
    bool found = false;



    return found;
}
*/


