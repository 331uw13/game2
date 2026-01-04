
#include "state.h"
#include "enemy.h"
#include "errmsg.h"

void update_enemy_animation(struct gstate* gst, struct enemy* enemy) {
    
    switch(enemy->type) {

        case ENEMY_BAT:
            sprite_set_animation(&enemy->entity.sprite, &gst->animations[ANIM_ENEMY_BAT_FLY]);
            break;

        default:
            errmsg("Enemy animation update is not handled for enemy type %i", enemy->type);
            return;
    }


    update_sprite_animation(&enemy->entity.sprite, gst->frametime);
}
