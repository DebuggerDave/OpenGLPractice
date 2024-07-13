#ifndef CONSTANTS_H
#define CONSTANTS_H

// system
static const unsigned int SCR_WIDTH = 1280;
static const unsigned int SCR_HEIGHT = 720;
static const unsigned int SHADOW_RESOLUTION = 1024;
static const unsigned int SHADOW_WIDTH = SHADOW_RESOLUTION;
static const unsigned int SHADOW_HEIGHT = SHADOW_RESOLUTION;

// rendering
static const float SHADOW_NEAR_PLANE = 5.0f;
static const float FAR_PLANE = 256.0f;
static const float NEAR_PLANE = 0.1f;

// controls
static const bool USE_MOUSE = false;
static const bool USE_GAMEPAD = true;
static const float BASE_SPEED = 2.5f;
static const float MAX_SPEED = BASE_SPEED * 10.0f;
static const float MIN_DEAD_ZONE = 0.1f;
static const float JOYSTICK_SENSITIVITY = 3.0f;

// time (in seconds)
static const int MINUTE = 60;
static const int HOUR = MINUTE * 60;
static const int DAY = HOUR * 24;
static const int HALF_DAY = DAY / 2.0f;

// world generation
static const float BLOCK_HALF_LENGTH = .5;

#endif