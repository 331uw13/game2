#ifndef WORLD_CHUNK_H
#define WORLD_CHUNK_H


#include <stdint.h>
#include <raylib.h>

#include "../item.h"
#include "../entity/entity.h"


#define CHUNK_SIZE 64

#define NORMAL_DOWN(y) (y >  0.001f)
#define NORMAL_UP(y)   (y < -0.001f)
#define NORMAL_LEFT(x) (x < -0.001f)
#define NORMAL_RIGHT(x)(x >  0.001f)

#define INF_DISTANCE -9999999999.9f


#define CHUNK_ITEMS_MAX 64    // TODO: Items should never get discarded if they cant spawn because of this limit!
#define CHUNK_ENTITIES_MAX 128

struct gstate;


enum chunk_cell_id {
    S_ID_NONE,
    S_ID_AIR,
    S_ID_FULL,
    S_ID_SURFACE
};

enum chunk_cell_type {
    T_ID_DIRT,
    T_ID_GRASS,
    T_ID_FLOWER
};

struct segment {
    Vector2 va;
    Vector2 vb;
    Vector2 normal;
};



struct chunk_cell {
    int world_x;
    int world_y;
    enum chunk_cell_id    id;
    struct segment      segment;
    
    enum chunk_cell_type  type;
};

struct chunk {
    struct world*      world;
    struct chunk_cell* cells;
    uint32_t           num_cells;

    struct item  items [CHUNK_ITEMS_MAX];
    uint32_t     num_items;

    struct entity entities [CHUNK_ENTITIES_MAX];
    uint32_t      num_entities;


    int col; // TODO: Rename.
    int row; // TODO: Rename.

    float scale;
};




struct worldgen_config {
    Vector2 world_size;
    float chunk_scale;
};


void load_chunk(struct worldgen_config* worldgencfg, struct world* world, struct chunk* chunk, int col, int row);
void free_chunk(struct chunk* chunk);
void render_chunk(struct gstate* gst, struct chunk* chunk);
struct chunk_cell* get_chunk_cell_at(struct chunk* chunk, Vector2 p);

void get_chunk_local_coords(Vector2 p, struct chunk* chunk, int* col, int* row);
void get_chunk_coords(Vector2 p, float chunk_scale, int* col, int* row);


void remove_entity(struct chunk* chunk, uint32_t index);

//enum cell_slope get_cell_slope(struct chunk_cell* cell);


#endif
