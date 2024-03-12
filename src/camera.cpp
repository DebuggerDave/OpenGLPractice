#include "camera.h"

Camera::Camera(const glm::vec3& position, const float yaw, const float pitch) :
	movement_speed(default_speed),
	mouse_sensitivity(default_sensitivity),
	zoom(default_zoom),
	position(position),
	yaw(yaw),
	pitch(pitch)
{
	updateCameraVectors();
}

Camera::Camera(const float x, const float y, const float z, const float yaw, const float pitch) :
	movement_speed(default_speed),
	mouse_sensitivity(default_sensitivity),
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

void Camera::processMovement(const Movement direction, const float delta_time)
{
	float velocity = movement_speed * delta_time;
	if (direction == moveForward)
		position += front * velocity;
	if (direction == moveBackward)
		position -= front * velocity;
	if (direction == moveLeft)
		position -= right * velocity;
	if (direction == moveRight)
		position += right * velocity;
	if (direction == moveUp)
		position += up * velocity;
	if (direction == moveDown)
		position -= up * velocity;
}

void Camera::processRotation(const float x_offset, const float y_offset, const GLboolean constrain_pitch)
{
	yaw += x_offset * mouse_sensitivity;
	pitch += y_offset * mouse_sensitivity;

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