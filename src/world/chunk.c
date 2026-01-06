#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include <raymath.h>
#include <float.h>
#include <string.h>

#include "chunk.h"

#define STB_PERLIN_IMPLEMENTATION
#include "../thirdparty/stb_perlin.h"


#include "../memory.h"
#include "../common.h"
#include "../state.h"
#include "../errmsg.h"

#include <stdio.h>



static
float perlin_noise(Vector2 p, float freq_x, float freq_y) {
    return stb_perlin_noise3(
            (p.x * freq_x) / 10.0f,
            (p.y * freq_y) / 10.0f,
            100.0f, 0, 0, 0);
}


static
float get_surface_level(struct worldgen_config* worldgencfg, Vector2 p) {
    
    const float level = 50.0f;


    return Lerp(0.0f, 1.0f, CLAMP(p.y / level, 0.0f, 1.0f));
}


static
float get_noise(struct worldgen_config* worldgencfg, Vector2 p, Vector2 world_pos) {
    float n = 0.0f;
    float y = p.y / worldgencfg->world_size.y;

    float freq = Lerp(0.2f, 0.5f, y * y * y);

    n = perlin_noise(p, freq*2.0f, 1.0f);
    n *= get_surface_level(worldgencfg, p);
    return n;
}

static
enum chunk_cell_type get_chunk_cell_type
(
    struct worldgen_config* worldgencfg, 
    struct chunk_cell* cell,
    Vector2 cell_pos
){
    float grass_noise = perlin_noise(cell_pos, 0.5, 0.5f);

    if(grass_noise < 0.0f) {
        return T_ID_GRASS;
    }
        
    return T_ID_DIRT;
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


void load_chunk(struct worldgen_config* worldgencfg, struct world* world, struct chunk* chunk, int col, int row) { 

    chunk->row = row;
    chunk->col = col;
    chunk->num_items = 0;
    chunk->world = world;
    chunk->cells = calloc(CHUNK_SIZE * CHUNK_SIZE,
        sizeof *chunk->cells);


    chunk->entities_link_head = NULL;
    chunk->entities_link_tail = NULL;

    const float isolevel = 0.1f;
    chunk->scale = worldgencfg->chunk_scale;

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
                get_noise(worldgencfg, points[0], p),
                get_noise(worldgencfg, points[1], p),
                get_noise(worldgencfg, points[2], p),
                get_noise(worldgencfg, points[3], p)
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
            
                cell->world_x = p.x * chunk->scale;
                cell->world_y = p.y * chunk->scale;
                cell->segment.va = Vector2Scale(edge_points[cases[case_index][i]], chunk->scale);
                cell->segment.vb = Vector2Scale(edge_points[cases[case_index][i+1]], chunk->scale);
                cell->segment.normal = cases_normals[case_index][k];
                cell->id = S_ID_SURFACE;
                cell->type = get_chunk_cell_type(worldgencfg, cell, p);
                chunk->num_cells++;
                k++;
            }
        }
    }
}

void free_chunk(struct chunk* chunk) {
    freeif(chunk->cells);
}


static
void render_chunk_items(struct gstate* gst, struct chunk* chunk) {
    
    for(uint32_t i = 0; i < chunk->num_items; i++) {
        struct item* item = &chunk->items[i];
        if(item->type == ITEM_NONE) {
            // TODO: Clean "none" items from chunks.
            continue;
        }

        float item_scale = 0.7f;
        Color item_tint = (Color){ 200, 200, 200, 255 };

        Texture* item_tex = &gst->item_textures[item->type];

        const float circle_radius = 13.0f;
        Vector2 circle_center = (Vector2) {
            item->pos.x - item_tex->width / (circle_radius / 2),
            item->pos.y - item_tex->height / (circle_radius / 2)
        };
      
        if(Vector2Distance(circle_center, gst->player.entity.pos) < 40.0f) {
        if(Vector2Distance(circle_center, gst->world_mouse_pos) <= circle_radius) {
            //render_item_info(gst, item);
          
            item_scale = 0.75f;
            item_tint = WHITE;


            draw_text(gst, TextFormat(
                        "%s\n"
                        "(Rarity: %s)\n"
                        "[E: Pickup]",
                        gst->item_descs[item->type],
                        item_rarity_to_str(gst->item_rarities[item->type]))
                    , 
                    (Vector2) {
                        item->pos.x + 10,
                        item->pos.y - 10
                    },
                    (Color){ 150, 120, 100, 100 + sin(GetTime()*10)*20 });

            if(IsKeyPressed(KEY_E)) {
                gst->player.pickedup_item = item;
            }
            
            //DrawCircleLines(circle_center.x, circle_center.y, circle_radius, 
            //        (Color){ 100, 100, 100, 100 });
        }
        }

        draw_texture(*item_tex, 
                (Vector2) {
                    item->pos.x,
                    item->pos.y
                },
                (Vector2){ 0, 0 }, 0.0f, item_scale, item_tint);





        //DrawCircle(gst->world_mouse_pos.x, gst->world_mouse_pos.y, 2.0f, BLUE);
    }
}



static
void render_cell_grass(struct gstate* gst, struct chunk_cell* cell) {

    int blades = 4;
    float interp = 0.0f;
    float interp_increment = 1.0f / (float)blades;

    Vector2 direction = Vector2Scale(cell->segment.normal, 8.0f);

    for(int i = 0; i < blades; i++) {
        Vector2 p = Vector2Lerp(cell->segment.va, cell->segment.vb, interp);

        DrawLine(
                p.x,
                p.y,
                p.x + direction.x,
                p.y + (direction.y - 2.0) * 0.5f,
                (Color){ 30, 130, 30, 255 }
                );
        interp += interp_increment;
    }
}
/*
// Move entity to correct chunk.
static
bool correct_entity_parent_chunk(struct entity* entity, struct chunk* chunk) {

    int entity_chunk_x;
    int entity_chunk_y; 
    get_chunk_coords(entity->pos, chunk->scale, &entity_chunk_x, &entity_chunk_y);
    
    if(entity->parent_chunk_x == entity_chunk_x && entity->parent_chunk_y == entity_chunk_y) {
        return false;
    }

    struct chunk* correct_chunk = get_chunk_cr(chunk->world, entity_chunk_x, entity_chunk_y);
    if(!correct_chunk) {
        return false;
    }

    if(correct_chunk == chunk) {
        errmsg("Trying to move entity into same chunk.");
        return false;
    }

    if(correct_chunk->num_entities+1 >= CHUNK_ENTITIES_MAX) {
        errmsg("Entity moved chunks, but the destination chunk is full. (( FIXME ))");
        remove_entity(chunk, entity->chunk_entity_index);
        return true;
    }


    //printf("Removing entity %i\n", entity->type);

    return true;
}*/


static
bool correct_entity_parent_chunk(struct chunk* chunk, struct entity* entity) {

    int real_parent_chunk_x;
    int real_parent_chunk_y;

    get_chunk_coords(entity->pos, chunk->scale,
            &real_parent_chunk_x,
            &real_parent_chunk_y);

    if(real_parent_chunk_x == chunk->col
    && real_parent_chunk_y == chunk->row) {
        return false;
    }

    struct chunk* correct_chunk = get_chunk(entity->world, entity->pos);
    if(!correct_chunk) {
        errmsg("Entity type %i moved into invalid chunk (%f, %f)",
                entity->type,
                entity->pos.x,
                entity->pos.y);
        chunk_remove_entity(chunk, entity);
        return true;
    }

    chunk_remove_entity(chunk, entity);
    chunk_add_entity(correct_chunk, entity);

    return true;
}

static
void render_chunk_entities(struct gstate* gst, struct chunk* chunk) {
 
    struct entity* entity = chunk->entities_link_head;
    while(entity) {
        struct entity* next_entity = entity->next;

        if(correct_entity_parent_chunk(chunk, entity)) {
            entity = next_entity;
            continue;
        }


        entity_update_movement_mods(gst, entity);
        update_entity_animation(gst, entity);
        entity_render(gst, entity);

        entity = next_entity;
    }
}


static
bool chunk_find_free_entity(struct chunk* chunk, int64_t* index) {
    for(int64_t i = 0; i < CHUNK_ENTITIES_MAX; i++) {
        if(!chunk->entities[i]) {
            *index = i;
            return true;
        }
    }
    return false;
}
/*
static
void print_entity_nodes(struct chunk* chunk) {
    
    printf("===== Print Entity nodes, time:%f\n", GetTime());
    for(uint32_t i = 0; i < CHUNK_ENTITIES_MAX; i++) {
        struct entity* entity = chunk->entities[i];

        if(!entity) {
            printf("\033[90m[%i] Entity: NULL\033[0m\n", i);
            continue;
        }

        printf("[%i] Entity (\033[34m%p\033[0m), Enabled = %s, Link index: %li\n", 
                i, entity, entity->enabled ? "True" : "False", entity->parent_chunk_link_index);
        printf("   `-> Prev = \033[2;35m%p\033[0m\n", entity->prev);
        printf("   `-> Next = \033[2;35m%p\033[0m\n", entity->next);
    }

    printf("\033[90m ''''''''''''''''''''''''' \033[0m\n");
    printf("    Head = %p\n", chunk->entities_link_head);
    printf("    Tail = %p\n", chunk->entities_link_tail);
    printf("\033[90m ''''''''''''''''''''''''' \033[0m\n");

    printf("--------------------------------------------------------\n");
}
 */

bool chunk_remove_entity(struct chunk* chunk, struct entity* entity) {

    if(entity->parent_chunk_x != chunk->col
    || entity->parent_chunk_y != chunk->row) {
        errmsg("Trying to remove entity from chunk which it doesnt belong to.");
        return false;
    }


    // When removing entity
    // we want to actually update the 'entity->next->prev' pointer.
    // and also 'entity->prev->next'
    // So that this 'entity' will be unlinked.

    if(entity->prev) {
        entity->prev->next = entity->next;
        if(!entity->prev->next) {
            chunk->entities_link_tail = entity->prev;
        }
    }
    if(entity->next) {
        entity->next->prev = entity->prev;
        if(!entity->next->prev) {
            chunk->entities_link_head = entity->next;
        }
    }
  

    //printf("\033[31mRemoved entity \033[1m%p / %li\033[0m\n", entity, entity->parent_chunk_link_index);
  
    entity->enabled = false;
    chunk->entities[entity->parent_chunk_link_index] = NULL;

    if(chunk->num_entities > 0) {
        chunk->num_entities--;
    }

    if(chunk->num_entities == 0) {
        chunk->entities_link_head = NULL;
        chunk->entities_link_tail = NULL;
    }

    //print_entity_nodes(chunk);
    return true;
}

static
void entity_link_add_node(struct chunk* chunk, struct entity* entity, int64_t index) {

    for(int64_t i = index+1; i < CHUNK_ENTITIES_MAX; i++) {
        struct entity* next_ent = chunk->entities[i];
        if(next_ent) {
            
            entity->next = next_ent;
            entity->next->prev = entity;
            break;
        }
    }

    for(int64_t i = index-1; i >= 0; i--) {
        struct entity* prev_ent = chunk->entities[i];
        if(prev_ent) {
            
            entity->prev = prev_ent;
            entity->prev->next = entity;
            break;
        }
    }
}

bool chunk_add_entity(struct chunk* chunk, struct entity* entity) {
    int64_t index = 0;
    if(!chunk_find_free_entity(chunk, &index)) {
        errmsg("Cant add entity to chunk because its full. TODO: Resize 'chunk->entities[]'...");
        return false;
    }

    chunk->entities[index] = entity;
    entity->enabled = true;
    entity->prev = NULL;
    entity->next = NULL;

    entity_link_add_node(chunk, entity, index);

    if(entity->prev == NULL) {
        chunk->entities_link_head = entity;
    }
    if(entity->next == NULL) {
        chunk->entities_link_tail = entity;
    }

    entity->parent_chunk_x = chunk->col;
    entity->parent_chunk_y = chunk->row;
    entity->parent_chunk_link_index = index;

    //printf("\033[32m--> Added entity to index \033[1m%i\033[0m\n", index);
    //print_entity_nodes(chunk);

    chunk->num_entities++;
    return true;
}


void render_chunk(struct gstate* gst, struct chunk* chunk) {

    /*
    DrawRectangleLines(
                chunk->col * CHUNK_SIZE * chunk->scale,
                chunk->row * CHUNK_SIZE * chunk->scale,
                CHUNK_SIZE * chunk->scale,
                CHUNK_SIZE * chunk->scale,
                (Color){ 30, 120, 120, 30 });
    */
    /*
    DrawText(TextFormat("Entities: %i", chunk->num_entities),
                chunk->col * CHUNK_SIZE * chunk->scale,
                chunk->row * CHUNK_SIZE * chunk->scale,
                16,
                (Color){ 20, 130, 20, 200 }
            );
    */

    render_chunk_entities(gst, chunk);

    for(uint32_t i = 0; i < CHUNK_SIZE*CHUNK_SIZE; i++) {
        struct chunk_cell* cell = &chunk->cells[i];

        Color color = (Color){ 120, 120, 120, 255 };

        switch(cell->type) {
            case T_ID_GRASS:
                render_cell_grass(gst, cell);
                color = (Color){ 20, 100, 20, 255 };
                break;

            case T_ID_DIRT:
                color = (Color){ 80, 30, 50, 255 };
                break;
        }

        if(cell->id != S_ID_SURFACE) { continue; }
        DrawLine(cell->segment.va.x, cell->segment.va.y,
                 cell->segment.vb.x, cell->segment.vb.y, 
                 color);
  
        color.r *= 0.5f;
        color.g *= 0.5f;
        color.b *= 0.5f;

        DrawLine(cell->segment.va.x, cell->segment.va.y + 1.25,
                 cell->segment.vb.x, cell->segment.vb.y + 1.25, 
                 color);
        /*
        // Visualize normals.
        Vector2 c = Vector2Lerp(cell->segment.va, cell->segment.vb, 0.5);
        DrawLine(
                c.x,
                c.y,
                c.x + cell->segment.normal.x * 8,
                c.y + cell->segment.normal.y * 8,
                RED);
                */
    }

    render_chunk_items(gst, chunk);
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




