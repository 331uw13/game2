#ifndef ENTITY_MOVE_MODIFIER_FUNCS_H
#define ENTITY_MOVE_MODIFIER_FUNCS_H


// These functions define how entity can move
// by "entity" i mean any living creature but not the player.

// Movement modifiers can be attached to entities.
// "base modifiers" are for the very defining style of movement.
// And others are extra modifiers.

// Movement modifiers expect position (which it will update if need)
// and radius which is used to check collisions.

#include <raylib.h>


struct world;



void ENTMOVMOD_flying(struct world* world, Vector2* pos, float collision_radius);





#endif
