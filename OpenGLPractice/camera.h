#ifndef CAMERA_H
#define CAMERA_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vector>

// Process input and Calculate the corresponding Euler Angles, Vectors and Matrices for use in OpenGL
class Camera
{
public:
	// Defines several possible options for camera movement.
	// Used as abstraction to stay away from window-system specific input methods
	enum Movement {
		moveForward,
		moveBackward,
		moveLeft,
		moveRight
	};

	// Camera Attributes
	glm::vec3 position;
	glm::vec3 front;
	glm::vec3 up;
	glm::vec3 right;
	// Euler Angles
	float yaw;
	float pitch;
	// Camera options
	float movementSpeed;
	float mouseSensitivity;
	float zoom;

	// Constructor with vectors
	Camera(glm::vec3 position=glm::vec3(0.0f, 0.0f, 0.0f), float yaw=defaultYaw, float pitch=defaultPitch);
	// Constructor with scalar values
	Camera(float x=0, float y=0, float z=0, float yaw=defaultYaw, float pitch=defaultPitch);

	// Returns the view matrix calculated using Euler Angles and the LookAt Matrix
	glm::mat4 getViewMatrix();

	// Processes input received from any keyboard-like input system. Accepts input parameter in the form of camera defined ENUM (to abstract it from windowing systems)
	void processMovement(Movement direction, float deltaTime);

	// Processes input received from a mouse input system. Expects the offset value in both the x and y direction.
	void processRotation(float xoffset, float yoffset, GLboolean constrainPitch=true);

	// Processes input received from a mouse scroll-wheel event. Only requires input on the vertical wheel-axis
	void processZoom(float yoffset);

private:
	// default values
	static const float defaultPitch;
	static const float defaultYaw;
	static const float defaultSpeed;
	static const float defaultSensitivity;
	static const float defaultZoom;
	static const glm::vec3 worldUp;

	// Calculates the front vector from the Camera's (updated) Euler Angles
	void updateCameraVectors();
};
#endif