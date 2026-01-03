#include <dirent.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#include "animation.h"
#include "string.h"
#include "errmsg.h"



bool load_animation(struct animation* anim, char* directory) {    
    DIR* dir = opendir(directory);
    if(!dir) {
        errmsg("Failed to open directory '%s' | %s",
                directory, strerror(errno));
        return false;
    }

    struct dirent* ent = NULL;
    uint32_t num_textures = 0;
    while((ent = readdir(dir))) {
        if(ent->d_name[0] == '.') {
            continue;
        }

        if(ent->d_type == DT_REG) {
            num_textures++;
        }
    }

    if(num_textures == 0) {
        errmsg("Didnt find any files from directory '%s'", directory);
        return false;
    }


    if(num_textures >= ANIMATION_TEXTURES_MAX) {
        errmsg("Animation '%s' has too many textures: %i",
                directory, num_textures);

        num_textures = ANIMATION_TEXTURES_MAX - 1;
    }

    // Now we can build paths to the files.

    struct string_t dirstr = string_create(0);
    string_append(&dirstr, directory, -1);
    if(string_lastbyte(&dirstr) != '/') {
        string_pushbyte(&dirstr, '/');
    }

    string_nullterm(&dirstr);



    struct string_t texpath = string_create(0);

    for(uint32_t i = 0; i < num_textures; i++) {
    
        string_clear(&texpath);
        string_append(&texpath, dirstr.bytes, dirstr.size);
       
        char filename_buffer[32] = { 0 };
        ssize_t filename_len = snprintf(filename_buffer,
                sizeof(filename_buffer)-1,
                "%i.png", i);

        if(filename_len < 0) {
            errmsg("snprintf() failed while loading textures from '%s' | %s",
                    directory, strerror(errno));
            continue;
        }


        string_append(&texpath, filename_buffer, filename_len);
        string_nullterm(&texpath);

        anim->textures[anim->num_textures] = LoadTexture(texpath.bytes);
        if(!IsTextureValid(anim->textures[i])) {
            errmsg("Failed to load texture '%s'", texpath.bytes);
            continue;
        }

        anim->num_textures++;
    }



    free_string(&dirstr); 
    return true;
}

void free_animation(struct animation* anim) {
    for(uint32_t i = 0; i < anim->num_textures; i++) {
        UnloadTexture(anim->textures[i]);
    }
}

