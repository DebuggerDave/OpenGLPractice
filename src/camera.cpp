#include "camera.h"

const float Camera::defaultSpeed = 2.5f;
const float Camera::defaultSensitivity = 0.1f;
const float Camera::defaultZoom = 45.0f;
const float Camera::defaultPitch = 0.0f;
const float Camera::defaultYaw = -90.0f;
const glm::vec3 Camera::worldUp(glm::normalize(glm::vec3(0.0f, 1.0f, 0.0f)));

Camera::Camera(glm::vec3 position, float yaw, float pitch) :
	movementSpeed(defaultSpeed),
	mouseSensitivity(defaultSensitivity),
	zoom(defaultZoom)
{
	this->position = position;
	this->yaw = yaw;
	this->pitch = pitch;
	updateCameraVectors();
}

Camera::Camera(float x, float y, float z, float yaw, float pitch) :
	movementSpeed(defaultSpeed),
	mouseSensitivity(defaultSensitivity),
	zoom(defaultZoom)
{
	this->position = glm::vec3(x, y, z);
	this->yaw = yaw;
	this->pitch = pitch;
	updateCameraVectors();
}

glm::mat4 Camera::getViewMatrix()
{
	return glm::lookAt(position, position + front, up);
}

void Camera::processMovement(Movement direction, float deltaTime)
{
	float velocity = movementSpeed * deltaTime;
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

void Camera::processRotation(float xoffset, float yoffset, GLboolean constrainPitch)
{
	xoffset *= mouseSensitivity;
	yoffset *= mouseSensitivity;

	yaw += xoffset;
	pitch += yoffset;

	// Make sure that when pitch is out of bounds, screen doesn't get flipped
	if (constrainPitch)
	{
		if (pitch > 89.0f)
			pitch = 89.0f;
		if (pitch < -89.0f)
			pitch = -89.0f;
	}

	// Update Front, Right and Up Vectors using the updated Euler angles
	updateCameraVectors();
}

void Camera::processZoom(float yoffset)
{
	if (zoom >= 1.0f && zoom <= 45.0f)
		zoom -= yoffset;
	if (zoom <= 1.0f)
		zoom = 1.0f;
	if (zoom >= 45.0f)
		zoom = 45.0f;
}

void Camera::updateCameraVectors()
{
	// Calculate the new Front vector
	glm::vec3 newFront;
	newFront.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	newFront.y = sin(glm::radians(pitch));
	newFront.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	front = glm::normalize(newFront);
	// Also re-calculate the Right and Up vector
	right = glm::normalize(glm::cross(front, worldUp));
	up = glm::normalize(glm::cross(right, front));
}