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
uniform vec3 camera;
uniform vec3 fog_color;


#define AMBIENT_LEVEL 0.03
vec3 compute_light
(
    vec3 pos, 
    vec3 color,
    float radius,
    float strength,
    float edge,
    float mat_specular,
    float mat_shine,
    vec3  mat_color
){
    vec3 light_dir   = normalize(pos - fragPosition);
    vec3 view_dir    = normalize(camera - fragPosition);
    vec3 halfway_dir = normalize(light_dir - view_dir);

    // Diffuse.
    float diff = max(dot(fragNormal, light_dir), 0.0);
    vec3 diffuse = diff * color;

    // Specular.
    float spec = pow(max(dot(view_dir, reflect(-light_dir, fragNormal)), 0.0), mat_specular);
    vec3 specular = spec * mix(vec3(1.0, 1.0, 1.0), color, 0.6);
    specular *= mat_shine;

    // Attenuation.
    float L = 0.8;
    float Q = 2.3;
    float dist = distance(fragPosition, pos) / radius;
    dist = pow(dist, edge);
    float a = 1.0 / (2.0 + L * dist + Q * (dist * dist));
    a = clamp(a, 0.0, 1.0);
        
    // Ambient.
    vec3 ambient = AMBIENT_LEVEL * mix(color, mat_color, 1.0-a);
    diffuse *= a;
    specular *= a;


    return diffuse + specular + ambient;
}


float get_fog() {
    float density = 0.00018;
    float dist = density * distance(camera, fragPosition);
    float t = 1.0/exp(dist * dist);
    return t;
}

/*
vec3 compute_sun() {
    float d = 1.0 - dot(
            normalize(fragNormal),
            normalize(sun));
    d = clamp(d, 0.0, 1.0);

    return (vec3(1.0, 0.9, 0.8) * d) * 0.01;
}
*/


void main()
{

    vec3  light_pos = camera;
    vec3  light_color = vec3(0.3, 0.9, 1.0);
    float light_radius = 30.0;
    float light_strength = 1.0;
    float light_edge = 0.01;
    float mat_specular = 1.0;
    float mat_shine = 0.1;

    vec3  mat_color = vec3(1.0, 0.8, 0.2);

    vec3 col = compute_light(light_pos, light_color, light_radius, light_strength, light_edge, mat_specular, mat_shine, mat_color);
   

    col = mix(fog_color, col, get_fog());
    finalColor = vec4(col, 1.0);
}




