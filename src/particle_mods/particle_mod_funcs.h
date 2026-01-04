#ifndef PARTICLE_MODIFIER_FUNCS_H
#define PARTICLE_MODIFIER_FUNCS_H


#include "particle_mods.h"


// These functions will define
// how the particle will behave.

// Particle system can have pointers to these functions
// and when particle system update happens
// it will add the effects from the function to the particle.


void PMOD_physical_particle   (PARTICLE_MOD_FUNC_ARGS);
void PMOD_fire_particle       (PARTICLE_MOD_FUNC_ARGS);
void PMOD_mirror_particle     (PARTICLE_MOD_FUNC_ARGS);
void PMOD_plasma_particle     (PARTICLE_MOD_FUNC_ARGS);
void PMOD_gravity_particle    (PARTICLE_MOD_FUNC_ARGS);


#endif
