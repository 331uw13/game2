#version 330

in vec2 frag_texcoord;
in vec4 frag_color;
in vec3 frag_normal;
in vec3 frag_position;

uniform float time;
uniform sampler2D texture_result;
uniform sampler2D texture_bloom;

out vec4 out_color;


void main() {
    vec2 tx = frag_texcoord;


    vec4 result = texture(texture_result, tx);
    vec3 bloom = texture(texture_bloom, tx).rgb;


    float off = 0.00125f;
    vec3 red_channel = texture(texture_result, tx+vec2(off, 0)).rgb * vec3(1, 0, 0);
    vec3 cyan_channel = texture(texture_result, tx+vec2(off, 0)).rgb * vec3(0, 1, 1);

    result.r *= 0.5f;
    result.rgb += red_channel;

    result.gb *= 0.5;
    result.rgb += cyan_channel;
    result.rgb += bloom;

    float line = 0.5+0.5*sin(frag_position.y*1.5 - time * 8);
    line = line * 0.2 + 0.7;
    result *= line;

    out_color = result;
}
