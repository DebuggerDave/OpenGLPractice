#ifndef SHADER_LIGHT_BLOCK_H
#define SHADER_LIGHT_BLOCK_H

#define NUM_DIRECTIONAL_LIGHTS 0
#define NUM_SPOT_LIGHTS 0
#define NUM_POINT_LIGHTS 0
#define UNIFORM_BUFFER_TYPE LightBlockData

#ifdef __cplusplus
#include <vector>
#include <glm/glm.hpp>
#define VEC4 glm::vec4
#define LIGHT_BLOCK_QUALIFIER struct
#define CONTAINER(type, name, size) std::vector<type> name
#else
#define VEC4 vec4
#define LIGHT_BLOCK_QUALIFIER layout (std140) uniform
#define CONTAINER(type, name, size) type name[size]
#endif

LIGHT_BLOCK_QUALIFIER UNIFORM_BUFFER_TYPE {
    CONTAINER(DirectionalLight, directional_lights, NUM_DIRECTIONAL_LIGHTS);
    CONTAINER(SpotLight, spot_lights, NUM_SPOT_LIGHTS);
    CONTAINER(PointLight, point_lights, NUM_POINT_LIGHTS);
};

#endif