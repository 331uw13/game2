#include <stdio.h>

#include "entity.h"
#include "../state.h"
#include "../errmsg.h"


void entity_add_movement_mod(struct entity* entity, entity_move_mod_fn_t* mod) {
    if(entity->num_movement_mods+1 >= ENTITY_MOVE_MODS_MAX) {
        errmsg("Entity has already maximum amount of movement modifiers(%i).",
                ENTITY_MOVE_MODS_MAX);
        return;
    }


    entity->movement_mods[entity->num_movement_mods] = mod;
    entity->num_movement_mods++;
}

void entity_update_movement_mods(struct gstate* gst, struct entity* entity) {
    for(uint32_t i = 0; i < entity->num_movement_mods; i++) {
        entity->movement_mods[i](gst, (void*)entity);
    }
}


void update_entity_animation(struct gstate* gst, struct entity* entity) {

    if(entity->type != ENTITY_ENEMY) {
        return; // For now, but more entity types will be added.
    }

    switch(entity->enemy.type) {

        case ENEMY_BAT:
            if(entity->sprite.animptr == NULL) {
                sprite_set_animation(&entity->sprite, &gst->animations[ANIM_ENEMY_BAT_FLY]);
            }
            break;


        default:
            errmsg("Unhandled enemy type %i", entity->enemy.type);
            return;
    }



    update_sprite_animation(&entity->sprite, gst->frametime);
}
