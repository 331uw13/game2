#ifndef PARTICLE_SYSTEM_H
#define PARTICLE_SYSTEM_H

#include <stdint.h>
#include <raylib.h>

#include "particle_mods/functions.h"
#include "particle_mods.h"
#include "world/world.h"

#define PSYSTEM_EMITTERS_MAX 32
#define PSYSTEM_PARTICLE_MODS_MAX 32


struct particle {
    Vector2 pos;
    Vector2 vel;
    Vector2 acc;
    float   lifetime;
    Color   color;
    float   scale;

    bool             alive;
    uint32_t         index;

    // If the particle is offscreen this is set to 'true'
    // and particle modifier functions dont need to add 
    // special effects.
    bool lazy_update;
};


struct ps_emitter_config {
    Rectangle spawn_rect;
    Vector2   initial_velocity;
    float     initial_scale;
};

struct ps_emitter {
    struct particle* particles;
    uint32_t         num_particles;
    uint32_t         max_particles;
    uint32_t         next_particle_index;


    struct psystem* psystem;
    struct ps_emitter_config cfg;
};


struct psystem {

    struct ps_emitter emitters [PSYSTEM_EMITTERS_MAX];
    uint32_t          num_emitters;

    particle_mod_fn*  particle_mods [PSYSTEM_PARTICLE_MODS_MAX];
    uint32_t          num_particle_mods;

    struct world*     world;

    // TODO: Particle Modifiers.
    //       they will define the partiles behaviour.

    
    char* name; // Used for better error messages.
};


struct gstate;


struct psystem* new_psystem(struct world* world, const char* name);
struct ps_emitter* add_particle_emitter(struct psystem* ps, uint32_t max_particles, Rectangle spawn_rect);

void add_particles(struct gstate* gst, struct psystem* ps, uint32_t n);

void add_particle_mod(struct psystem* ps, particle_mod_fn* mod);

void remove_particle(struct ps_emitter* emitter, uint32_t index);

void update_psystem(struct gstate* gst, struct psystem* ps);
void render_psystem(struct gstate* gst, struct psystem* ps);
void free_psystem(struct psystem* ps);

void show_link_list(struct ps_emitter* emitter);


#endif
