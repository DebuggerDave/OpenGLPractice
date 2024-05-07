#ifndef LIGHT_DATA_H
#define LIGHT_DATA_H

#ifdef __cplusplus
#include "glm/glm.hpp"
#define VEC4 glm::vec4
#else
#define VEC4 vec4
#endif

struct LightColor {
#ifdef __cplusplus
    LightColor();
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

#endif