#version 330

in vec2 frag_texcoord;
in vec4 frag_color;
in vec3 frag_normal;
in vec3 frag_position;

uniform float time;
uniform sampler2D texture_result;
uniform sampler2D texture_bloom;
uniform sampler2D texture_gui;
uniform sampler2D texture_bloom_gui;

uniform vec2 resolution;

out vec4 out_color;



vec4 add_scanlines(vec4 current) {
    float line = 0.5+0.5*sin(frag_position.y * 1.4f + time * 2);
    line = line * 0.5f + 0.8f;
    return vec4(current.rgb * line, current.a);
}

vec3 shift_channels(vec3 current) {
    vec2 shift = vec2(1.0 / resolution);
    
    float red_channel   = texture(texture_result, frag_texcoord + shift).r;
    current.r += red_channel * 0.3f;

    return current;
}

void main() {
    vec2 tx = frag_texcoord;


    vec4 result = texture(texture_result, tx);
    vec3 bloom = texture(texture_bloom, tx).rgb;
    
    float bloom_intensivity = 0.22f;

    result += texture(texture_gui, tx);
    result.rgb += texture(texture_bloom, tx    ).rgb * bloom_intensivity;
    result.rgb += texture(texture_bloom_gui, tx).rgb * bloom_intensivity;

    result.rgb = shift_channels(result.rgb);
    result = add_scanlines(result);

    result.rgb = pow(result.rgb, vec3(1.0f / 2.2f)); 
    out_color = result;
}
