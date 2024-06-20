#ifndef SHADER_LIGHT_BLOCK_H
#define SHADER_LIGHT_BLOCK_H

#define NUM_DIRECTIONAL_LIGHTS 0
#define NUM_SPOT_LIGHTS 0
#define NUM_POINT_LIGHTS 0
#define UNIFORM_BUFFER_TYPE LightBlockData

#ifdef __cplusplus
#include <vector>
#include "glm/ext/vector_float4.hpp"
#define VEC4 glm::vec4
#define LIGHT_BLOCK_QUALIFIER struct
#define CONTAINER(type, name, size) std::vector<type> name
#else
#define VEC4 vec4
#define LIGHT_BLOCK_QUALIFIER layout (std140) uniform
#define CONTAINER(type, name, size) type name[size]
#endif

struct LightColor {
#ifdef __cplusplus
    LightColor(const glm::vec4& ambient=glm::vec4(0.2f), const glm::vec4& diffuse=glm::vec4(1.0f), const glm::vec4& specular=glm::vec4(1.0f));
#endif
    VEC4 ambient;
    VEC4 diffuse;
    VEC4 specular;
};
struct Attenuation {
#ifdef __cplusplus
    Attenuation();
#endif
    float constant;
    float linear;
    float quadratic;
    float pad0;
};

struct DirectionalLight {
#ifdef __cplusplus
    DirectionalLight();
#endif
    LightColor color;
    VEC4 dir;
};

struct SpotLight {
#ifdef __cplusplus
    SpotLight();
#endif
    LightColor color;
    Attenuation attenuation;
    VEC4 dir;
    VEC4 pos;
    // ----
    float inner_angle_cosine;
    float outer_angle_cosine;
    float pad0;
    float pad1;
};

struct PointLight {
#ifdef __cplusplus
    PointLight();
#endif
    LightColor color;
    Attenuation attenuation;
    VEC4 pos;
};

LIGHT_BLOCK_QUALIFIER UNIFORM_BUFFER_TYPE {
    CONTAINER(DirectionalLight, directional_lights, NUM_DIRECTIONAL_LIGHTS);
    CONTAINER(SpotLight, spot_lights, NUM_SPOT_LIGHTS);
    CONTAINER(PointLight, point_lights, NUM_POINT_LIGHTS);
};

#endif