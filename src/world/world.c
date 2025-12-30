#include <stdio.h>
#include <stdlib.h>
#include <raymath.h>

#include "world.h"
#include "../memory.h"




void load_world(struct world* w, int chunks_width, int chunks_height) {
   
    w->num_chunks_x = chunks_width;
    w->num_chunks_y = chunks_height;
    w->num_chunks = chunks_height * chunks_width;
    w->chunks = calloc(w->num_chunks, sizeof *w->chunks);

    for(int y = 0; y < chunks_height; y++) {
        for(int x = 0; x < chunks_width; x++) {
           
            load_chunk(&w->chunks[y * chunks_width + x], x, y);

        }
    }
}


void free_world(struct world* w) {
    for(size_t i = 0; i < w->num_chunks; i++) {
        free_chunk(&w->chunks[i]);
    }
    freeif(w->chunks);
}

void render_world(struct world* w) {
    for(size_t i = 0; i < w->num_chunks; i++) {
        render_chunk(&w->chunks[i]);
    } 
}



struct chunk* get_chunk(struct world* w, int col, int row) {
    if(col < 0 || row < 0) {
        return NULL;
    }
    if(col >= w->num_chunks_x || row >= w->num_chunks_y) {
        return NULL;
    }
    return &w->chunks[row * w->num_chunks_x + col];
}

struct chunk_cell* get_world_chunk_cell(struct world* w, int world_col, int world_row) {
    int chunk_x = world_col / CHUNK_SIZE;
    int chunk_y = world_row / CHUNK_SIZE;

    struct chunk* chunk = get_chunk(w, chunk_x, chunk_y);
    if(!chunk) {
        return NULL;
    }

    int local_x = world_col % CHUNK_SIZE;
    int local_y = world_row % CHUNK_SIZE;
    return &chunk->cells[ local_y * CHUNK_SIZE + local_x ];
}

bool get_surface(struct world* w, Vector2 from, Vector2* surface) {

    int col = floor(from.x / w->chunks[0].scale);
    int row = floor(from.y / w->chunks[0].scale);

    struct chunk_cell* cell = NULL;
    
    for(int i = 0; i < 32; i++) {

        cell = get_world_chunk_cell(w, col, row);
        if(!cell) {
            return false;
        }

        if(cell->id == S_ID_SURFACE
        && NORMAL_UP(cell->segment.normal.y)) {
            break;
        }

        row++;
    }

    if(cell->id != S_ID_SURFACE) {    
        return false;
    }



    DrawLine(
            cell->segment.va.x,
            cell->segment.va.y,
            cell->segment.vb.x,
            cell->segment.vb.y,
            (Color){ 0, 255, 0, 255 });




    return false;
}

