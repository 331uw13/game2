#ifndef SHADER_UTIL_H
#define SHADER_UTIL_H

#include <raylib.h>
#include <stdbool.h>


#define NO_GEOMETRY_SHADER NULL



bool load_shader(
        const char* vs_filepath, // (Vertex shader)
        const char* fs_filepath, // (Fragment shader)
        const char* gs_filepath, // (Geometry shader)
        Shader* shader);

void free_shader(Shader* shader);


void uniformv3(Shader shader, const char* name, Vector3 v);

/*
// TODO: Struct for compute shader.
void dispatch_compute(
        struct state_t* gst,
        int compute_shader_index,
        size_t num_groups_x,
        size_t num_groups_y,
        size_t num_groups_z,
        int barrier_bit
        );
*/
/*
int load_compute_shader(
        struct state_t* gst,
        const char* filepath,
        int shader_index
        );
*/
#endif
