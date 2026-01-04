#ifndef WORLD_H
#define WORLD_H


#include <stddef.h>
#include <stdint.h>

#include "chunk.h"
#include "../direction.h"





struct psystem;
struct ps_emitter;


struct world {
    struct chunk* chunks;
    size_t        num_chunks;
    int           num_chunks_x;
    int           num_chunks_y;


    struct psystem*    fire_psystem;
    struct ps_emitter* fire_emitter;
};


struct gstate;

void load_world(struct world* w, int chunks_width, int chunks_height);
void render_world(struct gstate* gst, struct world* w);

void free_world(struct world* w);
bool get_surface(struct world* w, Vector2 from, Vector2 direction, Vector2* surface, Vector2* normal);

struct chunk_cell* get_world_chunk_cell(struct world* w, int world_col, int world_row);
struct chunk_cell* get_world_chunk_cell_v(struct world* w, Vector2 p);

struct chunk_cell* raycast_world
    (struct world* w, Vector2 start, Vector2 direction, int max_len);


bool can_move_up(struct world* w, Vector2 center, float radius, Vector2* hit_normal);
bool can_move_down(struct world* w, Vector2 center, float radius, Vector2* hit_normal);
bool can_move_left(struct world* w, Vector2 center, float radius, Vector2* hit_normal);
bool can_move_right(struct world* w, Vector2 center, float radius, Vector2* hit_normal);

void spawn_item(struct world* w, Vector2 pos, enum item_type type);
void spawn_enemy(struct world* w, Vector2 pos, enum enemy_type type);


#endif
