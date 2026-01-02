#include <math.h>
#include <stddef.h>

#include "bloom.h"



#define NUM_SAMPLES  6
#define SCALE_FACTOR 0.6f

#define GLSL_VERSION "#version 330\n"


static const char* BLOOM_TRESHOLD_FRAGMENT = 
GLSL_VERSION
"in vec2 frag_texcoord;"
"in vec4 frag_color;"
"in vec3 frag_normal;"
"in vec3 frag_position;"
""
"uniform sampler2D texture0;"
""
"out vec4 out_color;"
""
"void main() {"
"    vec4 black = vec4(0.0, 0.0, 0.0, 1.0);"
"    vec4 result = texture(texture0, frag_texcoord);"
"    float light = dot(result.rgb, vec3(0.9, 2.0, 0.9));"
""
"    out_color.rgb = mix(black, result, pow(light, 4.0)).rgb*0.5;"
"    out_color.a = result.a;"
"}";



static const char* BLOOM_DOWNSAMPLE_FRAGMENT =
GLSL_VERSION
"in vec2 frag_texcoord;"
"in vec4 frag_color;"
"in vec3 frag_normal;"
"in vec3 frag_position;"
""
"uniform sampler2D texture0;"
""
"out vec4 out_color;"
""
"void main() {"
""
"    int size = 2;"
"    vec2 texture_size = vec2(textureSize(texture0, 0));"
"    vec2 texel_size = 1.0 / texture_size;"
"    vec2 uv = gl_FragCoord.xy / texture_size;"
""
"    vec3 result = vec3(0, 0, 0);"
"    float sum = 0.0;"
""
"    for(int x = -size; x <= size; x++) {"
"        for(int y = -size; y <= size; y++) {"
"            vec2 offset = vec2(float(x), float(y));"
"            vec2 texel_pos = frag_texcoord + offset * texel_size;"
"            if(texel_pos.y > 0.99) { break; }"
"            if(texel_pos.y < 0.01) { break; }"
"            if(texel_pos.x > 0.99) { break; }"
"            if(texel_pos.x < 0.01) { break; }"
""
"            sum += 1.0;"
"            result += texture(texture0, texel_pos).rgb;"
"        }"
"    }"
""
"    result /= sum;"
"    out_color = vec4(result, 1.0);"
"}";


static const char* BLOOM_UPSAMPLE_FRAGMENT = 
GLSL_VERSION
"in vec2 frag_texcoord;"
"in vec4 frag_color;"
"in vec3 frag_normal;"
"in vec3 frag_position;"
""
"uniform sampler2D texture0;"
""
"out vec4 out_color;"
""
"void main() {"
""
"    int size = 2;"
"    vec2 texture_size = vec2(textureSize(texture0, 0));"
""
"    vec2 texel_size = 1.0 / texture_size;"
"    vec2 uv = gl_FragCoord.xy / texture_size;"
"    vec3 result = vec3(0, 0, 0);"
"    float sum = 0;"
""
"    for(int x = -size; x <= size; x++) {"
"        for(int y = -size; y <= size; y++) {"
"            vec2 offset = vec2(float(x), float(y));"
"            vec2 texel_pos = frag_texcoord + offset * texel_size;"
""
"            if(texel_pos.y > 0.99) { break; }"
"            if(texel_pos.y < 0.01) { break; }"
"            if(texel_pos.x > 0.99) { break; }"
"            if(texel_pos.x < 0.01) { break; }"
""
"            float dist = distance(frag_texcoord, texel_pos);"
""
"            result += texture(texture0, texel_pos).rgb * dist;"
"            sum += dist;"
"        }"
"    }"
""
"    result /= sum;"
"    out_color = vec4(result, 1.0);"
"}";

static const char* DEFAULT_VERTEX = 
GLSL_VERSION
"in vec3 vertexPosition;"
"in vec2 vertexTexcoord;"
"in vec3 vertexNormal;"
"in vec4 vertexColor;"
""
""
"uniform mat4 mvp;"
"uniform mat4 matModel;"
"uniform mat4 matNormal;"
""
""
"out vec2 frag_texcoord;"
"out vec4 frag_color;"
"out vec3 frag_normal;"
"out vec3 frag_position;"
""
"void main() {"
""
"    frag_texcoord = vertexTexcoord;"
"    frag_color = vertexColor;"
"    vec3 vertex_pos = vertexPosition;"
"    frag_position = vec3(matModel*vec4(vertex_pos, 1.0));"
"    frag_normal = normalize(vec3(matNormal * vec4(vertexNormal, 1.0)));"
"    gl_Position = mvp * vec4(vertex_pos, 1.0);"
"}";


struct bloom_ {
    RenderTexture2D targets [NUM_SAMPLES];

    Shader treshold_shader;
    Shader upsample_shader;
    Shader downsample_shader;

}
bloom;


void init_bloom(int screen_width, int screen_height) {    
    int sample_res_X = screen_width;
    int sample_res_Y = screen_height;


    float sX = 0.8;
    float sY = 0.8;

    for(int i = 0; i < NUM_SAMPLES; i++) {
        RenderTexture2D* target = &bloom.targets[i];
        *target = LoadRenderTexture(sample_res_X, sample_res_Y);
        SetTextureFilter(target->texture, TEXTURE_FILTER_BILINEAR);

        sample_res_X = round((float)sample_res_X * sX);
        sample_res_Y = round((float)sample_res_Y * sY);
    }

    bloom.treshold_shader = LoadShaderFromMemory  (DEFAULT_VERTEX, BLOOM_TRESHOLD_FRAGMENT);
    bloom.upsample_shader = LoadShaderFromMemory (DEFAULT_VERTEX, BLOOM_UPSAMPLE_FRAGMENT);
    bloom.downsample_shader = LoadShaderFromMemory(DEFAULT_VERTEX, BLOOM_DOWNSAMPLE_FRAGMENT);
}

void free_bloom() {
    for(int i = 0; i < NUM_SAMPLES; i++) {
        UnloadRenderTexture(bloom.targets[i]);
    }
    UnloadShader(bloom.treshold_shader);
    UnloadShader(bloom.upsample_shader);
    UnloadShader(bloom.downsample_shader);
}

static
void resample_texture(
        RenderTexture2D to,
        RenderTexture2D from,
        Shader* shader
){
    BeginTextureMode(to);
    ClearBackground((Color){ 0, 0, 0, 0 });
    if(shader) {
        BeginShaderMode(*shader);
    }
    int src_width = from.texture.width;
    int src_height = from.texture.height;
    int dst_width = to.texture.width;
    int dst_height = to.texture.height;
    
    DrawTexturePro(
            from.texture,
            (Rectangle){ 0, 0, (float)src_width, -(float)src_height },
            (Rectangle){ 0, 0, (float)dst_width, (float)dst_height },
            (Vector2){ 0, 0 }, 0, WHITE);

    if(shader) {
        EndShaderMode();
    }

    EndTextureMode();
}


void render_bloom(RenderTexture2D scene, RenderTexture2D* result_out) {
    // Get bloom treshold texture.
    resample_texture(
            /* TO */   bloom.targets[0],
            /* FROM */ scene,
            &bloom.treshold_shader);

    size_t p = 0;

    // Scale down and filter the texture each step.
    for(size_t i = 1; i < NUM_SAMPLES; i++) {
        p = i - 1; // Previous sample.

        resample_texture(
                /* TO */   bloom.targets[i],
                /* FROM */ bloom.targets[p],
                &bloom.downsample_shader);
    }

    // Scale up and filter the texture each step.
    for(size_t i = NUM_SAMPLES-2; i > 0; i--) {
        p = i + 1; // Previous sample.

        resample_texture(
                /* TO */   bloom.targets[i],
                /* FROM */ bloom.targets[p],
                &bloom.upsample_shader);
    }


    // Now the result is ready.

    resample_texture(
            /* TO */   *result_out,
            /* FROM */ bloom.targets[1],
            &bloom.upsample_shader);

}

