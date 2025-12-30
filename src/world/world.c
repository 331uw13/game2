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
           
            load_chunk(&w->chunks[y * chunks_height + x], x, y);

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


bool get_surface(struct world* w, Vector2 from, Vector2* surface) {
    int chunk_x;
    int chunk_y;
    get_chunk_coords(from, w->chunks[0].scale, &chunk_x, &chunk_y);
    
    if((chunk_x < 0 || chunk_y < 0)
    || (chunk_x >= w->num_chunks_x || chunk_y >= w->num_chunks_y)) {
        return false;
    }





    //printf("%i, %i\n", chunk_x, chunk_y);

    return false;
}

