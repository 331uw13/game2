#include <stdio.h>
#include <stdlib.h>
#include <math.h>
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

struct chunk_cell* get_world_chunk_cell_v(struct world* w, Vector2 p) {
    return get_world_chunk_cell(w, 
            (int)floor(p.x / w->chunks[0].scale),
            (int)floor(p.y / w->chunks[0].scale));
}

struct chunk_cell* raycast_world
    (struct world* w, Vector2 start, Vector2 direction, int max_len) {
    
    Vector2 end = Vector2Add(start, Vector2Scale(direction, max_len));

    int start_x = (int)floor(start.x);
    int start_y = (int)floor(start.y);
    int end_x = (int)floor(end.x);
    int end_y = (int)floor(end.y);

    int width = end_x - start_x;
    int height = end_y - start_y;

    int dx0 = 0;
    int dx1 = 0;
    int dy0 = 0;
    int dy1 = 0;
    dx1 = dx0 = (width < 0) ? -1 : 1;
    dy0 = (height < 0) ? -1 : 1;

    int aw = abs(width);
    int ah = abs(height);
    int longest = aw;
    int shortest = ah;

    if(longest < shortest) {
        longest = ah;
        shortest = aw;
        dy1 = (height < 0) ? -1 : 1; 
        dx1 = 0;
    }

    int numerator = longest >> 1;

    int ray_x = start_x;
    int ray_y = start_y;

    for(int i = 0; i < longest; i++) {
        
        struct chunk_cell* cell = get_world_chunk_cell(w,
                ray_x / w->chunks[0].scale, 
                ray_y / w->chunks[0].scale);
        if(cell && (cell->id == S_ID_SURFACE || cell->id == S_ID_FULL)) {
            // Use segment normal and ray direction
            // to know if the ray got stuck at starting position.
            // That may happen because we need to divide 
            // the ray position by chunk scale to get world column and row.
            float d = Vector2DotProduct(cell->segment.normal, direction);
            if(d < 0.0) {
                return cell;
            }
        }
        
        /*DrawCircle(
                ray_x,
                ray_y,
                2.0, PURPLE);*/

        numerator += shortest;
        if(numerator > longest) {
            numerator -= longest;    
            ray_x += dx0 * 4;
            ray_y += dy0 * 4;
        }
        else {
            ray_x += dx1 * 4;
            ray_y += dy1 * 4;
        }
    }
    
    return NULL;
}


static
bool get_segment_intersection(Vector2 p, Vector2 direction, struct segment* s, Vector2* out) {
    direction = Vector2Normalize(direction);
    Vector2 A = Vector2Subtract(s->vb, s->va);
    Vector2 B = Vector2Subtract(s->va, p);

    float r = direction.x * A.y - direction.y * A.x;
    if(fabs(r) < 2.0f) {
        return false; // Direction is near parallel to segment points.
    }

    float dist = (B.x * A.y - B.y * A.x) / r;
    
    *out = Vector2Add(p, Vector2Scale(direction, dist));
    return true;
}


bool get_surface(struct world* w, Vector2 from, Vector2 direction, Vector2* surface, Vector2* normal) {
    int max_raylen = 300;
    struct chunk_cell* cell = raycast_world(w, from, direction, max_raylen);
    if(!cell) {
        return false;
    }


    Vector2 surface_point;
    if(!get_segment_intersection(from, direction, &cell->segment, &surface_point)) {
        return false;
    }
    
    
    *surface = surface_point;
    if(normal) {
        *normal = cell->segment.normal;
    }
    return true;
}

/*
bool can_move_up(struct world* w, Rectangle rect) {
    Vector2 topleft = (Vector2){
        rect.x + 8.0f,
        rect.y - 1.0f
    };

    Vector2 topright = (Vector2){
        rect.x + rect.width - 8.0f,
        rect.y - 1.0f
    };

    struct chunk_cell* cell_l = raycast_world(w, topleft, NV_UP,  2);
    struct chunk_cell* cell_r = raycast_world(w, topright, NV_UP, 2);

    if(cell_l || cell_r) {
        return false;
    }
    return true;
}

bool can_move_down(struct world* w, Rectangle rect) {
    Vector2 bottomleft = (Vector2){
        rect.x,
        rect.y + rect.height 
    };

    Vector2 bottomright = (Vector2){
        rect.x + rect.width,
        rect.y + rect.height
    };

    struct chunk_cell* cell_l = raycast_world(w, bottomleft, NV_DOWN,  1);
    struct chunk_cell* cell_r = raycast_world(w, bottomright, NV_DOWN, 1);

    if(cell_l || cell_r) {
        return false;
    }
    return true;
}


bool can_move_left(struct world* w, Rectangle rect) {
    Vector2 topleft = (Vector2){
        rect.x,
        rect.y
    };

    Vector2 bottomleft = (Vector2){
        rect.x,
        rect.y + rect.height - 8
    };

    struct chunk_cell* cell_t = raycast_world(w, topleft,    NV_LEFT, 1);
    struct chunk_cell* cell_b = raycast_world(w, bottomleft, NV_LEFT, 1);

    if(cell_t) {
        enum cell_slope top_slope = get_cell_slope(cell_t);
        if(top_slope == C_SLOPE_CEILING
        || top_slope == C_SLOPE_CEILING_LEFT
        || top_slope == C_SLOPE_CEILING_RIGHT
        || top_slope == C_SLOPE_VERTICAL) {
            return false;
        }
    }


    if(cell_b) {
        enum cell_slope bottom_slope = get_cell_slope(cell_b);
        if(bottom_slope != C_SLOPE_RIGHT
        && bottom_slope != C_SLOPE_LEFT
        && bottom_slope != C_SLOPE_FLAT) {
            return false;
        }

    }

    return true;
}


bool can_move_right(struct world* w, Rectangle rect) {
    Vector2 topright = (Vector2){
        rect.x + rect.width,
        rect.y
    };

    Vector2 bottomright = (Vector2){
        rect.x + rect.width,
        rect.y + rect.height - 8
    };

    struct chunk_cell* cell_t = raycast_world(w, topright,    NV_RIGHT, 1);
    struct chunk_cell* cell_b = raycast_world(w, bottomright, NV_RIGHT, 1);

    if(cell_t) {
        enum cell_slope top_slope = get_cell_slope(cell_t);
        if(top_slope == C_SLOPE_CEILING
        || top_slope == C_SLOPE_CEILING_LEFT
        || top_slope == C_SLOPE_CEILING_RIGHT
        || top_slope == C_SLOPE_VERTICAL) {
            return false;
        }
    }

    if(cell_b) {
        enum cell_slope bottom_slope = get_cell_slope(cell_b);
        if(bottom_slope != C_SLOPE_LEFT
        && bottom_slope != C_SLOPE_RIGHT
        && bottom_slope != C_SLOPE_FLAT) {
            return false;
        }
    }

    return true;
}*/


bool can_move_up(struct world* w, Vector2 center, float radius) {
    bool can_move_up = true;
    Vector2 ceiling;
    if(get_surface(w, center, NV_UP, &ceiling, NULL)) {
        DrawCircle(ceiling.x, ceiling.y, 3.0f, ORANGE);
        if(Vector2Distance(center, ceiling) < radius) {
            can_move_up = false;
        }
    }
    return can_move_up;
}

bool can_move_right(struct world* w, Vector2 center, float radius) {
    bool can_move_right = true;
    Vector2 right;
    if(get_surface(w, center, NV_RIGHT, &right, NULL)) {
        DrawCircle(right.x, right.y, 3.0f, ORANGE);
        if(Vector2Distance(center, right) < radius) {
            can_move_right = false;
        }
    }
    return can_move_right;
}
 
bool can_move_left(struct world* w, Vector2 center, float radius) {
    bool can_move_left = true;
    Vector2 left;
    if(get_surface(w, center, NV_LEFT, &left, NULL)) {
        DrawCircle(left.x, left.y, 3.0f, ORANGE);
        if(Vector2Distance(center, left) < radius) {
            can_move_left = false;
        }
    }
    return can_move_left;
}
   
bool can_move_down(struct world* w, Vector2 center, float radius) {
    bool can_move_down = true;
    Vector2 bottom;
    if(get_surface(w, center, NV_DOWN, &bottom, NULL)) {
        DrawCircle(bottom.x, bottom.y, 3.0f, ORANGE);
        if(Vector2Distance(center, bottom) < radius) {
            can_move_down = false;
        }
    }
    return can_move_down;
}





