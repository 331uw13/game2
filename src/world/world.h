#ifndef WORLD_H
#define WORLD_H


#include <stddef.h>

#include "chunk.h"




struct world {

    struct chunk* chunks;
    size_t        num_chunks;
    int           num_chunks_x;
    int           num_chunks_y;
};


void load_world(struct world* w, int chunks_width, int chunks_height);
void render_world(struct world* w);

void free_world(struct world* w);
bool get_surface(struct world* w, Vector2 from, Vector2* surface);

struct chunk_cell* get_world_chunk_cell(struct world* w, int world_col, int world_row);


#endif
