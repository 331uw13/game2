#ifndef ENEMY_ENTITY_H
#define ENEMY_ENTITY_H


struct gstate;


enum enemy_type {
    ENEMY_BAT,
    // ...

    ENEMY_TYPES_COUNT
};



struct enemy {
    enum enemy_type type;

    // Controlled by entity vision modifier './src/entity_mods/vision.c'
    bool can_see_player;

   
    union {
        struct {
            
        }
        bat;

        // ... More enemies will be added later ...
    };
};




#endif
