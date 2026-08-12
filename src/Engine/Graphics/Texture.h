#ifndef ENGINE_GRAPHICS_TEXTURE_H
#define ENGINE_GRAPHICS_TEXTURE_H

#include "GraphicsConfig.h"

class Texture {
private:
	unsigned int textureID;
	bool loaded;
	const char* path;
	int atlas_size_x;
	int atlas_size_y;

public:
	Texture(int atlas_x = 1, int atlas_y = 1) {
		textureID = 0;
		loaded = false;
		atlas_size_x = atlas_x;
		atlas_size_y = atlas_y;
		path = nullptr;
	}

	bool loadTexture(const char* path) {
		this->path = path;
		int width;
		int height;
		int nrChannels;
		unsigned char* data = stbi_load(path, &width, &height, &nrChannels, 0);
		//stbi_set_flip_vertically_on_load(1);

		if (data) {
			glGenTextures(1, &textureID);
			glBindTexture(GL_TEXTURE_2D, textureID);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
			glGenerateMipmap(GL_TEXTURE_2D);

			stbi_image_free(data);

			loaded = true;
			glBindTexture(GL_TEXTURE_2D, 0);
			return true;
		}
		else {
			std::cout << "Failed to load texture : " << path << std::endl;
		}
		//stbi_image_free(data);
		return false;
	}

	bool unloadTexture() {
		if (!loaded)
			return false;
		glDeleteTextures(1, &textureID);
		loaded = false;
		return true;
	}

	bool isLoaded() {
		return loaded;
	}

	unsigned int getID() {
		return textureID;
	}

	glm::vec4 getAtlasCoordinate(int i) {
		if (i < 0 || i >= atlas_size_x * atlas_size_y)
			return glm::vec4(0.0f);

		if (atlas_size_x == 1 && atlas_size_y == 1)
			return glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);

		float lx = (1.0f / (atlas_size_x));
		float ly = (1.0f / (atlas_size_y));
		float xs = (i % atlas_size_x) * lx;
		float ys = (i / atlas_size_x) * ly;

		return glm::vec4(xs, ys, lx, ly);
	}

	~Texture() {
		if (loaded)
			unloadTexture();
	}
};

#endif
