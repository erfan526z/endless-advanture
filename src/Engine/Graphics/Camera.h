#ifndef ENGINE_GRAPHICS_CAMERA_H
#define ENGINE_GRAPHICS_CAMERA_H

#include "GraphicsConfig.h"
#include "Input.h"

class Camera {
private:
	glm::vec3 position;
	glm::vec3 rotation;
	glm::vec3 cameraFront;
	glm::vec3 cameraUp;
	glm::mat4 projection;
	glm::mat4 view;
	float aspect_ratio;
	float far_plane;
	float near_plane;
	float fov_degrees;
	bool view_updated;
	bool projection_updated;

public:
	Camera(float x, float y, float z, float rx = 0.0f, float ry = 0.0f, float rz = 0.0f) {
		position = glm::vec3(x, y, z);
		rotation = glm::vec3(rx, ry, rz);
		view_updated = true;
		projection_updated = true;
		near_plane = 0.1f;
		far_plane = 500.0f;
		fov_degrees = 70.0f;
		aspect_ratio = 1.0f;
		cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
		cameraFront = glm::vec3();
		projection = glm::mat4();
		view = glm::mat4();
	}

	Camera() {
		position = glm::vec3(0.0f);
		rotation = glm::vec3(0.0f, -90.0f, 0.0f);
		view_updated = true;
		projection_updated = true;
		near_plane = 0.1f;
		far_plane = 500.0f;
		fov_degrees = 70.0f;
		aspect_ratio = 1.0f;
		cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
		cameraFront = glm::vec3();
		projection = glm::mat4();
		view = glm::mat4();
	}

	void updateAspectRatio(float aspect_ratio) {
		this->aspect_ratio = aspect_ratio;
		projection_updated = true;
	}

	void updateProjectionValues(float near_plane, float far_plane, float fov_degrees) {
		this->near_plane = near_plane;
		this->far_plane = far_plane;
		this->fov_degrees = fov_degrees;
		projection_updated = true;
	}

	void setPosition(float x, float y, float z) {
		position = glm::vec3(x, y, z);
		view_updated = true;
	}

	void setRotation(float rx, float ry, float rz) {
		rotation = glm::vec3(rx, ry, rz);
		view_updated = true;
	}

	void move(float dx, float dy, float dz) {
		position.x += dx;
		position.y += dy;
		position.z += dz;
		view_updated = true;
	}

	void rotate(float rx, float ry, float rz) {
		rotation.x += rx;
		rotation.y += ry;
		rotation.z += rz;
		view_updated = true;
	}

	void clampedRotate(float rx, float ry) {
		rotation.x += rx;
		rotation.y += ry;

		if (rotation.x > 89.0f)
			rotation.x = 89.0f;
		if (rotation.x < -89.0f)
			rotation.x = -89.0f;
		if (rotation.y > 180.0f)
			rotation.y -= 360.0f;
		if (rotation.y < -180.0f)
			rotation.y += 360.0f;
		view_updated = true;
	}

	glm::vec3 getPosition() {
		return position;
	}

	glm::vec3 getRotation() {
		return rotation;
	}

	glm::vec3 getLookingVector() {
		return cameraFront;
	}

	float getYawRadians() {
		return glm::radians(rotation.y);
	}

	glm::mat4 getViewMatrix() {

		if (view_updated) {
			glm::vec3 direction;
			direction.x = cos(glm::radians(rotation.y)) * cos(glm::radians(rotation.x));
			direction.y = sin(glm::radians(rotation.x));
			direction.z = sin(glm::radians(rotation.y)) * cos(glm::radians(rotation.x));
			cameraFront = glm::normalize(direction);
			view = glm::lookAt(position, position + cameraFront, cameraUp);
			view_updated = false;
		}
		return view;
	}

	glm::mat4 getProjectionMatrix() {
		if (projection_updated) {
			projection = glm::perspective(glm::radians(fov_degrees), aspect_ratio, near_plane, far_plane);
			projection_updated = false;
		}
		return projection;
	}
};

inline void inGameCursorPositionCallback(GLFWwindow* window, double xpos, double ypos) {
	static double lastX = xpos;
	static double lastY = ypos;

	if (_mouse_captured && _ptr_active_camera) {
		double offsetX = xpos - lastX;
		double offsetY = ypos - lastY;

		float sensitivity = 0.1f;

		((Camera*)_ptr_active_camera)->clampedRotate(-offsetY * sensitivity, offsetX * sensitivity);
	}

	lastX = xpos;
	lastY = ypos;
}

#endif
