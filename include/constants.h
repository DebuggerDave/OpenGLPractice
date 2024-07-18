#ifndef CONSTANTS_H
#define CONSTANTS_H

#include "glm/vec3.hpp"
#include "glm/vec4.hpp"
#include "light_uniform_buffer.h"

// system
static constexpr unsigned int SCR_WIDTH = 1280;
static constexpr unsigned int SCR_HEIGHT = 720;

// rendering
static constexpr float FAR_PLANE = 256.0f;
static constexpr float NEAR_PLANE = 0.1f;
static constexpr glm::vec3 WORLD_UP = glm::vec3(0.0f, 1.0f, 0.0f);
static constexpr glm::vec4 CLEAR_COLOR = glm::vec4(0.2f, 0.3f, 0.3f, 1.0f);
static constexpr glm::vec4 COLOR_BLACK = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
static const LightColor LIGHT_BLACK{COLOR_BLACK, COLOR_BLACK, COLOR_BLACK};

#endif