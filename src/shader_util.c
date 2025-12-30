#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include <raylib.h>
#include <rlgl.h>

#include "thirdparty/glad.h"
#include "shader_util.h"
#include "memory.h"
#include "errmsg.h"

#define DISABLE_ERROR_MSG 1 // TODO: Fix errors.



// Default shader vertex attribute names to set location points
// Bound by default to shader location:RL_DEFAULT_SHADER_ATTRIB_NAME_POSITION
#define RL_DEFAULT_SHADER_ATTRIB_NAME_POSITION     "vertexPosition" 
// Bound by default to shader location: RL_DEFAULT_SHADER_ATTRIB_NAME_TEXCOORD
#define RL_DEFAULT_SHADER_ATTRIB_NAME_TEXCOORD     "vertexTexCoord"
// Bound by default to shader location: RL_DEFAULT_SHADER_ATTRIB_NAME_NORMAL
#define RL_DEFAULT_SHADER_ATTRIB_NAME_NORMAL       "vertexNormal"
// Bound by default to shader location: RL_DEFAULT_SHADER_ATTRIB_NAME_COLOR
#define RL_DEFAULT_SHADER_ATTRIB_NAME_COLOR        "vertexColor"
// Bound by default to shader location: RL_DEFAULT_SHADER_ATTRIB_NAME_TANGENT
#define RL_DEFAULT_SHADER_ATTRIB_NAME_TANGENT      "vertexTangent"
// Bound by default to shader location: RL_DEFAULT_SHADER_ATTRIB_NAME_TEXCOORD2
#define RL_DEFAULT_SHADER_ATTRIB_NAME_TEXCOORD2    "vertexTexCoord2"
// Bound by default to shader location: RL_DEFAULT_SHADER_ATTRIB_NAME_BONEIDS
#define RL_DEFAULT_SHADER_ATTRIB_NAME_BONEIDS      "vertexBoneIds"
// Bound by default to shader location: RL_DEFAULT_SHADER_ATTRIB_NAME_BONEWEIGHTS
#define RL_DEFAULT_SHADER_ATTRIB_NAME_BONEWEIGHTS  "vertexBoneWeights"

#define RL_DEFAULT_SHADER_UNIFORM_NAME_MVP         "mvp"               // model-view-projection matrix
#define RL_DEFAULT_SHADER_UNIFORM_NAME_VIEW        "matView"           // view matrix
#define RL_DEFAULT_SHADER_UNIFORM_NAME_PROJECTION  "matProjection"     // projection matrix
#define RL_DEFAULT_SHADER_UNIFORM_NAME_MODEL       "matModel"          // model matrix
#define RL_DEFAULT_SHADER_UNIFORM_NAME_NORMAL      "matNormal"         // normal matrix (transpose(inverse(matModelView))
#define RL_DEFAULT_SHADER_UNIFORM_NAME_COLOR       "colDiffuse"        // color diffuse (base tint color, multiplied by texture color)
#define RL_DEFAULT_SHADER_UNIFORM_NAME_BONE_MATRICES  "boneMatrices"   // bone matrices
#define RL_DEFAULT_SHADER_SAMPLER2D_NAME_TEXTURE0  "texture0"          // texture0 (texture slot active 0)
#define RL_DEFAULT_SHADER_SAMPLER2D_NAME_TEXTURE1  "texture1"          // texture1 (texture slot active 1)
#define RL_DEFAULT_SHADER_SAMPLER2D_NAME_TEXTURE2  "texture2"          // texture2 (texture slot active 2)


static int get_shader_infolog_len(uint32_t shaderid) {
    int log_length = 0;
    glGetShaderiv(shaderid, GL_INFO_LOG_LENGTH, &log_length);
    return log_length;
}

static int get_program_infolog_len(uint32_t shaderid) {
    int log_length = 0;
    glGetProgramiv(shaderid, GL_INFO_LOG_LENGTH, &log_length);
    return log_length;
}


#define SHADER_LOGTYPE_WARNING 0
#define SHADER_LOGTYPE_ERROR 1

static void print_shader_infolog(uint32_t shaderid, size_t max_log_length, int log_type) {
    char* log = NULL;
    int log_size = 0;

    log = malloc(max_log_length);
    if(!log) {
        fprintf(stderr, "\033[31m(ERROR) '%s': Failed to allocate memory for log\033[0m\n",
                __func__);
        return;
    }

    memset(log, 0, max_log_length);
    glGetShaderInfoLog(shaderid, max_log_length, &log_size, log);
   
    if(log_size <= 0) {
        fprintf(stderr, "\033[31m(ERROR) '%s': Log is empty\033[0m\n", __func__);
        goto error;
    }

    if(log_type == SHADER_LOGTYPE_WARNING) {
        printf("\033[35m(SHADER_WARNING):\n%s\033[0m\n", log);
    }
    else
    if(log_type == SHADER_LOGTYPE_ERROR) {
        fprintf(stderr, "\033[31m(SHADER_ERROR):\n%s\033[0m\n", log);
    }

error:
    free(log);
}


static uint32_t compile_shader(const char* code, const int code_size, int shader_type) {
    uint32_t shaderid = 0;
    shaderid = glCreateShader(shader_type);

    if(!code || code_size < 8) {
        fprintf(stderr, "\033[31m(ERROR) '%s': Shader code cannot be empty\033[0m\n",
                __func__);
        goto error;
    }

    glShaderSource(shaderid, 1, &code, &code_size);
    glCompileShader(shaderid);


    int compile_status;
    glGetShaderiv(shaderid, GL_COMPILE_STATUS, &compile_status);

    size_t max_log_length = get_shader_infolog_len(shaderid);


    if(compile_status == GL_FALSE) {
        print_shader_infolog(shaderid, max_log_length, SHADER_LOGTYPE_ERROR);
        
        glDeleteShader(shaderid);
        shaderid = 0;
        goto error;
    }

    if(max_log_length > 0) {
        print_shader_infolog(shaderid, max_log_length, SHADER_LOGTYPE_WARNING);
    }

error:
    return shaderid;
}

static int link_shader_program(uint32_t program) {
    int link_status = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &link_status);
    if(link_status != GL_TRUE) {
        int max_infolog_len = get_program_infolog_len(program);

        if(max_infolog_len < 0) {
            fprintf(stderr, 
                    "\033[31m(ERROR) '%s': Shader program failed to link"
                    " but log is empty\033[0m\n",
                    __func__);
            glDeleteProgram(program);
            goto error;
        }

        char* log = NULL;
        int log_length = 0;
        log = malloc(max_infolog_len);
        

        if(!log) {
            fprintf(stderr, 
                    "\033[31m(ERROR) '%s': Failed to allocate memory"
                    " for linking error log\033[0m\n",
                    __func__);
            goto error;
        }


        glGetProgramInfoLog(program, max_infolog_len, &log_length, log);

        fprintf(stderr, "\033[31m(SHADER_LINKING_ERROR):\n%s\033[0m", log);
        free(log);

        glDeleteProgram(program);
    }
error:
    return link_status;
}


static void bind_program_attrib_locs(uint32_t program) {
    glBindAttribLocation(program, RL_DEFAULT_SHADER_ATTRIB_LOCATION_POSITION, RL_DEFAULT_SHADER_ATTRIB_NAME_POSITION);
    glBindAttribLocation(program, RL_DEFAULT_SHADER_ATTRIB_LOCATION_TEXCOORD, RL_DEFAULT_SHADER_ATTRIB_NAME_TEXCOORD);
    glBindAttribLocation(program, RL_DEFAULT_SHADER_ATTRIB_LOCATION_NORMAL, RL_DEFAULT_SHADER_ATTRIB_NAME_NORMAL);
    glBindAttribLocation(program, RL_DEFAULT_SHADER_ATTRIB_LOCATION_COLOR, RL_DEFAULT_SHADER_ATTRIB_NAME_COLOR);
    glBindAttribLocation(program, RL_DEFAULT_SHADER_ATTRIB_LOCATION_TANGENT, RL_DEFAULT_SHADER_ATTRIB_NAME_TANGENT);
    glBindAttribLocation(program, RL_DEFAULT_SHADER_ATTRIB_LOCATION_TEXCOORD2, RL_DEFAULT_SHADER_ATTRIB_NAME_TEXCOORD2);
}

static void set_shader_attrib_locs(Shader* shader) {
    shader->locs = malloc(RL_MAX_SHADER_LOCATIONS * sizeof(int));
    memset(shader->locs, -1, RL_MAX_SHADER_LOCATIONS * sizeof(int));

    shader->locs[SHADER_LOC_VERTEX_POSITION] = rlGetLocationAttrib(shader->id, RL_DEFAULT_SHADER_ATTRIB_NAME_POSITION);
    shader->locs[SHADER_LOC_VERTEX_TEXCOORD01] = rlGetLocationAttrib(shader->id, RL_DEFAULT_SHADER_ATTRIB_NAME_TEXCOORD);
    shader->locs[SHADER_LOC_VERTEX_TEXCOORD02] = rlGetLocationAttrib(shader->id, RL_DEFAULT_SHADER_ATTRIB_NAME_TEXCOORD2);
    shader->locs[SHADER_LOC_VERTEX_NORMAL] = rlGetLocationAttrib(shader->id, RL_DEFAULT_SHADER_ATTRIB_NAME_NORMAL);
    shader->locs[SHADER_LOC_VERTEX_TANGENT] = rlGetLocationAttrib(shader->id, RL_DEFAULT_SHADER_ATTRIB_NAME_TANGENT);
    shader->locs[SHADER_LOC_VERTEX_COLOR] = rlGetLocationAttrib(shader->id, RL_DEFAULT_SHADER_ATTRIB_NAME_COLOR);
    shader->locs[SHADER_LOC_VERTEX_BONEIDS] = rlGetLocationAttrib(shader->id, RL_DEFAULT_SHADER_ATTRIB_NAME_BONEIDS);
    shader->locs[SHADER_LOC_VERTEX_BONEWEIGHTS] = rlGetLocationAttrib(shader->id, RL_DEFAULT_SHADER_ATTRIB_NAME_BONEWEIGHTS);
    //shader->locs[SHADER_LOC_VERTEX_INSTANCE_TX] = rlGetLocationAttrib(shader->id, RL_DEFAULT_SHADER_ATTRIB_NAME_INSTANCE_TX);

    // Get handles to GLSL uniform locations (vertex shader)
    shader->locs[SHADER_LOC_MATRIX_MVP] = rlGetLocationUniform(shader->id, RL_DEFAULT_SHADER_UNIFORM_NAME_MVP);
    shader->locs[SHADER_LOC_MATRIX_VIEW] = rlGetLocationUniform(shader->id, RL_DEFAULT_SHADER_UNIFORM_NAME_VIEW);
    shader->locs[SHADER_LOC_MATRIX_PROJECTION] = rlGetLocationUniform(shader->id, RL_DEFAULT_SHADER_UNIFORM_NAME_PROJECTION);
    shader->locs[SHADER_LOC_MATRIX_MODEL] = rlGetLocationUniform(shader->id, RL_DEFAULT_SHADER_UNIFORM_NAME_MODEL);
    shader->locs[SHADER_LOC_MATRIX_NORMAL] = rlGetLocationUniform(shader->id, RL_DEFAULT_SHADER_UNIFORM_NAME_NORMAL);
    shader->locs[SHADER_LOC_BONE_MATRICES] = rlGetLocationUniform(shader->id, RL_DEFAULT_SHADER_UNIFORM_NAME_BONE_MATRICES);

    // Get handles to GLSL uniform locations (fragment shader)
    shader->locs[SHADER_LOC_COLOR_DIFFUSE] = rlGetLocationUniform(shader->id, RL_DEFAULT_SHADER_UNIFORM_NAME_COLOR);
    shader->locs[SHADER_LOC_MAP_DIFFUSE] = rlGetLocationUniform(shader->id, RL_DEFAULT_SHADER_SAMPLER2D_NAME_TEXTURE0);  // SHADER_LOC_MAP_ALBEDO
    shader->locs[SHADER_LOC_MAP_SPECULAR] = rlGetLocationUniform(shader->id, RL_DEFAULT_SHADER_SAMPLER2D_NAME_TEXTURE1); // SHADER_LOC_MAP_METALNESS
    shader->locs[SHADER_LOC_MAP_NORMAL] = rlGetLocationUniform(shader->id, RL_DEFAULT_SHADER_SAMPLER2D_NAME_TEXTURE2);

    printf("%s %p\n",__func__, shader->locs);
}


bool load_shader(
        const char* vs_filepath,
        const char* fs_filepath,
        const char* gs_filepath,
        Shader* shader
){
    int result = 0;
    int has_geometry_shader = (gs_filepath != NO_GEOMETRY_SHADER);

    /*
    platform_file_t vertex_file = { 0 };
    platform_file_t fragment_file = { 0 };
    platform_file_t geometry_file = { 0 };

    platform_init_file(&vertex_file);
    platform_init_file(&fragment_file);
    if(has_geometry_shader) {
        platform_init_file(&geometry_file);
    }
    */

    printf("\033[36m,-> Compile and link:\n"
            "\033[36m:    \033[90m (Vertex shader)   \033[34m%s\n"
            "\033[36m:    \033[90m (Fragment shader) \033[34m%s\n"
            "\033[36m:    \033[90m (Geometry shader) \033[34m%s\n"
            "\033[0m",
            vs_filepath,
            fs_filepath,
            (!has_geometry_shader) ? "\033[90m(No geometry shader)\033[0m" : gs_filepath
            );

    /*
    // Errors are reported from functions.
    if(!platform_read_file(&vertex_file, vs_filepath)) {
        goto error;
    }
    if(!platform_read_file(&fragment_file, fs_filepath)) {
        goto error_and_close;
    }
    // Geometry shader is optional.
    if(has_geometry_shader) { 
        if(!platform_read_file(&geometry_file, gs_filepath)) {
            goto error_and_close;
        }
    }
    */
    char* fs_code = LoadFileText(fs_filepath);
    char* vs_code = LoadFileText(vs_filepath);
    char* gs_code = has_geometry_shader ? LoadFileText(gs_filepath) : NULL;

    size_t fs_code_size = strlen(fs_code);
    size_t vs_code_size = strlen(vs_code);
    size_t gs_code_size = has_geometry_shader ? strlen(gs_code) : 0;



    /*
    printf("----------- VERTEX SHADER --------------\n");
    printf("\033[90m%s\033[0m\n", vs_code);
   
    printf("----------- FRAGMENT SHADER --------------\n");
    printf("\033[90m%s\033[0m\n", fs_code);
    */

    // Try to compile the vertex shader.
    uint32_t vs_shaderid 
        = compile_shader(vs_code, vs_code_size, GL_VERTEX_SHADER);

    if(vs_shaderid == 0) {
        fprintf(stderr, "\033[31m(ERROR) '%s': Vertex shader failed.\033[0m\n",
                __func__);
        goto error_and_free;
    }


    // Try to compile the fragment shader.
    uint32_t fs_shaderid 
        = compile_shader(fs_code, fs_code_size, GL_FRAGMENT_SHADER);

    if(fs_shaderid == 0) {
        fprintf(stderr, "\033[31m(ERROR) '%s': Fragment shader failed.\033[0m\n",
                __func__);

        glDeleteShader(vs_shaderid);
        goto error_and_free;
    }

    // Try to compile the geometry shader if it was needed.
    uint32_t gs_shaderid = 0;
    if(has_geometry_shader) {
        gs_shaderid = compile_shader(gs_code, gs_code_size, GL_GEOMETRY_SHADER);
        if(gs_shaderid == 0) {
            fprintf(stderr, "\033[31m(ERROR) '%s': Geometry shader failed.\033[0m\n",
                    __func__);
            glDeleteShader(vs_shaderid);
            glDeleteShader(fs_shaderid);
            goto error_and_free;
        }
    }

    uint32_t program = 0;
    program = glCreateProgram();


    glAttachShader(program, vs_shaderid);
    glAttachShader(program, fs_shaderid);
    if(has_geometry_shader) {
        glAttachShader(program, gs_shaderid);
    }
    
    bind_program_attrib_locs(program);
    glLinkProgram(program);

    // Individual shaders are not needed anymore.
    glDeleteShader(vs_shaderid);
    glDeleteShader(fs_shaderid);
    if(has_geometry_shader) {
        glDeleteShader(gs_shaderid);
    }


    if(!link_shader_program(program)) {
        goto error_and_free;   
    }

    shader->id = program;
    set_shader_attrib_locs(shader);

    if(shader->id) {
        printf("\033[36m`-> \033[32mSuccess.\033[0m\n");
    }
    else {
        printf("\033[36m`-> \033[31mFailed.\033[0m\n");
    }


    result = 1;


error_and_free:
    
    if(fs_code) {
        UnloadFileText(fs_code);
    }
    if(vs_code) {
        UnloadFileText(vs_code);
    }
    if(gs_code) {
        UnloadFileText(gs_code);
    }


    return result;
}

void free_shader(Shader* shader) {
    freeif(shader->locs);
    glDeleteProgram(shader->id);
}


void uniform1f(Shader shader, const char* name, float v) {
    glUseProgram(shader.id);
    int loc = glGetUniformLocation(shader.id, name);

    if(loc < 0) {
        errmsg("Could not find location '%s' for shader '<TODO>'", name);
    }

    glUniform1f(loc, v); 
}

void uniform2f(Shader shader, const char* name, Vector2 v) {
    glUseProgram(shader.id);
    int loc = glGetUniformLocation(shader.id, name);

    if(loc < 0) {
        errmsg("Could not find location '%s' for shader '<TODO>'", name);
    }

    glUniform2f(loc, v.x, v.y);  
}

void uniform3f(Shader shader, const char* name, Vector3 v) {
    glUseProgram(shader.id);
    int loc = glGetUniformLocation(shader.id, name);

    if(loc < 0) {
        errmsg("Could not find location '%s' for shader '<TODO>'", name);
    }

    glUniform3f(loc, v.x, v.y, v.z);
}

/*
int load_compute_shader(
        struct state_t* gst,
        const char* filepath,
        int shader_index
){
    int result = 0;

    platform_file_t compute_file = { 0 };
    platform_init_file(&compute_file);

    printf("\033[36m,-> Compile and link:\n"
            "\033[36m:    \033[90m (Compute shader)   \033[34m%s\n"
            "\033[0m",
            filepath
            );
    
    if(!platform_read_file(&compute_file, filepath)) {
        goto error;
    }

    size_t code_size = 0;
    char* code = preproc_glsl(&compute_file, &code_size);


    uint32_t sh 
        = compile_shader(code, code_size, GL_COMPUTE_SHADER);

    if(sh == 0) {
        fprintf(stderr, "\033[31m(ERROR) '%s': Compute shader failed.\033[0m\n",
                __func__);
        goto error_and_free;
    }


    uint32_t program = 0;
    program = glCreateProgram();

    glAttachShader(program, sh);
    glLinkProgram(program);

    // Individual shaders are not needed anymore.
    glDeleteShader(sh);

    if(!link_shader_program(program)) {
        goto error_and_free;   
    }


    gst->shaders[shader_index].id = program;
    gst->shaders[shader_index].locs = NULL;

    result = 1;

error_and_free:

    if(code) {
        free(code);
    }

    platform_close_file(&compute_file);

    if(!result) {
        STATE_ABORT(gst, "Compute shader doesnt work.");
    }
error:
    return result;
}


static int get_uloc(struct state_t* gst, int shader_index, int shader_u) {
    int res = U_NOTFOUND;

    if((shader_u < 0) || (shader_u >= MAX_UNIFORM_LOCS)) {
        fprintf(stderr, "\033[31m(ERROR) '%s': 'shader_u' out of bounds for shader index %i.\033[0m\n",
                __func__, shader_index);
        goto error;
    }

    uloc_t* ulocptr = &gst->shader_u[shader_index].ulocs[shader_u];

    if(*ulocptr == U_NOTFOUND) {
        int loc = GetShaderLocation(gst->shaders[shader_index], g_shader_uniform_names[shader_u]);
        if(loc < 0) {
            if(!DISABLE_ERROR_MSG) {
                fprintf(stderr, "\033[31m(ERROR) '%s': Uniform with name '%s' not found for shader index %i, loc=%i\033[0m\n"
                    "\033[90m(Note: shader compiler might optimize out unused uniforms!)\033[0m\n",
                    __func__, g_shader_uniform_names[shader_u], shader_index, loc);
            }
            goto error;
        }
       
        printf("Shader uniform found: '%s' (%i) shader_index=%i\n",
                g_shader_uniform_names[shader_u], loc, shader_index);
        *ulocptr = (uloc_t)loc;
    }

    res = (int)*ulocptr;

error:
    return res;
}

void init_shaderutil(struct state_t* gst) {
    for(size_t s_i = 0; s_i < MAX_SHADERS; s_i++) {
        for(size_t i = 0; i < MAX_UNIFORM_LOCS; i++) {
            gst->shader_u[s_i].ulocs[i] = U_NOTFOUND;
        }
    }
}


void dispatch_compute(
        struct state_t* gst,
        int compute_shader_index,
        size_t num_groups_x,
        size_t num_groups_y,
        size_t num_groups_z,
        int barrier_bit
){
    rlEnableShader(gst->shaders[compute_shader_index].id);
    //glUseProgram(gst->compute_shaders[compute_shader_index]);
    glDispatchCompute(num_groups_x, num_groups_y, num_groups_z);

    // TODO: This maybe can moved to somewhere so something else can be done while the compute shader finishes??
    glMemoryBarrier(barrier_bit);
}


void shader_setu_float(struct state_t* gst, int shader_index, int shader_u, float* v) {
    int loc = get_uloc(gst, shader_index, shader_u);
    if(loc == U_NOTFOUND) {
        return;
    }

    SetShaderValueV(gst->shaders[shader_index], loc, v, SHADER_UNIFORM_FLOAT, 1);
}

void shader_setu_int(struct state_t* gst, int shader_index, int shader_u, int* v) {
    int loc = get_uloc(gst, shader_index, shader_u);
    if(loc == U_NOTFOUND) {
        return;
    }

    SetShaderValueV(gst->shaders[shader_index], loc, v, SHADER_UNIFORM_INT, 1);
}


void shader_setu_vec2(struct state_t* gst, int shader_index, int shader_u, Vector2* v) {
    int loc = get_uloc(gst, shader_index, shader_u);
    if(loc == U_NOTFOUND) {
        return;
    }

    SetShaderValueV(gst->shaders[shader_index], loc, v, SHADER_UNIFORM_VEC2, 1);
}

void shader_setu_vec3(struct state_t* gst, int shader_index, int shader_u, Vector3* v) {
    int loc = get_uloc(gst, shader_index, shader_u);
    if(loc == U_NOTFOUND) {
        return;
    }

    SetShaderValueV(gst->shaders[shader_index], loc, v, SHADER_UNIFORM_VEC3, 1);
}

void shader_setu_vec4(struct state_t* gst, int shader_index, int shader_u, Vector4* v) {
    int loc = get_uloc(gst, shader_index, shader_u);
    if(loc == U_NOTFOUND) {
        return;
    }

    SetShaderValueV(gst->shaders[shader_index], loc, v, SHADER_UNIFORM_VEC4, 1);
}

void shader_setu_ivec4 (struct state_t* gst, int shader_index, int shader_u, struct vec4int_t* v) {
    int loc = get_uloc(gst, shader_index, shader_u);
    if(loc == U_NOTFOUND) {
        return;
    }

    SetShaderValueV(gst->shaders[shader_index], loc, v, SHADER_UNIFORM_IVEC4, 1);
}

void shader_setu_sampler(struct state_t* gst, int shader_index, int shader_u, int texid) {
    int loc = get_uloc(gst, shader_index, shader_u);
    if(loc == U_NOTFOUND) {
        return;
    }

    rlEnableShader(gst->shaders[shader_index].id);
    rlSetUniformSampler(loc, texid);
}

void shader_setu_matrix(struct state_t* gst, int shader_index, int shader_u, Matrix m) {
    int loc = get_uloc(gst, shader_index, shader_u);
    if(loc == U_NOTFOUND) {
        return;
    }

    SetShaderValueMatrix(gst->shaders[shader_index], loc, m);
}

void shader_setu_color(struct state_t* gst, int shader_index, int shader_u, Color* c) {
    
    Vector4 v4f = (Vector4) {
        (float)c->r / 255.0,
        (float)c->g / 255.0,
        (float)c->b / 255.0,
        (float)c->a / 255.0
    };

    shader_setu_vec4(gst, shader_index, shader_u, &v4f);

}
*/

