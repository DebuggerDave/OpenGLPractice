#include "camera.h"

#include "constants.h"

#include "glm/vec3.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "glm/geometric.hpp"
#include "glm/trigonometric.hpp"
#include "glad/gl.h"

Camera::Camera(const glm::vec3& position, const float yaw, const float pitch) noexcept :
	position(position),
	yaw(yaw),
	pitch(pitch)
{
	updateCameraVectors();
}

Camera::Camera(const float x, const float y, const float z, const float yaw, const float pitch) noexcept :
	Camera(glm::vec3(x, y, z), yaw, pitch)
{}

glm::mat4 Camera::getViewMatrix() const
{
	return glm::lookAt(position, position + front, up);
}

void Camera::processMovement(const Movement direction, const float velocity)
{
	if (direction == Movement::Forward)
		position += front * velocity;
	if (direction == Movement::Backward)
		position -= front * velocity;
	if (direction == Movement::Left)
		position -= right * velocity;
	if (direction == Movement::Right)
		position += right * velocity;
	if (direction == Movement::Up)
		position += up * velocity;
	if (direction == Movement::Down)
		position -= up * velocity;
}

void Camera::processMouseRotation(const float x_offset, const float y_offset, const bool constrain_pitch)
{
	processRotation(x_offset, y_offset, static_cast<GLboolean>(constrain_pitch), mouse_sensitivity);
}

void Camera::processJoystickRotation(const float x_offset, const float y_offset, const bool constrain_pitch)
{
	processRotation(x_offset, y_offset, static_cast<GLboolean>(constrain_pitch), joystick_sensitivity);
}

void Camera::processZoom(float y_offset)
{
	if (zoom >= 1.0f && zoom <= 45.0f)
		zoom -= y_offset;
	if (zoom <= 1.0f)
		zoom = 1.0f;
	if (zoom >= 45.0f)
		zoom = 45.0f;
}

glm::vec3 Camera::getPosition() const
{
	return position;
}

glm::vec3 Camera::getFront() const
{
	return front;
}

float Camera::getZoom() const
{
	return zoom;
}

void Camera::updateCameraVectors()
{
	// Calculate the new Front vector
	glm::vec3 new_front;
	new_front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	new_front.y = sin(glm::radians(pitch));
	new_front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	front = glm::normalize(new_front);
	// Also re-calculate the Right and Up vector
	right = glm::normalize(glm::cross(front, WORLD_UP));
	up = glm::normalize(glm::cross(right, front));
}

void Camera::processRotation(const float x_offset, const float y_offset, const bool constrain_pitch, const float sensitivity)
{
	yaw += x_offset * sensitivity;
	pitch += y_offset * sensitivity;

	// Make sure that when pitch is out of bounds, screen doesn't get flipped
	if (constrain_pitch)
	{
		if (pitch > 89.0f)
			pitch = 89.0f;
		if (pitch < -89.0f)
			pitch = -89.0f;
	}

	// Update Front, Right and Up Vectors using the updated Euler angles
	updateCameraVectors();
}