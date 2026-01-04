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

    // This counts the frames between 'can_see_player' checks.
    // To know if the enemy can see the player,
    // we can cast ray from the enemy position towards player position.
    // But probably we can just check if every few frames
    uint32_t can_see_player_check_countdown;

    // Set to 'true' for first modifier update when enemy spawns.
    // So that first enemy modifier can set the initial velocity if needed.
    bool spawn_event;

    union {
        struct {
            
        }
        bat;

        // ... More enemies will be added later ...
    };
};



#endif
