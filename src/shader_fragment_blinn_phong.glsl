#version 330 core

in vec4 position_world;
in vec4 normal;
in vec2 texcoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform vec4 bbox_min;
uniform vec4 bbox_max;

// Material parameters
uniform bool has_kd_texture;
uniform bool has_ke_texture;
uniform bool has_opacity_texture;
uniform sampler2D texture_sampler_kd;
uniform sampler2D texture_sampler_ke;
uniform sampler2D texture_sampler_opacity;

uniform vec3 kd; // Diffuse/albedo
uniform vec3 ks; // Specular color
uniform vec3 ke; // Emissive color

uniform float ns;      // Shininess
uniform float opacity; // Transparency

#define AMBIENT 0.1;

out vec4 color;

void main()
{
    // Camera position in world space
    vec4 origin = vec4(0.0, 0.0, 0.0, 1.0);
    vec4 camera_position = inverse(view) * origin;

    // Fragment world position
    vec3 p = position_world.xyz;

    // Surface normal
    vec3 n = normalize(normal.xyz);

    // Directional light (sun)
    vec3 l = normalize(vec3(1.0, 1.0, 0.2));

    // View direction
    vec3 v = normalize(camera_position.xyz - p);

    // Blinn-Phong half vector
    vec3 h = normalize(l + v);

    // Base color (texture * material diffuse)
    vec3 albedo = has_kd_texture ? texture(texture_sampler_kd, texcoords).rgb * kd : kd;

    // Ambient lighting
    float ambient_strength = AMBIENT;
    vec3 ambient = ambient_strength * albedo;

    // Diffuse Lambert term
    float lambert = max(dot(n, l), 0.0);
    vec3 diffuse = lambert * albedo;

    // Blinn-Phong specular
    float specular_factor = 0.0;

    if(lambert > 0.0)
    {
        specular_factor = pow(max(dot(n, h), 0.0), ns);
    }

    vec3 specular = ks * specular_factor;

    // Emissive
    vec3 emissive = has_ke_texture ? texture(texture_sampler_ke, texcoords).rgb * ke : ke;

    // Final lighting
    vec3 final_color =
        ambient +
        diffuse +
        specular +
        emissive;

    // Gamma correction
    final_color = pow(final_color, vec3(1.0 / 2.2));

    color.rgb = final_color;
    color.a = has_opacity_texture ? texture(texture_sampler_opacity, texcoords).r * opacity : opacity;
}