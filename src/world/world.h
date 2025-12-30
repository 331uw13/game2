#ifndef WORLD_H
#define WORLD_H


#include <stddef.h>

#include "chunk.h"




struct world {

    struct chunk* chunks;
    size_t        num_chunks;

};


void load_world(struct world* w, int chunks_width, int chunks_height);
void free_world(struct world* w);


#endif
