#include <raymath.h>
#include <stdio.h>

#include "state.h"
#include "player.h"
#include "common.h"
#include "world/chunk.h"


void create_player(struct world* world, struct player* pl, Vector2 spawn_pos) {
    
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
    pl->attack_delay = 0.01f;
    pl->onground = false;
    pl->world = world;

    load_sprite(&pl->sprite, "./sprites/player");
    sprite_set_anim(&pl->sprite, "idle");

    pl->spell_psys = new_psystem(world, "player_spell_psystem");
    pl->spell_emitter = add_particle_emitter(pl->spell_psys, 1000, (Rectangle){ 0, 0, 20, 20 });




    add_particle_mod(pl->spell_psys, PMOD_fire_particle);
    add_particle_mod(pl->spell_psys, PMOD_physical_particle);
}

void free_player(struct player* pl) {
    free_sprite(&pl->sprite);
    free_psystem(pl->spell_psys);
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


    if(IsMouseButtonDown(MOUSE_LEFT_BUTTON) && !pl->casting_spell) {
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

        add_particles(gst, pl->spell_psys, 10);
    }


    if(pl->casting_spell) {
        pl->attack_timer += gst->frametime;
        if(pl->attack_timer >= pl->attack_delay) {
            pl->casting_spell = false;
        }
    }
}


void update_player(struct gstate* gst, struct player* pl) {
    get_movement_input(pl, gst->frametime);
    update_position(pl, gst->frametime);
    update_attacking(gst, pl);
    sprite_update_anim(&pl->sprite, gst->frametime);

    update_psystem(gst, pl->spell_psys);
}


void render_player(struct gstate* gst, struct player* pl) {
    render_sprite(&pl->sprite, pl->pos);
    render_psystem(pl->spell_psys);

    
    if(IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {

        Vector2 wand_pos = pl->pos;
        Vector2 wand_offset = (Vector2){ 3, 13 };

        /*
        float angle = Lerp(120.0f, 40.0f, pl->attack_timer); 
        angle *= pl->attack_side;
        */

        float angle = atan2(pl->spell_direction.y, pl->spell_direction.x) * RAD2DEG;

        draw_texture(gst->item_textures[ITEM_WAND],
                wand_pos,
                wand_offset,
                angle + 90.0,
                1.0f, WHITE);
    }
}

