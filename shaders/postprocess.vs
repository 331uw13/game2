#version 330


in vec3 vertexPosition;
in vec2 vertexTexcoord;
in vec3 vertexNormal;
in vec4 vertexColor;
uniform mat4 mvp;
uniform mat4 matModel;
uniform mat4 matNormal;
out vec2 frag_texcoord;
out vec4 frag_color;
out vec3 frag_normal;
out vec3 frag_position;

void main() {
    frag_texcoord = vertexTexcoord;
    frag_color = vertexColor;
    vec3 vertex_pos = vertexPosition;

        frag_position = vec3(matModel*vec4(vertex_pos, 1.0));
        frag_normal = normalize(vec3(matNormal * vec4(vertexNormal, 1.0)));
        gl_Position = mvp * vec4(vertex_pos, 1.0);
}
