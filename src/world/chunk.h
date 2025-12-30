#ifndef WORLD_CHUNK_H
#define WORLD_CHUNK_H


#include <stdint.h>
#include <raylib.h>


#define CHUNK_SIZE 64


enum chunk_cell_id {
    S_ID_NONE,
    S_ID_AIR,
    S_ID_FULL,
    S_ID_SURFACE
};


struct chunk_cell {
    enum chunk_cell_id id;

    Vector2 va;
    Vector2 vb;
};

struct chunk {

    struct chunk_cell* cells;
    uint32_t           num_cells;

    int col;
    int row;
    float scale;
};


void load_chunk(struct chunk* chunk, int col, int row);
void free_chunk(struct chunk* chunk);
void render_chunk(struct chunk* chunk);
struct chunk_cell* get_chunk_cell_at(struct chunk* chunk, Vector2 p);

void get_chunk_local_coords(Vector2 p, struct chunk* chunk, int* col, int* row);
void get_chunk_coords(Vector2 p, struct chunk* chunk, int* col, int* row);


#endif
