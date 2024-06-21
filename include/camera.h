#ifndef CAMERA_H
#define CAMERA_H

#include "glm/ext/vector_float3.hpp"
#include "glm/fwd.hpp"
struct GLFWwindow;

// Process input and Calculate the corresponding Euler Angles, Vectors and Matrices for use in OpenGL
class Camera
{
public:
	enum class Movement {
		Forward,
		Backward,
		Left,
		Right,
		Up,
		Down
	};

	Camera(const glm::vec3& position, const float yaw=default_yaw, const float pitch=default_pitch);
	Camera(const float x=0, const float y=0, const float z=0, const float yaw=default_yaw, const float pitch=default_pitch);

	// Returns the view matrix calculated using Euler Angles and the LookAt Matrix
	glm::mat4 getViewMatrix() const;
	// Move in direction indicated by direction param
	void processMovement(const Movement direction, const float delta_time);
	// Process rotation with mouse parameters
	void processMouseRotation(const float x_offset, const float y_offset, const bool constrain_pitch=true);
	// Process rotation with joystick parameters
	void processJoystickRotation(const float x_offset, const float y_offset, const bool constrain_pitch=true);
	// Processes input received from a mouse scroll-wheel event. Only requires input on the vertical wheel-axis
	void processZoom(const float y_offset);

	// Getters
	glm::vec3 getPosition() const;
	glm::vec3 getFront() const;
	float getZoom() const;

private:
	static const float default_mouse_sensitivity;
	static const float default_joystick_sensitivity;
	static const float default_zoom;
	static const float default_pitch;
	static const float default_yaw;
	static const glm::vec3 world_up;
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
	void processRotation(const float x_offset, const float y_offset, const bool constrain_pitch, const float sensitivity);
};
#endif