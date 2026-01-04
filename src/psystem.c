#include <stdlib.h>
#include <stddef.h>
#include <string.h>


#include "psystem.h"
#include "state.h"
#include "errmsg.h"
#include "memory.h"



struct psystem* new_psystem(struct world* world, const char* name) {
    struct psystem* ps = malloc(sizeof *ps);
    ps->num_emitters = 0;
    ps->name = strdup(name);
    ps->world = world;
    ps->num_particle_mods = 0;
    ps->num_emitters = 0;
    return ps;
}


struct ps_emitter* add_particle_emitter(struct psystem* ps, uint32_t max_particles, Rectangle spawn_rect) {

    if(ps->num_emitters+1 >= PSYSTEM_EMITTERS_MAX) {
        errmsg("Particle system '%s' has maximum amount of emitters already.\n", ps->name);
        return NULL;
    }

    struct ps_emitter* emitter = &ps->emitters[ps->num_emitters];
    ps->num_emitters++;


    emitter->max_particles = max_particles;
    emitter->particles = calloc(max_particles, sizeof *emitter->particles);
    emitter->next_particle_index = 0;
    emitter->num_particles = 0;
    emitter->psystem = ps;

    emitter->cfg.spawn_rect = spawn_rect;
    emitter->cfg.initial_velocity = (Vector2){ 0, 0 };
    emitter->cfg.initial_scale = 2.0f;

    for(uint32_t i = 0; i < emitter->max_particles; i++) {
        struct particle* part = &emitter->particles[i];
        part->alive = false;
        part->index = i;
    }

    return emitter;
}


static
int find_dead_index(struct ps_emitter* emitter) {
    if(emitter->num_particles >= emitter->max_particles) {
        return -1;
    }

    struct particle* part = emitter->particles + emitter->next_particle_index;

    if(!part->alive) {
        return emitter->next_particle_index;
    }


    for(uint32_t i = 0; i < emitter->max_particles; i++) {
        part = &emitter->particles[i];
        if(!part->alive) {
            return i;
        }
    }

    return -1;
}


void remove_particle(struct ps_emitter* emitter, uint32_t index) {
    struct particle* part = &emitter->particles[index];

    part->alive = false;
    emitter->next_particle_index = index;
    if(emitter->num_particles > 0) {
        emitter->num_particles--;
    }
}

static
void add_one_particle(struct gstate* gst, struct psystem* ps, struct ps_emitter* emitter) {
    
    int part_index = find_dead_index(emitter);
    if(part_index < 0) {
        return;
    }


    struct particle* part = &emitter->particles[part_index];
    if(part->alive) {
        return;
    }
    part->alive = true;

    float rx = drand48() * emitter->cfg.spawn_rect.width;
    float ry = drand48() * emitter->cfg.spawn_rect.height;
    part->pos.x = rx + emitter->cfg.spawn_rect.x - emitter->cfg.spawn_rect.width / 2;
    part->pos.y = ry + emitter->cfg.spawn_rect.y - emitter->cfg.spawn_rect.height / 2;
    part->vel = emitter->cfg.initial_velocity;
    part->acc = (Vector2) { 0, 0 };
    part->lifetime = 1.0f;
    part->scale = emitter->cfg.initial_scale;
    part->color = (Color){ 10, 10, 10, 255 };

    for(uint32_t mi = 0; mi < ps->num_particle_mods; mi++) {
        ps->particle_mods[mi](PMODCTX_PARTICLE_SPAWN, gst, emitter, part);
    }

    if(emitter->num_particles+1 < emitter->max_particles) {
        emitter->num_particles++;
    }
}


void add_particles(struct gstate* gst, struct psystem* ps, uint32_t n) {
    for(uint32_t i = 0; i < ps->num_emitters; i++) {
        for(uint32_t c = 0; c < n; c++) {
            add_one_particle(gst, ps, &ps->emitters[i]);
        }
    }
}

void add_particle_mod(struct psystem* ps, particle_mod_fn* mod) {
    if(ps->num_particle_mods+1 >= PSYSTEM_PARTICLE_MODS_MAX) {
        errmsg("Particle system '%s' has maximum particle mods already.", ps->name);
        return;
    }

    ps->particle_mods[ps->num_particle_mods] = mod;
    ps->num_particle_mods++;
}

void update_psystem(struct gstate* gst, struct psystem* ps) {
    
    float left_edge  = gst->player.entity.pos.x - gst->screen_width / 2;
    float right_edge = gst->player.entity.pos.x + gst->screen_width / 2;
    
    float top_edge    = gst->player.entity.pos.y - gst->screen_height / 2;
    float bottom_edge = gst->player.entity.pos.y + gst->screen_height / 2;



    for(uint32_t mi = 0; mi < ps->num_particle_mods; mi++) {
        for(uint32_t ei = 0; ei < ps->num_emitters; ei++) {
            struct ps_emitter* emitter = &ps->emitters[ei];


            for(uint32_t pi = 0; pi < emitter->max_particles; pi++) {
                struct particle* part = &emitter->particles[pi];

                part->lazy_update = (
                       part->pos.x < left_edge
                    || part->pos.y < top_edge
                    || part->pos.x > right_edge
                    || part->pos.y > bottom_edge
                );

                if(!part->alive) {
                    continue;
                }

                part->lifetime -= gst->frametime;
                if(part->lifetime < 0.0f) {
                    //ps->particle_mods[mi](PMODCTX_PARTICLE_DEATH, gst, emitter, part);
                    remove_particle(emitter, part->index);
                    continue;
                }

                ps->particle_mods[mi](PMODCTX_FRAME_UPDATE, gst, emitter, part);
            }
        }
    }
}



void render_psystem(struct gstate* gst, struct psystem* ps) {
    for(uint32_t i = 0; i < ps->num_emitters; i++) {
        struct ps_emitter* emitter = &ps->emitters[i];


        for(uint32_t i = 0; i < emitter->max_particles; i++) {
            struct particle* part = &emitter->particles[i];
            if(!part->alive || part->lazy_update) {
                continue;
            }

            DrawRectangle(part->pos.x, part->pos.y, part->scale, part->scale, part->color);
        } 
    }
}

void free_psystem(struct psystem* ps) {
    freeif(ps->name);
    freeif(ps);
}
