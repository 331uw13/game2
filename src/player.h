#ifndef PLAYER_H
#define PLAYER_H

#include <raylib.h>
#include <rcamera.h>

#include "sprite.h"
#include "world/world.h"
#include "psystem.h"
#include "inventory.h"
#include "entity/entity.h"



struct player {
    Camera2D cam;

    struct entity entity;
    Vector2 want_pos;

    /*
    Vector2 pos;
    Vector2 want_pos;
    Vector2 old_pos;
    Vector2 vel;
    */
    /*Vector2 feet_pos;
    Vector2 head_pos;
    Vector2 body_pos;*/
    Vector2 surface;
    bool    got_surface;

    float mana;
    float max_mana;
    float mana_regen;
    float mana_regen_timer;
    float mana_regen_delay;
    float mana_cost_mult;

    bool moving; // TODO: Rename to "moving_by_user"
    bool was_moving;
    bool onground;
    int  jump_counter;
    bool jumped;
    bool attack_button_pressed;
    bool attack_button_down;

    float attack_timer;
    float attack_delay;
    bool  casting_spell;


    bool              using_inventory;
    struct inventory* inventory;
    struct item*      pickedup_item;
    // int mana;
    // int health;

    float spell_force;
    struct psystem*    spell_psys;
    struct ps_emitter* spell_emitter;

    //struct sprite sprite;
    //struct world* world;
};


struct gstate;

void create_player(struct gstate* gst, struct world* world, struct player* pl, Vector2 spawn_pos);
void update_player(struct gstate* gst, struct player* pl);
void render_player(struct gstate* gst, struct player* pl);
void player_jump(struct player* pl);
void free_player(struct player* pl);

void render_player_infobar(struct gstate* gst, struct player* pl);


#endif
