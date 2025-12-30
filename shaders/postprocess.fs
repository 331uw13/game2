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
    result.rgb += bloom;


    float line = 0.5+0.5*sin(frag_position.y*1.6 - time * 8);
    line = line * 0.2 + 0.7;
    result *= line;

    out_color = result;
}
