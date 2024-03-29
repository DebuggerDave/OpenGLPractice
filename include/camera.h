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

	Camera(const glm::vec3& position=glm::vec3(0.0f, 0.0f, 0.0f), const float yaw=default_yaw, const float pitch=default_pitch);
	Camera(const float x=0, const float y=0, const float z=0, const float yaw=default_yaw, const float pitch=default_pitch);

	// Returns the view matrix calculated using Euler Angles and the LookAt Matrix
	glm::mat4 getViewMatrix() const;
	// Move in direction indicated by direction param
	void processMovement(const Movement direction, const float delta_time);
	// Process rotation with mouse parameters
	void processMouseRotation(const float x_offset, const float y_offset, const GLboolean constrain_pitch=true);
	// Process rotation with joystick parameters
	void processJoystickRotation(const float x_offset, const float y_offset, const GLboolean constrain_pitch=true);
	// Processes input received from a mouse scroll-wheel event. Only requires input on the vertical wheel-axis
	void processZoom(const float y_offset);

	// Getters
	glm::vec3 getPosition() const;
	glm::vec3 getFront() const;
	float getZoom() const;

private:
	// default values
	inline static const float default_mouse_sensitivity = 0.1f;
	inline static const float default_joystick_sensitivity = 1.0f;
	inline static const float default_zoom = 45.0f;
	inline static const float default_pitch = 0.0f;
	inline static const float default_yaw = -90.0f;
	inline static const glm::vec3 world_up = glm::normalize(glm::vec3(0.0f, 1.0f, 0.0f));
	// Camera Attributes
	glm::vec3 position;
	glm::vec3 front;
	glm::vec3 up;
	glm::vec3 right;
	// Euler Angles
	float yaw;
	float pitch;
	// Camera options
	float mouse_sensitivity;
	float joystick_sensitivity;
	float zoom;

	// Calculates the front vector from the Camera's (updated) Euler Angles
	void updateCameraVectors();
	// Adjust yaw and pitch based on x and y movement
	void processRotation(const float x_offset, const float y_offset, const GLboolean constrain_pitch, const float sensitivity);
};
#endif