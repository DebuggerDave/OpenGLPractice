#ifndef SHADER_MACROS_H
#define SHADER_MACROS_H

#ifdef __cplusplus
#define VEC4 glm::vec4
#define LIGHTBLOCKQUALIFIER struct
#else
#define VEC4 vec4
#define LIGHTBLOCKQUALIFIER layout (std140) uniform
#endif

#define NUM_LIGHTS 3

struct Light {
    VEC4 dir_pos;
    VEC4 ambient;
    VEC4 diffuse;
    VEC4 specular;
};
LIGHTBLOCKQUALIFIER LightBlock {
    Light light[NUM_LIGHTS];
};

#endif