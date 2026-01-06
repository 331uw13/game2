#include <math.h>
#include <stdlib.h>
#include <stdio.h>

#include "entity_move_mod_funcs.h"
#include "../state.h"
#include "../common.h"
#include "../errmsg.h"





void ENTMOVMOD_enemy_walking(struct gstate* gst, struct entity* entity) {

    entity->vel.y += 1.0f * (gst->frametime * 50.0f);
    entity->want_pos.x += entity->vel.x * (gst->frametime * 30.0f);    
    entity->want_pos.y += entity->vel.y * (gst->frametime * 30.0f);


    entity_world_collision_adjust(entity, gst->frametime);




}

