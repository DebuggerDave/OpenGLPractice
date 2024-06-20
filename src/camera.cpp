#include "camera.h"

#include "pch.h"

const float Camera::default_mouse_sensitivity = 0.1f;
const float Camera::default_joystick_sensitivity = 1.0f;
const float Camera::default_zoom = 45.0f;
const float Camera::default_pitch = 0.0f;
const float Camera::default_yaw = -90.0f;
const glm::vec3 Camera::world_up = glm::normalize(glm::vec3(0.0f, 1.0f, 0.0f));

Camera::Camera(const glm::vec3& position, const float yaw, const float pitch) :
	mouse_sensitivity(default_mouse_sensitivity),
	joystick_sensitivity(default_joystick_sensitivity),
	zoom(default_zoom),
	position(position),
	yaw(yaw),
	pitch(pitch)
{
	updateCameraVectors();
}

Camera::Camera(const float x, const float y, const float z, const float yaw, const float pitch) :
	mouse_sensitivity(default_mouse_sensitivity),
	joystick_sensitivity(default_joystick_sensitivity),
	zoom(default_zoom),
	position(glm::vec3(x, y, z)),
	yaw(yaw),
	pitch(pitch)
{
	updateCameraVectors();
}

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
	processRotation(x_offset, y_offset, (GLboolean)constrain_pitch, mouse_sensitivity);
}

void Camera::processJoystickRotation(const float x_offset, const float y_offset, const bool constrain_pitch)
{
	processRotation(x_offset, y_offset, (GLboolean)constrain_pitch, joystick_sensitivity);
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
	right = glm::normalize(glm::cross(front, world_up));
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