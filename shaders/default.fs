#version 330

// Input vertex attributes (from vertex shader)
in vec3 fragPosition;
in vec2 fragTexCoord;
in vec4 fragColor;
in vec3 fragNormal;

// Input uniform values
uniform sampler2D texture0;
uniform vec4 colDiffuse;

// Output fragment color
out vec4 finalColor;

uniform vec3 sun;


vec3 ambient = vec3(0.3, 0.1, 0.05);

void main()
{
    float d = dot(
            normalize(fragNormal),
            normalize(sun));
    d = clamp(d, 0.0, 1.0);

    vec3 col = vec3(1.0, 0.3, 0.1) * d;

    finalColor = vec4(col + ambient, 1.0);
}
