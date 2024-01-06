#ifndef SHADER_MACROS_H
#define SHADER_MACROS_H

#ifdef __cplusplus
#define VEC4 glm::vec4
#define LIGHTBLOCKQUALIFIER struct
#else
#define VEC4 vec4
#define LIGHTBLOCKQUALIFIER layout (std140) uniform
#endif

#define NUM_LIGHTS 2
#define NUM_DIRECTIONAL_LIGHTS 1

struct Light {
    VEC4 dir;
    VEC4 pos;
    VEC4 ambient;
    VEC4 diffuse;
    VEC4 specular;
    //
    float inner_angle_cosine;
    float outer_angle_cosine;
    float constant;
    float linear;
    //
    float quadratic;
    float pad0;
    float pad1;
    float pad2;
};
struct Directional_Light {
    VEC4 dir;
    VEC4 ambient;
    VEC4 diffuse;
    VEC4 specular;   
};
LIGHTBLOCKQUALIFIER LightBlock {
    Light lights[NUM_LIGHTS];
    Directional_Light directional_lights[NUM_DIRECTIONAL_LIGHTS];
};

#endif