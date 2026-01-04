#ifndef PARTICLE_MODIFIERS_H
#define PARTICLE_MODIFIERS_H


// Some helping definitions for particle modifiers
// so this dont need to be copy pasted manualy
// everytime new particle mod is created.


struct gstate;
struct psystem;
struct ps_emitter;
struct particle;


enum pmod_context {
    PMODCTX_FRAME_UPDATE,
    PMODCTX_PARTICLE_SPAWN,
    PMODCTX_PARTICLE_DEATH
};


typedef void (particle_mod_fn)(
    enum pmod_context,
    struct gstate*,
    struct ps_emitter*,
    struct particle*
);


#define PARTICLE_MOD_FUNC_ARGS\
    enum pmod_context context,\
    struct gstate* gst,\
    struct ps_emitter* emitter,\
    struct particle* part



#endif
