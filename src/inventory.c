#include <stdlib.h>
#include <stdio.h>

#include "inventory.h"
#include "memory.h"
#include "state.h"
#include "common.h"


enum item_type* get_inventory_slot(struct inventory* inv, int column, int row) {
    int64_t i = row * inv->columns + column;
    if(i < 0 || i >= (inv->columns * inv->rows))  {
        return NULL;
    }

    return &inv->storage[i];
}

struct inventory* new_inventory(int columns, int rows) {
    struct inventory* inv = malloc(sizeof *inv);

    inv->storage = calloc(columns * rows, sizeof *inv->storage);
    inv->columns = columns;
    inv->rows = rows;
    inv->pos = (Vector2){ 100, 100 };
    inv->box_size = 32;

    for(uint32_t i = 0; i < columns * rows; i++) {
        inv->storage[i] = ITEM_NONE;
    }


    return inv;
}

void free_inventory(struct inventory* inv) {
    freeif(inv->storage);
    freeif(inv);
}

void render_inventory(struct gstate* gst, struct inventory* inv) {
  
    float scale = 1.0f;

    Vector2 pos = inv->pos;
    Vector2 origin = pos;

    Color bg_color = (Color){ 10, 4, 1, 255 };
    Color box_color = (Color){ 4, 2, 1, 255 };
    DrawRectangle(pos.x, pos.y, inv->box_size * inv->columns, inv->box_size * inv->rows, bg_color);


    for(int row = 0; row < inv->rows; row++) {
        for(int col = 0; col < inv->columns; col++) {
            enum item_type itype = *get_inventory_slot(inv, col, row);
            DrawRectangle(pos.x + 2, pos.y + 2, inv->box_size - 4, inv->box_size - 4, box_color);
            
            if(itype == ITEM_NONE) {
            
                pos.x += inv->box_size;
                continue;
            }


            Texture* tex = &gst->item_textures[itype];
            
            draw_texture(*tex, 
                    (Vector2) {
                        pos.x + inv->box_size / 2.0f,
                        pos.y + inv->box_size / 2.0f
                    },
                    (Vector2){ 0, 0 }, 0.0f, 1.0f, WHITE);
            pos.x += inv->box_size;
        }
        pos.x = origin.x;
        pos.y += inv->box_size;
    }
}

bool get_mouse_on_inventory(struct gstate* gst, struct inventory* inv, int* column, int* row) {
    Vector2 inv_pos = get_world_coords(gst, inv->pos);


    if(gst->world_mouse_pos.x < inv_pos.x
    || gst->world_mouse_pos.x > inv_pos.x + inv->columns * inv->box_size
    || gst->world_mouse_pos.y < inv_pos.y
    || gst->world_mouse_pos.y > inv_pos.y + inv->rows * inv->box_size) {
        return false;
    }

    *column = 0;
    *row = 0;

 
    *column = (gst->world_mouse_pos.x - inv_pos.x) / (inv->box_size);
    *row    = (gst->world_mouse_pos.y - inv_pos.y) / (inv->box_size);


    if(*column < 0 || *column >= inv->columns
    || *row < 0 || *row >= inv->rows) {
        *column = 0;
        *row = 0;
        return false;
    }

    return true;
}

