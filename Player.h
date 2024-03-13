#pragma once

#include "ChunkConstants.h"
#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>

class PhysicalPlayer {

public:

	void setPositionOn(int x, int y, int z) {
		xChunk = getChunkNumber(x);
		zChunk = getChunkNumber(z);
		xLocal = (float)(x - xChunk * CHUNK_SIZE) + 0.5f;
		zLocal = (float)(z - zChunk * CHUNK_SIZE) + 0.5f;
		this->y = y;
	}

	void move(float x, float y, float z) {
		xLocal += x;
		if (xLocal >= (float)CHUNK_SIZE) {
			xLocal -= (float)CHUNK_SIZE;
			xChunk++;
		}
		if (xLocal < 0.0f) {
			xLocal += (float)CHUNK_SIZE;
			xChunk--;
		}

		zLocal += z;
		if (zLocal >= (float)CHUNK_SIZE) {
			zLocal -= (float)CHUNK_SIZE;
			zChunk++;
		}
		if (zLocal < 0.0f) {
			zLocal += (float)CHUNK_SIZE;
			zChunk--;
		}

		this->y += y;
	}

	glm::vec3 getLocalPosition() {
		return glm::vec3(xLocal, y, zLocal);
	}

	glm::vec3 getLocalEyePosition() {
		return glm::vec3(xLocal, y + eye_height, zLocal);
	}

	glm::vec3 getLookingVector() {
		return view_front;
	}

	float getYaw() {
		return yaw;
	}

	float getPitch() {
		return pitch;
	}

	int getGlobalX() {
		return xChunk * CHUNK_SIZE + (int)xLocal;
	}

	int getGlobalY() {
		return (int)y;
	}

	int getGlobalZ() {
		return zChunk * CHUNK_SIZE + (int)zLocal;
	}

	int getOffsetX() {
		return xChunk * CHUNK_SIZE;
	}

	int getOffsetZ() {
		return zChunk * CHUNK_SIZE;
	}

	int getChunkX() {
		return xChunk;
	}

	int getChunkZ() {
		return zChunk;
	}

	void clampedRotate(float rx, float ry) {
		pitch += rx;
		yaw += ry;

		if (pitch >= 89.999f)
			pitch = 89.999f;
		if (pitch <= -89.999f)
			pitch = -89.999f;
		if (yaw > 180.0f)
			yaw -= 360.0f;
		if (yaw < -180.0f)
			yaw += 360.0f;
	}

	void setRotation(float pitch, float yaw) {
		this->pitch = pitch;
		this->yaw = yaw;
	}

	void setBodyDimensions(float eye_h, float body_w, float body_h) {
		body_height = body_h;
		body_width = body_w;
		eye_height = eye_h;
	}

	float getEyeHeigth() {
		return eye_height;
	}

	float getBodyWidth() {
		return body_width;
	}

	float getBodyHeight() {
		return body_height;
	}

	glm::mat4 getViewMatrix()
	{
		glm::vec3 direction;
		direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
		direction.y = sin(glm::radians(pitch));
		direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
		glm::vec3 pos_with_eye = glm::vec3(xLocal, y + eye_height, zLocal);
		view_front = glm::normalize(direction);
		glm::mat4 view = glm::lookAt(pos_with_eye, pos_with_eye + view_front, view_up);
		return view;
	}

private:

	int xChunk = 0;
	int zChunk = 0;
	float xLocal = 0.5f;
	float zLocal = 0.5f;
	float y = 64.0f;
	float yaw = 0.0f;
	float pitch = 0.0f;
	float eye_height = 1.65f;
	float body_width = 0.6f;
	float body_height = 1.8f;

	glm::vec3 view_up = glm::vec3(0.0f, 1.0f, 0.0f);
	glm::vec3 view_front = glm::vec3(1.0f, 0.0f, 0.0f);

	int getChunkNumber(int v) { return (v < 0) ? ((v + 1) / CHUNK_SIZE - 1) : (v / CHUNK_SIZE); }

};