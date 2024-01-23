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
	enum Movement {
		moveForward,
		moveBackward,
		moveLeft,
		moveRight,
		moveUp,
		moveDown
	};

	Camera(const glm::vec3& position=glm::vec3(0.0f, 0.0f, 0.0f), const float yaw=defaultYaw, const float pitch=defaultPitch);
	Camera(const float x=0, const float y=0, const float z=0, const float yaw=defaultYaw, const float pitch=defaultPitch);

	// Returns the view matrix calculated using Euler Angles and the LookAt Matrix
	glm::mat4 getViewMatrix() const;
	// make in direction indicated by direction param
	void processMovement(const Movement direction, const float deltaTime);
	// Processes input received from a mouse input system. Expects the offset value in both the x and y direction.
	void processRotation(const float xoffset, const float yoffset, const GLboolean constrainPitch=true);
	// Processes input received from a mouse scroll-wheel event. Only requires input on the vertical wheel-axis
	void processZoom(const float yoffset);

	glm::vec3 getPosition() const;
	glm::vec3 getFront() const;
	float getZoom() const;

private:
	// default values
	inline static const float defaultSpeed = 2.5f;
	inline static const float defaultSensitivity = 0.1f;
	inline static const float defaultZoom = 45.0f;
	inline static const float defaultPitch = 0.0f;
	inline static const float defaultYaw = -90.0f;
	inline static const glm::vec3 worldUp = glm::normalize(glm::vec3(0.0f, 1.0f, 0.0f));
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

	// Calculates the front vector from the Camera's (updated) Euler Angles
	void updateCameraVectors();
};
#endif