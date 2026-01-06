#include <raymath.h>
#include <stdio.h>

#include "state.h"
#include "player.h"
#include "common.h"
#include "world/chunk.h"


void create_player(struct gstate* gst, struct world* world, struct player* pl, Vector2 spawn_pos) {
    
    pl->cam = (Camera2D) { 0 };
    pl->cam.target = (Vector2){ spawn_pos.x, spawn_pos.y };
    pl->cam_offset = (Vector2){ 0, 0 };
    pl->from_cam_offset = pl->cam_offset;
    pl->to_cam_offset = pl->cam_offset;
    pl->cam_offset_lerp_timer = 0.0f;

    pl->cam.rotation = 0.0f;
    pl->cam.zoom = 1.0f;
    pl->moving = false;
    pl->entity.world = NULL;
    pl->jump_counter = 0;
    pl->attack_timer = 0.0f;
    pl->attack_delay = 0.02f;
    pl->spell_force = 0.0f;
    pl->cam_offset = (Vector2){ 0, 0 };
    pl->using_inventory = false;
    pl->pickedup_item = NULL;


    pl->entity = null_entity();
    pl->entity.world = world;
    pl->entity.max_health = 100;
    pl->entity.health = pl->entity.max_health;
    pl->entity.type = ENTITY_PLAYER;
    
    pl->entity.pos = spawn_pos;
    pl->entity.want_pos = pl->entity.pos;
    pl->entity.sprite.render_offset.y = -6;

    // Hitarea for world collisions
    entity_add_hitarea(&pl->entity,
            (struct hitarea) {
                .impact_damage_mult = -1,
                .offset = (Vector2) { 0, 0 },
                .radius = 10.0f
            });

    // Hitarea for head.
    entity_add_hitarea(&pl->entity,
            (struct hitarea) {
                .impact_damage_mult = 1.75f,
                .offset = (Vector2) { 0, -14 },
                .radius = 6.0f
            });

    // Hitarea for body and feet
    entity_add_hitarea(&pl->entity,
            (struct hitarea) {
                .impact_damage_mult = 1.75f,
                .offset = (Vector2) { 0, 0 },
                .radius = 6.0f
            });


    pl->max_mana = 30.0f;
    pl->mana_regen = 1.5f;
    pl->mana_regen_delay = 1.0f;
    pl->mana_regen_timer = 0.0f;
    pl->mana_cost_mult = 1.0f;
    pl->mana = pl->max_mana;

    pl->spell_psys = new_psystem(world, "player_spell_psystem");
    pl->spell_emitter = add_particle_emitter(pl->spell_psys, 2000, (Rectangle){ 0, 0, 20, 20 });

    pl->inventory = new_inventory(6, 1);
    pl->inventory->pos = (Vector2){ 10,  10 };


    sprite_set_animation(&pl->entity.sprite, &gst->animations[ANIM_PLAYER_IDLE]);

    add_particle_mod(pl->spell_psys, PMOD_fire_particle);
    add_particle_mod(pl->spell_psys, PMOD_fire_particle);
    
    //add_particle_mod(pl->spell_psys, PMOD_default_particle);
    add_particle_mod(pl->spell_psys, PMOD_physical_particle);
}

void free_player(struct player* pl) {
    free_psystem(pl->spell_psys);
    free_inventory(pl->inventory);
}


static
void get_movement_input(struct player* pl, float frametime) {
    float speed = 600;
    pl->entity.want_pos = pl->entity.pos;

    if(IsKeyDown(KEY_LEFT_CONTROL)) {
        speed *= 2;
    }
    else
    if(IsKeyDown(KEY_LEFT_SHIFT)) {
        speed *= 0.5;
    }

    Vector2 old_vel = pl->entity.vel;

    if(IsKeyDown(KEY_A)) {
        pl->entity.vel.x -= frametime * speed;
        pl->entity.sprite.flags |= SPRITE_FLIP_HORIZONTAL;
    }
    else
    if(IsKeyDown(KEY_D)) {
        pl->entity.vel.x += frametime * speed;
        pl->entity.sprite.flags &= ~SPRITE_FLIP_HORIZONTAL;
    }
    
    if(IsKeyPressed(KEY_SPACE)) {
        player_jump(pl);
    }
    
    pl->was_moving = pl->moving;
    pl->moving = (Vector2Distance(old_vel, pl->entity.vel) > 0.1f);
}

void player_jump(struct player* pl) {
    pl->entity.vel.y = -200.0;
    pl->jump_counter++;
    pl->jumped = true;
}



/*
static
void set_player_onground(struct player* pl) {
    if(pl->got_surface) {
        pl->entity.pos.y = pl->surface.y;
        //pl->entity.pos.y = pl->surface.y - pl->entity.sprite.height / 2;
    }
}*/


static
void update_position(struct player* pl, float frametime) {
    /*pl->head_pos.x = pl->entity.pos.x + pl->entity.sprite.width / 2;
    pl->head_pos.y = pl->entity.pos.y + 10;
    
    pl->body_pos.x = pl->entity.pos.x + pl->entity.sprite.width / 2;
    pl->body_pos.y = pl->entity.pos.y + pl->entity.sprite.height / 2 + 5;
    
    pl->feet_pos.x = pl->entity.pos.x + pl->entity.sprite.width / 2;
    pl->feet_pos.y = pl->entity.pos.y + pl->entity.sprite.height - 8;
    */


    const float chunk_scale = pl->entity.world->chunks[0].scale;
   

    
    float friction = pow(1.0f - 0.00985f, 500.0f * frametime);
    pl->entity.vel.x *= friction;
   
    // Gravity.
    pl->entity.vel.y += 10.0f * (frametime * 60.0f);
    
    pl->entity.want_pos.x += pl->entity.vel.x * frametime;
    pl->entity.want_pos.y += pl->entity.vel.y * frametime;

    entity_world_collision_adjust(&pl->entity, frametime);

    /*

    pl->want_pos.x += pl->entity.vel.x * frametime;
    pl->want_pos.y += pl->entity.vel.y * frametime;

    
    float friction = pow(1.0f - 0.00885f, 500.0f * frametime);
    pl->entity.vel.x *= friction;
   
    // Gravity.
    pl->entity.vel.y += 10.0f * (frametime * 60.0f);


    float radius = pl->entity.hitareas[0].radius;
    Vector2 center = (Vector2){
        pl->entity.pos.x,
        pl->entity.pos.y
    };


    if(IsKeyDown(KEY_LEFT_ALT)) {
        pl->entity.pos = pl->want_pos;
        return;
    }




    pl->got_surface = get_surface(pl->entity.world, center, NV_DOWN, &pl->surface, NULL);


    // Terrain collision checking.

    bool allow_move_up     = can_move_up(pl->entity.world, center, radius, NULL);
    bool allow_move_down   = can_move_down(pl->entity.world, center, radius, NULL);
    bool allow_move_left   = can_move_left(pl->entity.world, center, radius, NULL);
    bool allow_move_right  = can_move_right(pl->entity.world, center, radius, NULL);

    bool want_move_left  = (pl->want_pos.x < pl->entity.pos.x);
    bool want_move_right = (pl->want_pos.x > pl->entity.pos.x);
    bool want_move_down  = (pl->want_pos.y > pl->entity.pos.y);
    bool want_move_up    = (pl->want_pos.y < pl->entity.pos.y);




    // Handle Left movement.

    if(want_move_left && allow_move_left) {
        pl->entity.pos.x = pl->want_pos.x;
    }

    // Handle Right movement.

    if(want_move_right && allow_move_right) {
        pl->entity.pos.x = pl->want_pos.x;
    }
   
 

    // Handle Up movement.

    if(want_move_up && allow_move_up) {
        pl->entity.pos.y = pl->want_pos.y;
    }
    else
    if(!allow_move_up) {
        pl->entity.vel.y = 0.0f;
        pl->entity.pos.y += 1.0f;
    }

    // Handle Down movement.

    if(want_move_down && allow_move_down) {
        pl->entity.pos.y = pl->want_pos.y;
    }


    if(pl->got_surface) {
        pl->onground = (pl->entity.pos.y + radius > pl->surface.y - 1.0f);
    }
    else {
        pl->onground = false;
    }

    if(pl->onground && !pl->jumped && want_move_down) {
        pl->jump_counter = 0;
        pl->entity.vel.y = 0;
        pl->entity.pos.y = (pl->surface.y - radius) - 0.1f;
    }
    
    if(!pl->onground) {
        pl->jumped = false;
    }
    */

}

static
Vector2 get_spell_direction() {
    Vector2 mouse = GetMousePosition();
    Vector2 screen = (Vector2) {
        GetScreenWidth(),
        GetScreenHeight()
    };

    return Vector2Normalize((Vector2) {
        mouse.x - screen.x / 2,
        mouse.y - screen.y / 2
    });
}

static
void update_attacking(struct gstate* gst, struct player* pl) {

    if(pl->spell_psys->num_particle_mods == 1) {
        return; // Only "physical_particle" mod is enabled
                // Nothing will happen.
    }

    if(pl->attack_button_down
    && (pl->mana > 0.1f * pl->mana_cost_mult)
    && !pl->casting_spell
    && !pl->using_inventory
    ) {
        pl->attack_timer = 0.0f;
        pl->casting_spell = true;


        Vector2 spell_direction = get_spell_direction();

        pl->spell_emitter->cfg.initial_velocity 
            = Vector2Scale(spell_direction, 7.0f);
        
        pl->spell_emitter->cfg.spawn_rect.x = pl->entity.pos.x + spell_direction.x * 30;
        pl->spell_emitter->cfg.spawn_rect.y = pl->entity.pos.y + spell_direction.y * 30;
        pl->spell_emitter->cfg.spawn_rect.width = 8;
        pl->spell_emitter->cfg.spawn_rect.height = 8;


        pl->entity.vel.x -= spell_direction.x * pl->spell_force;
        pl->entity.vel.y -= spell_direction.y * pl->spell_force;

        add_particles(gst, pl->spell_psys, 10);

        pl->mana -= 0.1f * pl->mana_cost_mult;
        pl->mana_regen_timer = 0.0f;
        if(pl->mana <= 1.0f) {
            pl->mana_regen_delay = 3.0f;
        }
        else {
            pl->mana_regen_delay = 1.0f;
        }
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
        case ITEM_WAND: break; // Ignored. This is the default item player has.

        case ITEM_FIREBEND:
            add_particle_mod(pl->spell_psys, PMOD_fire_particle);
            pl->spell_force += 1.5f;
            break;
    
        case ITEM_PLASMABEND:
            add_particle_mod(pl->spell_psys, PMOD_plasma_particle);
            pl->spell_force += 1.75f;
            break;
        
        case ITEM_GRAVITYBEND:
            add_particle_mod(pl->spell_psys, PMOD_gravity_particle);
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
            spawn_item(pl->entity.world, pl->entity.pos, *storage_slot);
            *storage_slot = ITEM_NONE;
        }
    }
}


static
void add_player_camoffset(struct player* pl, float frametime) {    
    
    if(pl->cam_offset_lerp_timer >= 1.0f) {
        pl->cam_offset_lerp_timer = 0.0f;

        Vector2 new_offset = Vector2Scale(pl->entity.vel, 0.02f);
        pl->from_cam_offset = pl->cam_offset;
        pl->to_cam_offset = new_offset;
    }


    pl->cam_offset = Vector2Lerp(pl->from_cam_offset, pl->to_cam_offset, pl->cam_offset_lerp_timer);
    pl->cam_offset_lerp_timer += frametime * 3.0;
}


void update_player(struct gstate* gst, struct player* pl) {
    pl->attack_button_pressed = IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
    pl->attack_button_down    = IsMouseButtonDown(MOUSE_LEFT_BUTTON);

    add_player_camoffset(pl, gst->frametime);

    //pl->cam_offset = Vector2Scale(pl->entity.vel, 0.05f);
    
    update_inventory_control(gst, pl);
    get_movement_input(pl, gst->frametime);

    update_position(pl, gst->frametime);
    update_attacking(gst, pl);
    update_sprite_animation(&pl->entity.sprite, gst->frametime);

    update_psystem(gst, pl->spell_psys);
    

    if(pl->moving && !pl->was_moving) {
        // Started moving.
        sprite_set_animation(&pl->entity.sprite, &gst->animations[ANIM_PLAYER_WALK]);
    }
    else
    if(!pl->moving && pl->was_moving) {
        // Stopped moving.
        sprite_set_animation(&pl->entity.sprite, &gst->animations[ANIM_PLAYER_IDLE]);
    }


    if(pl->mana_regen_timer > pl->mana_regen_delay) {
        pl->mana += pl->mana_regen * gst->frametime;
        pl->mana = CLAMP(pl->mana, 0.0f, pl->max_mana);
    }
    else {
        pl->mana_regen_timer += gst->frametime;
    }
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

    pl->entity.sprite.color_tint = (Color){ 60, 60, 60, 255 };
    entity_render(gst, &pl->entity);
    //render_sprite(gst, &pl->entity.sprite, (Vector2) { pl->entity.pos.x, pl->entity.pos.y - 6.0f });
    
    render_psystem(gst, pl->spell_psys);
    render_pickedup_item(gst, pl);

    if(pl->attack_button_down
    && !pl->using_inventory
    && pl->spell_psys->num_particle_mods > 1) {

        Vector2 wand_pos = pl->entity.pos;
        Vector2 wand_offset = (Vector2){ 6, 13 };

        Vector2 spell_direction = get_spell_direction();
        float angle = atan2(spell_direction.y, spell_direction.x) * RAD2DEG;

        draw_texture(gst->item_textures[ITEM_WAND],
                wand_pos,
                wand_offset,
                angle + 90.0 + 15.0f,
                1.0f, WHITE);
    }
}


void render_player_infobar(struct gstate* gst, struct player* pl) {
    render_inventory(gst, pl->inventory);


    if(IsKeyPressed(KEY_R)) {
        damage_entity(&pl->entity, 10);
    }


    Vector2 infobars_pos = (Vector2) {
        50.0f,
        gst->screen_height - 100
    };

    const float infobar_scale = 1.5f;

    const int innerbar_width = 16 * infobar_scale;
    const int innerbar_height = (64 - 13) * infobar_scale;
   
    const int infobar_offset_x = 16 * infobar_scale;
    const int infobar_offset_y = 8 * infobar_scale;
           

    //map_fvalue(pl->entity.health, 0.0f, pl->entity.max_health, 0.0f, innerbar_height),



    // Health bar.

    float healthbar_offset = 
        map_fvalue(pl->entity.health, 0.0f, pl->entity.max_health, 0.0f, innerbar_height);
    DrawRectangle(
            infobars_pos.x,
            infobars_pos.y + (innerbar_height - healthbar_offset) + 1,
            innerbar_width,
            healthbar_offset,
            RED);


    Color health_low = (Color){ 40, 10, 20, 255 };
    Color health_high = (Color){ 200, 20, 80, 255 };

    DrawTextureEx(gst->textures[TEXTURE_HEALTHBAR],
            (Vector2) {
                infobars_pos.x - infobar_offset_x,
                infobars_pos.y - infobar_offset_y
            },
            0.0f,
            infobar_scale, 
            ColorLerp(health_low, health_high, (float)pl->entity.health / (float)pl->entity.max_health));


    infobars_pos.x += 32 * infobar_scale;



    // Mana bar.

    float manabar_offset = 
        map_fvalue(pl->mana, 0.0f, pl->max_mana, 0.0f, innerbar_height);
    DrawRectangle(
            infobars_pos.x,
            infobars_pos.y + (innerbar_height - manabar_offset) + 1,
            innerbar_width,
            manabar_offset,
            (Color){ 200, 80, 250, 255 });

    Color mana_low = (Color){ 40, 10, 20, 255 };
    Color mana_high = (Color){ 135, 135, 135, 255 };

    DrawTextureEx(gst->textures[TEXTURE_MANABAR],
            (Vector2) {
                infobars_pos.x - infobar_offset_x,
                infobars_pos.y - infobar_offset_y
            },
            0.0f,
            infobar_scale, 
            ColorLerp(mana_low, mana_high, pl->mana / pl->max_mana));




}

