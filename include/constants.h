#ifndef CONSTANTS_H
#define CONSTANTS_H

static const unsigned int SCR_WIDTH = 1280;
static const unsigned int SCR_HEIGHT = 720;
static const unsigned int SHADOW_RESOLUTION = 1024;
static const unsigned int SHADOW_WIDTH = SHADOW_RESOLUTION;
static const unsigned int SHADOW_HEIGHT = SHADOW_RESOLUTION;
static const float SHADOW_NEAR_PLANE = 5.0f;
static const float FAR_PLANE = 100.0f;
static const float NEAR_PLANE = 0.1f;
static const bool USE_MOUSE = false;
static const bool USE_GAMEPAD = true;
static const float BASE_SPEED = 2.5f;
static const float MAX_SPEED = BASE_SPEED * 3.0f;

#endif