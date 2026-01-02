#ifndef INVENTORY_H
#define INVENTORY_H


#include "item.h"


struct inventory {

    enum item_type* storage;
    int rows;
    int columns;
    Vector2 pos;
    float   box_size;
};



struct gstate;


struct inventory* new_inventory(int columns, int rows);
void              free_inventory(struct inventory* inv);

void render_inventory(struct gstate* gst, struct inventory* inv);
enum item_type* get_inventory_slot(struct inventory* inv, int column, int row);

bool get_mouse_on_inventory(struct gstate* gst, struct inventory* inv, int* column, int* row);


#endif
