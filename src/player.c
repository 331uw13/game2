#include <raymath.h>
#include <stdio.h>

#include "state.h"
#include "player.h"
#include "common.h"
#include "world/chunk.h"


void create_player(struct gstate* gst, struct world* world, struct player* pl, Vector2 spawn_pos) {
    
    pl->cam = (Camera2D) { 0 };
    pl->cam.target = (Vector2){ spawn_pos.x, spawn_pos.y };

    pl->cam.rotation = 0.0f;
    pl->cam.zoom = 1.0f;
    pl->pos = spawn_pos;
    pl->want_pos = pl->pos;
    pl->moving = false;
    pl->world = NULL;
    pl->jump_counter = 0;
    pl->attack_timer = 0.0f;
    pl->attack_delay = 0.05f;
    pl->onground = false;
    pl->spell_force = 0.0f;
    pl->world = world;
    pl->using_inventory = false;
    pl->pickedup_item = NULL;

    load_sprite(&pl->sprite, "./sprites/player");
    sprite_set_anim(&pl->sprite, "idle");

    pl->spell_psys = new_psystem(world, "player_spell_psystem");
    pl->spell_emitter = add_particle_emitter(pl->spell_psys, 1000, (Rectangle){ 0, 0, 20, 20 });

    pl->inventory = new_inventory(6, 1);
    pl->inventory->pos = (Vector2){ 10,  10 };



    //add_particle_mod(pl->spell_psys, PMOD_fire_particle);
    
    //add_particle_mod(pl->spell_psys, PMOD_default_particle);
    add_particle_mod(pl->spell_psys, PMOD_physical_particle);
}

void free_player(struct player* pl) {
    free_sprite(&pl->sprite);
    free_psystem(pl->spell_psys);
    free_inventory(pl->inventory);
}


static
void stopped_moving(struct player* pl) {
    sprite_set_anim(&pl->sprite, "idle");
}

static
void started_moving(struct player* pl) {
    sprite_set_anim(&pl->sprite, "walk");
}

static
void get_movement_input(struct player* pl, float frametime) {
    float speed = 500;
    pl->want_pos = pl->pos;

    if(IsKeyDown(KEY_LEFT_CONTROL)) {
        speed *= 2;
    }
    else
    if(IsKeyDown(KEY_LEFT_SHIFT)) {
        speed *= 0.5;
    }

    Vector2 old_vel = pl->vel;

    if(IsKeyDown(KEY_A)) {
        pl->vel.x -= frametime * speed;
        pl->sprite.flags |= SPRITE_FLIP_HORIZONTAL;
    }
    else
    if(IsKeyDown(KEY_D)) {
        pl->vel.x += frametime * speed;
        pl->sprite.flags &= ~SPRITE_FLIP_HORIZONTAL;
    }
    
    if(IsKeyPressed(KEY_SPACE)) {
        player_jump(pl);
    }
    

    if(Vector2Distance(old_vel, pl->vel) > 0.1f) {
        if(!pl->moving) {
            started_moving(pl);
        }
        pl->moving = true;
    }
    else {
        if(pl->moving) {
            stopped_moving(pl);
        }
        pl->moving = false;
    }
}

void player_jump(struct player* pl) {
    pl->vel.y = -200.0;
    pl->jump_counter++;
    pl->jumped = true;
}




static
void set_player_onground(struct player* pl) {
    if(pl->got_surface) {
        pl->pos.y = pl->surface.y - pl->sprite.height / 2;
    }
}


static
void update_position(struct player* pl, float frametime) {
    pl->head_pos.x = pl->pos.x + pl->sprite.width / 2;
    pl->head_pos.y = pl->pos.y + 10;
    
    pl->body_pos.x = pl->pos.x + pl->sprite.width / 2;
    pl->body_pos.y = pl->pos.y + pl->sprite.height / 2 + 5;
    
    pl->feet_pos.x = pl->pos.x + pl->sprite.width / 2;
    pl->feet_pos.y = pl->pos.y + pl->sprite.height - 8;




    const float chunk_scale = pl->world->chunks[0].scale;
   


    pl->want_pos.x += pl->vel.x * frametime;
    pl->want_pos.y += pl->vel.y * frametime;

    
    float friction = pow(1.0f - 0.00885f, 500.0f * frametime);
    pl->vel.x *= friction;
   
    // Gravity.
    pl->vel.y += 10.0f * (frametime * 60.0f);


    float radius = 10.0f;
    Vector2 center = (Vector2){
        pl->pos.x,
        pl->pos.y + 6
    };


    //DrawCircleLines(center.x, center.y, radius, RED);


    pl->got_surface = get_surface(pl->world, center, NV_DOWN, &pl->surface, NULL);


    // Do final checking.



    bool allow_move_up     = can_move_up(pl->world, center, radius, NULL);
    bool allow_move_down   = can_move_down(pl->world, center, radius, NULL);
    bool allow_move_left   = can_move_left(pl->world, center, radius, NULL);
    bool allow_move_right  = can_move_right(pl->world, center, radius, NULL);

    bool want_move_left  = (pl->want_pos.x < pl->pos.x);
    bool want_move_right = (pl->want_pos.x > pl->pos.x);
    bool want_move_down  = (pl->want_pos.y > pl->pos.y);
    bool want_move_up    = (pl->want_pos.y < pl->pos.y);




    // Handle Left movement.

    if(want_move_left && allow_move_left) {
        pl->pos.x = pl->want_pos.x;
    }
    else
    if(want_move_left && allow_move_up) {
        center.y = pl->want_pos.y;
        if(can_move_left(pl->world, center, radius, NULL)) {
            pl->pos.x = pl->want_pos.x;
        }
    }


    // Handle Right movement.

    if(want_move_right && allow_move_right) {
        pl->pos.x = pl->want_pos.x;
    }
    else
    if(want_move_right && allow_move_up) {
        center.y = pl->want_pos.y;
        if(can_move_right(pl->world, center, radius, NULL)) {
            pl->pos.x = pl->want_pos.x;
        }
    }
    
 
    if(want_move_up && allow_move_up) {
        pl->pos.y = pl->want_pos.y;
    }

    if(want_move_down && allow_move_down) {
        pl->pos.y = pl->want_pos.y;
    }
    else 
    if(want_move_down) {        
        set_player_onground(pl);
        pl->vel.y = 0;
    }


    if(pl->got_surface) {
        pl->onground = (pl->pos.y + pl->sprite.height/2 > pl->surface.y-1);
    }
    else {
        pl->onground = false;
    }

    if(pl->onground && !pl->jumped) {
        pl->jump_counter = 0;
    }
    
    if(!pl->onground) {
        pl->jumped = false;
    }

}


static
void update_attacking(struct gstate* gst, struct player* pl) {

    if(pl->spell_psys->num_particle_mods == 1) {
        return; // Only "physical_particle" mod is enabled
                // Nothing will happen.
    }

    if(pl->attack_button_down && !pl->casting_spell && !pl->using_inventory) {
        pl->attack_timer = 0.0f;
        pl->casting_spell = true;

        Vector2 mouse = GetMousePosition();
        Vector2 screen = (Vector2) {
            GetScreenWidth(),
            GetScreenHeight()
        };

        pl->attack_side = (mouse.x < screen.x / 2) ? -1 : 1;
  
        pl->spell_direction = Vector2Normalize((Vector2) {
            mouse.x - screen.x / 2,
            mouse.y - screen.y / 2
        });

        pl->spell_emitter->cfg.initial_velocity 
            = Vector2Scale(pl->spell_direction, 7.0f);
        
        pl->spell_emitter->cfg.spawn_rect.x = pl->pos.x + pl->spell_direction.x * 30;
        pl->spell_emitter->cfg.spawn_rect.y = pl->pos.y + pl->spell_direction.y * 30;
        pl->spell_emitter->cfg.spawn_rect.width = 8;
        pl->spell_emitter->cfg.spawn_rect.height = 8;


        pl->vel.x -= pl->spell_direction.x * pl->spell_force;
        pl->vel.y -= pl->spell_direction.y * pl->spell_force;

        add_particles(gst, pl->spell_psys, 10);
    }


    if(pl->casting_spell) {
        pl->attack_timer += gst->frametime;
        if(pl->attack_timer >= pl->attack_delay) {
            pl->casting_spell = false;
        }
    }
}


static
void player_added_item(struct gstate* gst, struct player* pl, enum item_type item_type) {

    switch(item_type) {
        case ITEM_FIREBEND:
            add_particle_mod(pl->spell_psys, PMOD_fire_particle);
            pl->spell_force += 1.5f;
            break;
    
        case ITEM_MIRROR_PARTICLE:
            add_particle_mod(pl->spell_psys, PMOD_mirror_particle);
            break;
    

        case ITEM_PARTICLE_GROWTH:
            pl->spell_emitter->cfg.initial_scale += 2.0f;
            pl->spell_force += 6.0f;
            break;
    }

}


static
void update_inventory_control(struct gstate* gst, struct player* pl) {
    int inv_column;
    int inv_row;
    
    pl->using_inventory = 
        get_mouse_on_inventory(gst, pl->inventory, &inv_column, &inv_row);

    if(!pl->using_inventory && pl->attack_button_pressed) {
        pl->pickedup_item = NULL;
    }
    else
    if(pl->using_inventory && pl->attack_button_pressed) {    
        enum item_type* storage_slot = get_inventory_slot(pl->inventory, inv_column, inv_row);
    
        if(!storage_slot) {
            return;
        }

        if(pl->pickedup_item != NULL) {
            if(storage_slot) {
                if(*storage_slot == ITEM_NONE) {
                    player_added_item(gst, pl, pl->pickedup_item->type);
                    *storage_slot = pl->pickedup_item->type;
                }
                
                pl->pickedup_item->type = ITEM_NONE;
                pl->pickedup_item = NULL;
            }
        }
        else {
            // Nothing was selected drop item.
            spawn_item(pl->world, pl->pos, *storage_slot);
            *storage_slot = ITEM_NONE;
        }
    }


}

void update_player(struct gstate* gst, struct player* pl) {
    pl->attack_button_pressed = IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
    pl->attack_button_down    = IsMouseButtonDown(MOUSE_LEFT_BUTTON);

    
    update_inventory_control(gst, pl);
    get_movement_input(pl, gst->frametime);

    update_position(pl, gst->frametime);
    update_attacking(gst, pl);
    sprite_update_anim(&pl->sprite, gst->frametime);

    update_psystem(gst, pl->spell_psys);
    






}


static
void render_pickedup_item(struct gstate* gst, struct player* pl) {
    if(pl->pickedup_item == NULL) {
        return;
    }

    draw_texture(
            gst->item_textures[pl->pickedup_item->type],
            gst->world_mouse_pos,
            (Vector2){ 0, 0 },
            0.0f,
            1.0f,
            WHITE
            );

}

void render_player(struct gstate* gst, struct player* pl) {

    render_inventory(gst, pl->inventory);
    
    render_sprite(&pl->sprite, pl->pos, (Color){ 200, 150, 130, 255 });
    render_psystem(pl->spell_psys);
    render_pickedup_item(gst, pl);

    if(pl->attack_button_down && !pl->using_inventory && pl->spell_psys->num_particle_mods > 1) {

        Vector2 wand_pos = pl->pos;
        Vector2 wand_offset = (Vector2){ 6, 13 };

        float angle = atan2(pl->spell_direction.y, pl->spell_direction.x) * RAD2DEG;

        draw_texture(gst->item_textures[ITEM_WAND],
                wand_pos,
                wand_offset,
                angle + 90.0 + 15.0f,
                1.0f, WHITE);
    }
}

