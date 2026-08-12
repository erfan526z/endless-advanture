#ifndef ENGINE_GRAPHICS_BASIC_ENTITY_H
#define ENGINE_GRAPHICS_BASIC_ENTITY_H

#include "GraphicsConfig.h"
#include "RawModel.h"
#include "Texture.h"

class BasicEntity {
private:
	RawModel* model;
	Texture* texture;
	glm::vec3 position;
	glm::vec3 rotation;
	glm::vec3 scale;
	glm::mat4 transformation;
	bool local_updated;
	bool internal;
	int atlas;
	float atlas_y_multiplyer;

public:

	BasicEntity(const char* model_path, const char* texture_path) {
		model = new RawModel();
		texture = new Texture();
		texture->loadTexture(texture_path);
		model->initFromFile(model_path);
		position = glm::vec3();
		rotation = glm::vec3();
		scale = glm::vec3(1.0f);
		local_updated = true;
		internal = true;
		transformation = glm::mat4();
		atlas = 0;
		atlas_y_multiplyer = 1;
	}

	BasicEntity(Texture* texture, RawModel* model) {
		this->model = model;
		this->texture = texture;
		position = glm::vec3();
		rotation = glm::vec3();
		scale = glm::vec3(1.0f);
		local_updated = true;
		internal = false;
		transformation = glm::mat4();
		atlas = 0;
		atlas_y_multiplyer = 1;
	}

	BasicEntity() {
		this->model = nullptr;
		this->texture = nullptr;
		position = glm::vec3();
		rotation = glm::vec3();
		scale = glm::vec3(1.0f);
		local_updated = true;
		internal = false;
		transformation = glm::mat4();
		atlas = 0;
		atlas_y_multiplyer = 1;
	}

	void init(Texture* texture, RawModel* model) {
		this->model = model;
		this->texture = texture;
	}

	void setPosition(float x, float y, float z) {
		position = glm::vec3(x, y, z);
		local_updated = true;
	}

	void setRotation(float rx, float ry, float rz) {
		rotation = glm::vec3(rx, ry, rz);
		local_updated = true;
	}

	void setScale(float sx, float sy, float sz) {
		scale = glm::vec3(sx, sy, sz);
		local_updated = true;
	}

	void move(float dx, float dy, float dz) {
		position.x += dx;
		position.y += dy;
		position.z += dz;
		local_updated = true;
	}

	void rotate(float drx, float dry, float drz) {
		rotation.x += drx;
		rotation.y += dry;
		rotation.z += drz;

		if (rotation.x > 180.0f)
			rotation.x -= 360.0f;
		if (rotation.x < -180.0f)
			rotation.x += 360.0f;
		if (rotation.y > 180.0f)
			rotation.y -= 360.0f;
		if (rotation.y < -180.0f)
			rotation.y += 360.0f;
		if (rotation.z > 180.0f)
			rotation.z -= 360.0f;
		if (rotation.z < -180.0f)
			rotation.z += 360.0f;

		local_updated = true;
	}

	glm::vec3 getPosition() {
		return position;
	}

	glm::vec3 getRotation() {
		return rotation;
	}

	glm::vec3 getScale() {
		return scale;
	}

	glm::mat4 getTransformationMatrix() {
		if (local_updated) {
			transformation = glm::mat4(1.0f);
			transformation = glm::translate(transformation, position);
			transformation = glm::rotate(transformation, glm::radians(rotation.x), glm::vec3(1.0, 0.0, 0.0));
			transformation = glm::rotate(transformation, glm::radians(rotation.y), glm::vec3(0.0, 1.0, 0.0));
			transformation = glm::rotate(transformation, glm::radians(rotation.z), glm::vec3(0.0, 0.0, 1.0));
			transformation = glm::scale(transformation, scale);
			local_updated = false;
		}

		return transformation;
	}

	RawModel* getModel() {
		return model;
	}

	Texture* getTexture() {
		return texture;
	}

	int getAtlasIndex() {
		return atlas;
	}

	void setAtlasIndex(int index) {
		atlas = index;
	}

	float& atlasYMultiplyer() {
		return atlas_y_multiplyer;
	}

	~BasicEntity() {
		if (internal) {
			texture->unloadTexture();
			model->destroyModel();
			delete texture;
			delete model;
		}
	}
};

#endif
