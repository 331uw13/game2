#ifndef WORLD_H
#define WORLD_H


#include <stddef.h>

#include "chunk.h"
#include "../direction.h"



struct world {
    struct chunk* chunks;
    size_t        num_chunks;
    int           num_chunks_x;
    int           num_chunks_y;
};


void load_world(struct world* w, int chunks_width, int chunks_height);
void render_world(struct world* w);

void free_world(struct world* w);
bool get_surface(struct world* w, Vector2 from, Vector2 direction, Vector2* surface, Vector2* normal);

struct chunk_cell* get_world_chunk_cell(struct world* w, int world_col, int world_row);
struct chunk_cell* get_world_chunk_cell_v(struct world* w, Vector2 p);

struct chunk_cell* raycast_world
    (struct world* w, Vector2 start, Vector2 direction, int max_len);

bool can_move_up(struct world* w, Vector2 center, float radius);
bool can_move_down(struct world* w, Vector2 center, float radius);
bool can_move_left(struct world* w, Vector2 center, float radius);
bool can_move_right(struct world* w, Vector2 center, float radius);
/*
bool can_move_up    (struct world* w, Rectangle rect);
bool can_move_down  (struct world* w, Rectangle rect);
bool can_move_left  (struct world* w, Rectangle rect);
bool can_move_right (struct world* w, Rectangle rect);
*/

#endif
