#version 330 core

in vec4 position_world;
in vec4 normal;
in vec2 texcoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec3 camera_position;

uniform bool has_kd_texture;
uniform bool has_ke_texture;
uniform bool has_opacity_texture;
uniform sampler2D texture_sampler_kd;
uniform sampler2D texture_sampler_ke;
uniform sampler2D texture_sampler_opacity;

uniform vec3 kd;
uniform vec3 ks;
uniform vec3 ke;
uniform float ns;
uniform float opacity;

#define AMBIENT_DIFFUSE 0.2
#define AMBIENT_SPECULAR 0.35
#define SUN_INTENSITY 6.0      // Tweak this (6.0 - 8.5 range)
#define PI 3.14159265359

out vec4 color;

// === PBR Functions ===
float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;
    float denom = NdotH2 * (a2 - 1.0) + 1.0;
    return a2 / (PI * denom * denom);
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = roughness + 1.0;
    float k = (r * r) / 8.0;
    return NdotV / (NdotV * (1.0 - k) + k);
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    return GeometrySchlickGGX(NdotV, roughness) * GeometrySchlickGGX(NdotL, roughness);
}

vec3 FresnelSchlick(vec3 H, vec3 V, vec3 F0)
{
    float cosTheta = max(dot(H, V), 0.0);
    return F0 + (1.0 - F0) * pow(max(1.0 - cosTheta, 0.0), 5.0);
}

void main()
{
    vec3 p = position_world.xyz;
    vec3 n = normalize(normal.xyz);
    vec3 l = normalize(vec3(0.0, 1.0, 0.0));     // Nice top-down sun
    vec3 v = normalize(camera_position.xyz - p);
    vec3 h = normalize(l + v);

    vec3 albedo = has_kd_texture ? texture(texture_sampler_kd, texcoords).rgb * kd : kd;
    vec3 emissive = has_ke_texture ? texture(texture_sampler_ke, texcoords).rgb * ke : ke;

    // Cleaner conversion (works well with your updated MTL)
    float shininess = max(ns, 10.0);
    float roughness = clamp(1.0 - sqrt(shininess / (shininess + 35.0)), 0.04, 0.95);

    float specularIntensity = length(ks);
    float metallic = clamp(specularIntensity * 2.4, 0.0, 1.0);

    // Boost metallic on very bright + high shininess materials (Chrome, Rims, etc.)
    if (shininess > 700.0 && specularIntensity > 0.6)
        metallic = max(metallic, 0.85);

    vec3 F0 = mix(vec3(0.025), albedo, metallic);

    // BRDF
    float NdotL = max(dot(n, l), 0.0);
    float NdotV = max(dot(n, v), 0.0);

    vec3 F = FresnelSchlick(h, v, F0);
    float D = DistributionGGX(n, h, roughness);
    float G = GeometrySmith(n, v, l, roughness);

    vec3 specular = (D * G * F) / max(4.0 * NdotV * NdotL, 0.001);

    vec3 kS = F;
    vec3 kD = (vec3(1.0) - kS) * (1.0 - metallic);

    vec3 Lo = (kD * albedo / PI + specular) * SUN_INTENSITY * NdotL;

    vec3 ambientDiffuse = AMBIENT_DIFFUSE * albedo * (1.0 - metallic);
    vec3 ambientSpecular = AMBIENT_SPECULAR * F0 * (0.5 + 0.5 * metallic);

    vec3 ambient = ambientDiffuse + ambientSpecular;

    vec3 final_lighting = ambient + Lo + emissive;

    // Fog
    float dist = length(p - camera_position.xyz);
    float fog_factor = min(pow(dist / 85.0, 2.0), 1.0);
    vec3 fog_color = vec3(0.65, 0.65, 0.65);
    vec3 final_color = final_lighting * (1.0 - fog_factor) + fog_color * fog_factor;

    final_color = pow(final_color, vec3(1.0 / 2.2));

    color.rgb = final_color;
    color.a = has_opacity_texture ? texture(texture_sampler_opacity, texcoords).r * opacity : opacity;
}