#ifndef GLOBAL_FUNCTIONS_H
#define GLOBAL_FUNCTIONS_H

#include "stb_image.h"
#include "glad.h"
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

#define DEFAULT_WIDTH 1280
#define DEFAULT_HEIGHT 720

bool* _focus_pointer = nullptr;

int* _key_status = nullptr;

bool _mouse_left_pressed = false;

bool _mouse_right_pressed = false;

bool _mouse_captured = false;

bool _aspect_ratio_updated = true;

int _cwidth = DEFAULT_WIDTH;
int _cheight = DEFAULT_HEIGHT;

void* _ptr_active_camera = nullptr;

void _framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
	_aspect_ratio_updated = true;
	_cwidth = width;
	_cheight = height;
}

void _window_focus_callback(GLFWwindow* window, int focused)
{
	*_focus_pointer = focused;
}

void _keyboard_key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (_key_status) {
		_key_status[key] = action;
	}
}

void _mouse_key_callback(GLFWwindow* window, int button, int action, int modes) {
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
		_mouse_left_pressed = true;
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
		_mouse_left_pressed = false;
	if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
		_mouse_right_pressed = true;
	if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE)
		_mouse_right_pressed = false;
}

bool isKeyPressedOrHeld(int key) {
	if (_key_status && key >= 0 && key < 512) {
		return _key_status[key] == GLFW_PRESS || _key_status[key] == GLFW_REPEAT;
	}
	return false;
}

char getUsernameKeyAndClear(bool space = false) {
	for (int i = GLFW_KEY_A; i <= GLFW_KEY_Z; i++) {
		if (_key_status[i] == GLFW_PRESS) {
			_key_status[i] = 0;
			if (_key_status[GLFW_KEY_LEFT_SHIFT] || _key_status[GLFW_KEY_RIGHT_SHIFT])
				return (char)i;
			else
				return (char)i + 32;
		}
	}
	for (int i = GLFW_KEY_0; i <= GLFW_KEY_9; i++) {
		if (_key_status[i] == GLFW_PRESS) {
			_key_status[i] = 0;
			return (char)i;
		}
	}
	if (_key_status[GLFW_KEY_BACKSPACE]) {
		_key_status[GLFW_KEY_BACKSPACE] = 0;
		return 127;
	}
	if (_key_status[GLFW_KEY_SPACE] && space) {
		_key_status[GLFW_KEY_SPACE] = 0;
		return ' ';
	}
	return 0;
}

void cleanKeys() {
	for (int i = 0; i < 512; i++)
		_key_status[i] = 0;
}

bool isMouseLeftClickPressed() {
	return _mouse_left_pressed;
}

bool isMouseRightClickPressed() {
	return _mouse_right_pressed;
}

void clearMouseLeftClickPressed() {
	_mouse_left_pressed = false;
}

void clearMouseRightClickPressed() {
	_mouse_right_pressed = false;
}

void inGameCursorPositionCallback(GLFWwindow* window, double xpos, double ypos);

struct ModelVertex {
	float x, y, z;
};

struct ModelTextureCoord {
	float u, v;
};

struct ModelNormal {
	float x, y, z;
};

struct ModelFace {
	int v1, t1, n1;
	int v2, t2, n2;
	int v3, t3, n3;
};

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

class RawModel {
private:

	unsigned int vaoID = 0;
	unsigned int vboID = 0;
	unsigned int vsize = 0;

	bool loadObj(const char* path, std::vector<ModelVertex>& vertices, std::vector<ModelTextureCoord>& textureCoords, std::vector<ModelNormal>& normals, std::vector<ModelFace>& faces) {
		std::ifstream file(path);
		if (!file.is_open()) {
			return false;
		}

		std::string line;
		while (std::getline(file, line)) {
			std::istringstream iss(line);
			std::string type;
			iss >> type;

			if (type == "v") {
				ModelVertex vertex;
				iss >> vertex.x >> vertex.y >> vertex.z;
				vertices.push_back(vertex);
			}
			else if (type == "vt") {
				ModelTextureCoord textureCoord;
				iss >> textureCoord.u >> textureCoord.v;
				textureCoords.push_back(textureCoord);
			}
			else if (type == "vn") {
				ModelNormal normal;
				iss >> normal.x >> normal.y >> normal.z;
				normals.push_back(normal);
			}
			else if (type == "f") {
				ModelFace face;

				char slash;

				iss >> face.v1 >> slash >> face.t1 >> slash >> face.n1
					>> face.v2 >> slash >> face.t2 >> slash >> face.n2
					>> face.v3 >> slash >> face.t3 >> slash >> face.n3;

				// OBJ indices start at 1, so we need to subtract 1 from each index
				face.v1--;
				face.t1--;
				face.n1--;
				face.v2--;
				face.t2--;
				face.n2--;
				face.v3--;
				face.t3--;
				face.n3--;

				faces.push_back(face);
			}
		}

		file.close();

		return true;
	}

public:

	void init(unsigned int vao, unsigned int vbo, unsigned int indices_size) {
		vaoID = vao;
		vboID = vbo;
		this->vsize = vsize;
	}

	bool initFromFile(const char* file_path) {
		if (vaoID || vboID || vsize)
			return 0;

		std::vector<ModelVertex> vertices;
		std::vector<ModelTextureCoord> textureCoords;
		std::vector<ModelNormal> normals;
		std::vector<ModelFace> faces;

		if (!loadObj(file_path, vertices, textureCoords, normals, faces)) {
			return 0;
		}

		// Arrange items into arrays
		int face_size = faces.size();
		int buffer_size = face_size * 18;
		float* fbuffer = new float[buffer_size];

		/*for (int i = 0; i < faces.size(); i++) {
			ModelFace f = faces.at(i);
			ModelVertex cv = vertices.at(f.v1);
			fbuffer[i * 24] = cv.x;
			fbuffer[i * 24 + 1] = cv.y;
			fbuffer[i * 24 + 2] = cv.z;
			ModelTextureCoord ct = textureCoords.at(f.t1);
			fbuffer[i * 24 + 3] = ct.u;
			fbuffer[i * 24 + 4] = ct.v;
			ModelNormal cn = normals.at(f.n1);
			fbuffer[i * 24 + 5] = cn.x;
			fbuffer[i * 24 + 6] = cn.y;
			fbuffer[i * 24 + 7] = cn.z;

			cv = vertices.at(f.v2);
			fbuffer[i * 24 + 8] = cv.x;
			fbuffer[i * 24 + 9] = cv.y;
			fbuffer[i * 24 + 10] = cv.z;
			ct = textureCoords.at(f.t2);
			fbuffer[i * 24 + 11] = ct.u;
			fbuffer[i * 24 + 12] = ct.v;
			cn = normals.at(f.n2);
			fbuffer[i * 24 + 13] = cn.x;
			fbuffer[i * 24 + 14] = cn.y;
			fbuffer[i * 24 + 15] = cn.z;

			cv = vertices.at(f.v3);
			fbuffer[i * 24 + 16] = cv.x;
			fbuffer[i * 24 + 17] = cv.y;
			fbuffer[i * 24 + 18] = cv.z;
			ct = textureCoords.at(f.t3);
			fbuffer[i * 24 + 19] = ct.u;
			fbuffer[i * 24 + 20] = ct.v;
			cn = normals.at(f.n3);
			fbuffer[i * 24 + 21] = cn.x;
			fbuffer[i * 24 + 22] = cn.y;
			fbuffer[i * 24 + 23] = cn.z;
		}*/

		// THIS IS TEMPORARY
		for (int i = 0; i < faces.size(); i++) {
			ModelFace f = faces.at(i);
			ModelVertex cv = vertices.at(f.v1);
			fbuffer[i * 18] = cv.x;
			fbuffer[i * 18 + 1] = cv.y;
			fbuffer[i * 18 + 2] = cv.z;
			ModelTextureCoord ct = textureCoords.at(f.t1);
			fbuffer[i * 18 + 3] = ct.u;
			fbuffer[i * 18 + 4] = ct.v;
			float light = 1.0f;
			fbuffer[i * 18 + 5] = light;

			cv = vertices.at(f.v2);
			fbuffer[i * 18 + 6] = cv.x;
			fbuffer[i * 18 + 7] = cv.y;
			fbuffer[i * 18 + 8] = cv.z;
			ct = textureCoords.at(f.t2);
			fbuffer[i * 18 + 9] = ct.u;
			fbuffer[i * 18 + 10] = ct.v;
			light = 1.0f;
			fbuffer[i * 18 + 11] = light;

			cv = vertices.at(f.v3);
			fbuffer[i * 18 + 12] = cv.x;
			fbuffer[i * 18 + 13] = cv.y;
			fbuffer[i * 18 + 14] = cv.z;
			ct = textureCoords.at(f.t3);
			fbuffer[i * 18 + 15] = ct.u;
			fbuffer[i * 18 + 16] = ct.v;
			light = 1.0f;
			fbuffer[i * 18 + 17] = light;
		}

		vertices.clear();
		textureCoords.clear();
		normals.clear();

		// Loading values into VAO
		glGenVertexArrays(1, &vaoID);
		glBindVertexArray(vaoID);

		glGenBuffers(1, &vboID);
		glBindBuffer(GL_ARRAY_BUFFER, vboID);
		glBufferData(GL_ARRAY_BUFFER, buffer_size * sizeof(float), fbuffer, GL_STATIC_DRAW);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);

		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(1);

		glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(5 * sizeof(float)));
		glEnableVertexAttribArray(2);

		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		vsize = buffer_size;
		delete[] fbuffer;

		return 1;
	}

	void initFromData(float* vertices, int len) {
		glGenVertexArrays(1, &vaoID);
		glBindVertexArray(vaoID);

		glGenBuffers(1, &vboID);
		glBindBuffer(GL_ARRAY_BUFFER, vboID);
		glBufferData(GL_ARRAY_BUFFER, len * sizeof(float), vertices, GL_STATIC_DRAW);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);

		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(1);

		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		vsize = len / 5;
	}

	void initFromData(float* data, int len, int singledlen) {
		glGenVertexArrays(1, &vaoID);
		glBindVertexArray(vaoID);

		glGenBuffers(1, &vboID);
		glBindBuffer(GL_ARRAY_BUFFER, vboID);
		glBufferData(GL_ARRAY_BUFFER, len * sizeof(float), data, GL_STATIC_DRAW);

		glVertexAttribPointer(0, singledlen, GL_FLOAT, GL_FALSE, singledlen * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);

		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		vsize = len / singledlen;
	}

	void bindModel() {
		glBindVertexArray(vaoID);
	}

	unsigned int getVaoID() {
		return vaoID;
	}

	void unbindModel() {
		glBindVertexArray(0);
	}

	void destroyModel() {
		if (vaoID)
			glDeleteVertexArrays(1, &vaoID);
		if (vboID)
			glDeleteBuffers(1, &vboID);
		vaoID = vboID = vsize = 0;
	}

	unsigned int getSize() {
		return vsize;
	}
};

class Shader {
private:
	unsigned int vertexShader;
	unsigned int fragmentShader;
	unsigned int shaderProgram;
	bool compiled;
	bool linked;

public:
	Shader(const char* vertexShaderPath, const char* fragmentShaderPath) {
		int success;
		char infolog[512];

		char* vertexShaderText;
		char* fragmentShaderText;
		FILE* shaderFile;
		long long int fileLength = 0;

		shaderFile = fopen(vertexShaderPath, "r");
		if (shaderFile != NULL) {
			fseek(shaderFile, 0, SEEK_END);
			fileLength = ftell(shaderFile);
			vertexShaderText = new char[fileLength + 1];
			rewind(shaderFile);
			fread((char*)vertexShaderText, sizeof(char), fileLength, shaderFile);
			
			/*
			int nonNewlineIndex = 0;
			for (int i = 0; i < fileLength; i++) {
				if (vertexShaderText[i] != '\n') {
				    vertexShaderText[nonNewlineIndex] = vertexShaderText[i];
				    nonNewlineIndex++;
				}
			}
			vertexShaderText[nonNewlineIndex] = '\0';
			*/
			
			vertexShaderText[fileLength] = NULL;
			
			fclose(shaderFile);
			shaderFile = NULL;
		}
		else {
			std::cout << "Could not open vertex shader file : " << vertexShaderPath << std::endl;
		}

		fileLength = 0;
		shaderFile = fopen(fragmentShaderPath, "r");
		if (shaderFile != NULL) {
			fseek(shaderFile, 0, SEEK_END);
			fileLength = ftell(shaderFile);
			fragmentShaderText = new char[fileLength + 1];
			rewind(shaderFile);
			fread((char*)fragmentShaderText, sizeof(char), fileLength, shaderFile);
			
			/*
			int nonNewlineIndex = 0;
			for (int i = 0; i < fileLength; i++) {
				if (vertexShaderText[i] != '\n') {
				    vertexShaderText[nonNewlineIndex] = vertexShaderText[i];
				    nonNewlineIndex++;
				}
			}
			vertexShaderText[nonNewlineIndex] = '\0';
			*/
			fragmentShaderText[fileLength] = NULL;
			
			fclose(shaderFile);
		}
		else {
			std::cout << "Could not open fragment shader file : " << fragmentShaderPath << std::endl;
		}

		compiled = true;
		linked = false;

		vertexShader = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertexShader, 1, &vertexShaderText, NULL);
		glCompileShader(vertexShader);
		glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
		if (!success) {
			glGetShaderInfoLog(vertexShader, 512, NULL, infolog);
			std::cout << "Vertex shader compile failed :\n" << infolog << std::endl;
			compiled = false;
		}

		fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragmentShader, 1, &fragmentShaderText, NULL);
		glCompileShader(fragmentShader);
		glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
		if (!success) {
			glGetShaderInfoLog(fragmentShader, 512, NULL, infolog);
			std::cout << "Fragment shader compile failed :\n" << infolog << std::endl;
			compiled = false;
		}

		shaderProgram = 0;

		if (compiled) {
			shaderProgram = glCreateProgram();
			glAttachShader(shaderProgram, vertexShader);
			glAttachShader(shaderProgram, fragmentShader);
			glLinkProgram(shaderProgram);
			glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
			if (!success) {
				glGetProgramInfoLog(shaderProgram, 512, NULL, infolog);
				std::cout << "Shader linking failed :\n" << infolog << std::endl;
			}
			else
				linked = true;

			glDeleteShader(vertexShader);
			glDeleteShader(fragmentShader);
		}

		if (vertexShaderText)
			delete[] vertexShaderText;
		if (fragmentShaderText)
			delete[] fragmentShaderText;
	}

	void enableShaderProgram() {
		if (compiled && linked) {
			glUseProgram(shaderProgram);
		}
	}

	void deleteShaderProgram() {
		// TODO
	}

	unsigned int getID() {
		return shaderProgram;
	}

	void loadUniform4f(const char* name, float x, float y, float z, float w) {
		if (compiled && linked)
			glUniform4f(glGetUniformLocation(shaderProgram, name), x, y, z, w);
	}

	void loadUniform3f(const char* name, float x, float y, float z) {
		if (compiled && linked)
			glUniform3f(glGetUniformLocation(shaderProgram, name), x, y, z);
	}

	void loadUniform3f(const char* name, glm::vec3 value) {
		if (compiled && linked)
			glUniform3f(glGetUniformLocation(shaderProgram, name), value.x, value.y, value.z);
	}

	void loadUniform2f(const char* name, float x, float y) {
		if (compiled && linked)
			glUniform2f(glGetUniformLocation(shaderProgram, name), x, y);
	}

	void loadUniform1f(const char* name, float value) {
		if (compiled && linked)
			glUniform1f(glGetUniformLocation(shaderProgram, name), value);
	}

	void loadMatrix4f(const char* name, glm::mat4 matrix) {
		glUniformMatrix4fv(glGetUniformLocation(shaderProgram, name), 1, GL_FALSE, glm::value_ptr(matrix));
	}

	void loadMatrix3f(const char* name, glm::mat3 matrix) {
		glUniformMatrix3fv(glGetUniformLocation(shaderProgram, name), 1, GL_FALSE, glm::value_ptr(matrix));
	}

	void loadMatrix2f(const char* name, glm::mat2 matrix) {
		glUniformMatrix2fv(glGetUniformLocation(shaderProgram, name), 1, GL_FALSE, glm::value_ptr(matrix));
	}

	void loadDirectMatrix4f(unsigned int location, glm::mat4 matrix) {
		glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(matrix));
	}

	void loadDirectMatrix3f(unsigned int location, glm::mat3 matrix) {
		glUniformMatrix3fv(location, 1, GL_FALSE, glm::value_ptr(matrix));
	}

	void loadDirectMatrix2f(unsigned int location, glm::mat2 matrix) {
		glUniformMatrix2fv(location, 1, GL_FALSE, glm::value_ptr(matrix));
	}

	void loadUniform1i(const char* name, int value) {
		glUniform1i(glGetUniformLocation(shaderProgram, name), value);
	}

	~Shader() {
		// TODO
	}
};

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

class GUIImage {
public:

	GUIImage() {
		texture = nullptr;
		g_pos_x = g_pos_y = 0;
		g_width = g_height = 16;
		g_factor = 1;
		g_xalign = g_yalign = 0;
		g_atlas = 0;
		bound_changed = atlas_changed = true;
		atlas = glm::vec4(0.0f);
		bounds = glm::vec4(0.0f);
	}

	GUIImage(Texture* texture, int x_position, int y_position, int width, int height, int x_alignment, int y_alignment, int gui_scale, int atlas_index) {
		this->texture = texture;
		g_pos_x = x_position;
		g_pos_y = y_position;
		g_width = width;
		g_height = height;
		g_factor = gui_scale;
		g_xalign = x_alignment;
		g_yalign = y_alignment;
		g_atlas = atlas_index;
		bound_changed = atlas_changed = true;
		atlas = glm::vec4(0.0f);
		bounds = glm::vec4(0.0f);
	}

	GUIImage(Texture* texture, int x_position, int y_position, int height, int x_alignment, int y_alignment, int gui_scale, int atlas_index) {
		this->texture = texture;
		g_pos_x = x_position;
		g_pos_y = y_position;
		g_width = height;
		g_height = height;
		g_factor = gui_scale;
		g_xalign = x_alignment;
		g_yalign = y_alignment;
		g_atlas = atlas_index;
		bound_changed = atlas_changed = true;
		atlas = glm::vec4(0.0f);
		bounds = glm::vec4(0.0f);
	}

	void setPosition(int x, int y) {
		g_pos_x = x;
		g_pos_y = y;
		bound_changed = true;
	}

	void setSize(int height, int width = -1) {
		if (width == -1)
			g_width = g_height = height;
		else {
			g_width = width;
			g_height = height;
		}
		bound_changed = true;
	}

	void setAlignment(int xalign, int yalign) {
		g_xalign = xalign;
		g_yalign = yalign;
		bound_changed = true;
	}

	void setGUIScale(int factor) {
		g_factor = factor;
		bound_changed = true;
	}

	void setAtlasIndex(int index) {
		if (g_atlas == index) return;
		g_atlas = index;
		atlas_changed = true;
	}

	glm::vec4 calculateBounds(int screen_width, int screen_height) {
		if (last_screen_width == screen_width && last_screen_height == screen_height && !bound_changed)
			return bounds;
		bound_changed = false;
		last_screen_width = screen_width;
		last_screen_height = screen_height;
		int n_pos_x = g_factor * g_pos_x;
		int n_pos_y = g_factor * g_pos_y;
		int n_width = g_factor * g_width;
		int n_height = g_factor * g_height;
		int position_pixel_x = (!g_xalign) ? (screen_width / 2 + n_pos_x) : (g_xalign == 1) ? (screen_width - n_pos_x) - n_width / 2 : n_pos_x + n_width / 2;
		int position_pixel_y = (!g_yalign) ? (screen_height / 2 + n_pos_y) : (g_yalign == 1) ? (screen_height - n_pos_y) - n_height / 2 : n_pos_y + n_height / 2;
		float px = (float)(position_pixel_x * 2) / (float)(screen_width) - 1.0f;
		float py = (float)(position_pixel_y * 2) / (float)(screen_height) - 1.0f;
		float sx = (float)n_width / (float)(screen_width);
		float sy = (float)n_height / (float)(screen_height);
		bounds = glm::vec4(sx, sy, px, py);
		return bounds;
	}

	glm::vec4 calculateAtlas() {
		if (!atlas_changed) return atlas;
		atlas_changed = false;
		if (!texture) return glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
		return atlas = texture->getAtlasCoordinate(g_atlas);
	}

	Texture* getTexture() {
		return texture;
	}

	bool hasSameTexture(unsigned int t_id) {
		return texture->getID() == t_id;
	}

	bool isMouseInside(float mouse_x, float mouse_y) {
		mouse_y = (float)last_screen_height - mouse_y;
		int n_pos_x = g_factor * g_pos_x;
		int n_pos_y = g_factor * g_pos_y;
		int n_width = g_factor * g_width;
		int n_height = g_factor * g_height;
		int start_x = (!g_xalign) ? (last_screen_width / 2 + n_pos_x) - n_width / 2 :
			(g_xalign == 1) ? (last_screen_width - n_pos_x) - n_width : n_pos_x;
		int start_y = (!g_yalign) ? (last_screen_height / 2 + n_pos_y) - n_height / 2 : 
			(g_yalign == 1) ? (last_screen_height - n_pos_y) - n_height : n_pos_y;
		int end_x = start_x + n_width;
		int end_y = start_y + n_height;
		return (mouse_x >= start_x && mouse_x <= end_x && mouse_y >= start_y && mouse_y <= end_y);
	}

private:

	Texture* texture; // Pointer to the texture

	int g_pos_x; // Horizontal position in pixels

	int g_pos_y; // Vertical position in pixels

	int g_width; // Width in pixels

	int g_height; // Height in pixels

	int g_factor; // Scale factor (All pixel values will multiply by this, default: 1)

	int g_xalign; // Horizontal alignment

	int g_yalign; // Vertical alignement

	int g_atlas; // Texture Atlas Index

	bool bound_changed; // If nothing is updated, there is no need to recalculate bounds.

	bool atlas_changed; // If atlas index is the same, there is no need to recalculate altas vec4.

	glm::vec4 bounds;

	glm::vec4 atlas;

	int last_screen_width = 0;

	int last_screen_height = 0;

};

class GUIText {
public:

	GUIText() {
		texture = nullptr;
		g_pos_x = g_pos_y = 0;
		g_char_width = 16;
		g_factor = 1;
		g_xalign = g_yalign = 0;
		bound_changed = true;
		bounds = glm::vec4(0.0f);
		text[0] = 0;
	}

	GUIText(Texture* texture, const char* text, int x_position, int y_position, int char_width, int x_alignment, int y_alignment, int gui_scale) {
		this->texture = texture;
		g_pos_x = x_position;
		g_pos_y = y_position;
		g_char_width = char_width;
		g_factor = gui_scale;
		g_xalign = x_alignment;
		g_yalign = y_alignment;
		bound_changed = true;
		bounds = glm::vec4(0.0f);

		if (!text) {
			this->text[0] = 0;
			return;
		}

		int l1 = strlen(text) + 1;
		if (l1 >= 256) l1 = 256;
		memcpy(this->text, text, l1); // +1 is because we also copy '\0' at the end.
		this->text[255] = 0; // if the input text size was bigger than buffer size, it needs this to prevent access violation errors.
	}

	void setPosition(int x, int y) {
		g_pos_x = x;
		g_pos_y = y;
		bound_changed = true;
	}

	void setSize(int char_width) {
		g_char_width = char_width;
		bound_changed = true;
	}

	void setAlignment(int xalign, int yalign) {
		g_xalign = xalign;
		g_yalign = yalign;
		bound_changed = true;
	}

	void setGUIScale(int factor) {
		g_factor = factor;
		bound_changed = true;
	}

	void setText(const char* text) {
		if (!text) {
			this->text[0] = 0;
			return;
		}
		int l1 = strlen(text) + 1;
		if (l1 >= 256) l1 = 256;
		memcpy(this->text, text, l1); // +1 is because we also copy '\0' at the end.
		this->text[255] = 0; // if the input text size was bigger than buffer size, it needs this to prevent access violation errors.
		bound_changed = true;
	}

	glm::vec4 calculateBounds(int screen_width, int screen_height) {
		if (last_screen_width == screen_width && last_screen_height == screen_height && !bound_changed)
			return bounds;
		bound_changed = false;
		last_screen_width = screen_width;
		last_screen_height = screen_height;
		int n_pos_x = g_factor * g_pos_x;
		int n_pos_y = g_factor * g_pos_y;
		int n_char_width = g_factor * g_char_width;
		int position_pixel_x = (!g_xalign) ? (screen_width / 2 + n_pos_x) - (getStrlen() * n_char_width - n_char_width) / 2 : (g_xalign == 1) ? (screen_width - n_pos_x) - n_char_width / 2 : n_pos_x + n_char_width / 2;
		int position_pixel_y = (!g_yalign) ? (screen_height / 2 + n_pos_y) : (g_yalign == 1) ? (screen_height - n_pos_y) - n_char_width / 2 : n_pos_y + n_char_width / 2;
		float px = (float)(position_pixel_x * 2) / (float)(screen_width)-1.0f;
		float py = (float)(position_pixel_y * 2) / (float)(screen_height)-1.0f;
		float sx = (float)n_char_width / (float)(screen_width);
		float sy = (float)n_char_width / (float)(screen_height);
		bounds = glm::vec4(sx, sy, px, py);
		return bounds;
	}

	float getPositionOffset(int screen_width, int character) {
		int pixels = character * g_char_width * g_factor;
		return (float)(pixels * 2) / (float)(screen_width);
	}

	glm::vec4 calculateAtlas(int character) {
		if (character >= 256) return glm::vec4(0.0f);
		return texture->getAtlasCoordinate(text[character]);
	}

	Texture* getTexture() {
		return texture;
	}

	int getStrlen() {
		return strlen(text);
	}

	bool hasSameTexture(unsigned int t_id) {
		return texture->getID() == t_id;
	}

private:

	char text[256];

	Texture* texture; // Pointer to the font texture

	int g_pos_x; // Horizontal position in pixels

	int g_pos_y; // Vertical position in pixels

	int g_char_width; // Character width in pixels

	int g_factor; // Scale factor (All pixel values will multiply by this, default: 1)

	int g_xalign; // Horizontal alignment

	int g_yalign; // Vertical alignement

	bool bound_changed; // If nothing is updated, there is no need to recalculate bounds.

	glm::vec4 bounds;

	int last_screen_width = 0;

	int last_screen_height = 0;
};

class GUIScene {

public:

	GUIScene() {}

	~GUIScene() {
		guis.clear();
		texts.clear();
	}

	void add(GUIImage& gui) {
		guis.push_back(&gui);
	}

	void add(GUIText& text) {
		texts.push_back(&text);
	}

	void clear() {
		guis.clear();
		texts.clear();
	}

	GUIImage& guiAt(int i) {
		return *guis.at(i);
	}

	GUIText& textAt(int i) {
		return *texts.at(i);
	}

	int guiCnt() {
		return guis.size();
	}

	int textCnt() {
		return texts.size();
	}

	void setGUIScaleForAll(int gui_scale) {
		for (GUIImage* gui : guis)
			gui->setGUIScale(gui_scale);
		for (GUIText* text : texts)
			text->setGUIScale(gui_scale);
	}

private:
	std::vector<GUIImage*> guis;
	std::vector<GUIText*> texts;

};

class Renderer {
private:
	int width;
	int height;
	int active_shader;
	bool focused;
	bool active;
	bool def_gui_en;
	float sun_light;
	float weather_factor;
	GLFWwindow* window;
	Shader* shader_3d;
	Shader* shader_2d;
	Shader* shader_3dbg;
	Camera* currentCamera;
	RawModel* default_gui;
	RawModel* bg_render_assist;
	glm::vec3 sky_color;
	glm::vec3 horizon_color;

	void calculate_sky_color(float rain_fac, float time_fac) {
		sky_color.x = (1.0f - rain_fac) * time_fac * 0.4f + (rain_fac) * time_fac * 0.5f;
		sky_color.y = (1.0f - rain_fac) * time_fac * 0.6f + (rain_fac) * time_fac * 0.4f;
		sky_color.z = 0.9f - rain_fac * 0.65;

		horizon_color.x = 0.7f - rain_fac * 0.3f - time_fac * 0.1f;
		horizon_color.y = 0.7f - rain_fac * 0.3f - time_fac * 0.1f;
		horizon_color.z = 0.8f - rain_fac * 0.4f;
	}

public:

	Renderer() {
		shader_2d = nullptr;
		shader_3d = nullptr;
		shader_3dbg = nullptr;
		width = 800;
		height = 600;
		sky_color = glm::vec3(0.5f, 0.6f, 0.8f);
		horizon_color = glm::vec3(0.7f, 0.7f, 0.7f);
		active = false;
		focused = false;
		_focus_pointer = &focused;
		active_shader = 0;
		_key_status = new int[512];
		currentCamera = nullptr;
		window = nullptr;
		default_gui = nullptr;
		bg_render_assist = nullptr;
		def_gui_en = false;
		sun_light = 1.0f;
		weather_factor = 0.0f;
	}

	bool initializeWindow(int width = DEFAULT_WIDTH, int height = DEFAULT_HEIGHT, const char* title = "My Game") {
		glfwInit();

		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

		window = glfwCreateWindow(width, height, title, NULL, NULL);
		if (window == NULL)
		{
			std::cout << "Failed to create GLFW window" << std::endl;
			glfwTerminate();
			return -1;
		}

		glfwMakeContextCurrent(window);

		if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
		{
			std::cout << "Failed to initialize GLAD" << std::endl;
			return -1;
		}

		glViewport(0, 0, width, height);

		glfwSetFramebufferSizeCallback(window, _framebuffer_size_callback);

		glfwSetWindowFocusCallback(window, _window_focus_callback);

		glfwSetKeyCallback(window, _keyboard_key_callback);

		glfwSetCursorPosCallback(window, inGameCursorPositionCallback);

		glfwSetMouseButtonCallback(window, _mouse_key_callback);

		glfwSwapInterval(1);

		glActiveTexture(GL_TEXTURE0);

		active = true;

		def_gui_en = false;
		default_gui = new RawModel();
		float ver[] = {
			-1.0f, -1.0f, 0.0f,  0.0f, 1.0f,
			1.0f , -1.0f, 0.0f,  1.0f, 1.0f,
			1.0f ,  1.0f, 0.0f,  1.0f, 0.0f,

			-1.0f, -1.0f, 0.0f,  0.0f, 1.0f,
			1.0f ,  1.0f, 0.0f,  1.0f, 0.0f,
			-1.0f,  1.0f, 0.0f,  0.0f, 0.0f
		};
		default_gui->initFromData(ver, 30);

		bg_render_assist = new RawModel();
		float ver2[] = {
			-1.0f, -1.0f,
			-1.0f, 1.0f,
			1.0f, -1.0f,
			1.0f, -1.0f,
			-1.0f, 1.0f,
			1.0f, 1.0f
		};
		bg_render_assist->initFromData(ver2, 12, 2);

		return 0;
	}

	void initialize3DShader(const char* vshaderpath, const char* fshaderpath) {
		shader_3d = new Shader(vshaderpath, fshaderpath);
	}

	void initialize2DShader(const char* vshaderpath, const char* fshaderpath) {
		shader_2d = new Shader(vshaderpath, fshaderpath);
	}

	void initialize3DBackgroundShader(const char* vshaderpath, const char* fshaderpath) {
		shader_3dbg = new Shader(vshaderpath, fshaderpath);
	}

	void prepare() {
		glClearColor(sky_color.x * sun_light, sky_color.y * sun_light, sky_color.z * sun_light, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		calculate_sky_color(weather_factor, sun_light);
	}

	void prepare2D() {
		active_shader = 2;
		shader_2d->enableShaderProgram();
		glDisable(GL_DEPTH_TEST);
	}

	void prepare3D() {
		active_shader = 3;
		shader_3d->enableShaderProgram();
		glEnable(GL_DEPTH_TEST);
		shader_3d->loadMatrix4f("view", currentCamera->getViewMatrix());
		shader_3d->loadMatrix4f("projection", currentCamera->getProjectionMatrix());
		shader_3d->loadUniform1f("light_factor", sun_light);
		if (_aspect_ratio_updated) {
			currentCamera->updateAspectRatio((float)_cwidth / (float)_cheight);
			shader_3d->loadMatrix4f("projection", currentCamera->getProjectionMatrix());
		}
		shader_3d->loadUniform1f("fogDensity", 0.007f);
		shader_3d->loadUniform3f("fog_color", horizon_color);
	}

	void prepareBackground() {
		active_shader = 30;
		shader_3dbg->enableShaderProgram();
		shader_3dbg->loadUniform2f("screen", (float)_cwidth, (float)_cheight);
		shader_3dbg->loadUniform1f("light_factor", sun_light);
		shader_3dbg->loadUniform3f("color_sky", sky_color);
		shader_3dbg->loadUniform3f("color_horizon", horizon_color);
		shader_3dbg->loadUniform3f("view_direction", currentCamera->getLookingVector());
		glDisable(GL_DEPTH_TEST);
	}

	void setLightLevel(float factor) {
		sun_light = factor;
	}

	void setWeatherLevel(float weatherstate) {
		weather_factor = weatherstate;
	}

	void setCursorMode(int mode) {
		if (mode == 1) {
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
			_mouse_captured = true;
		}
		else {
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
			_mouse_captured = false;
		}
	}

	void renderBackground() {
		bg_render_assist->bindModel();
		glDrawArrays(GL_TRIANGLES, 0, bg_render_assist->getSize());
		bg_render_assist->unbindModel();
	}

	void renderGUIScene(GUIScene& scene) {

		// Prepare 
		
		default_gui->bindModel();
		unsigned int current_bound_texture = 0;

		// GUIs

		int guis = scene.guiCnt();
		for (int gui_idx = 0; gui_idx < guis; gui_idx++) {
			if (!scene.guiAt(gui_idx).hasSameTexture(current_bound_texture)) {
				glBindTexture(GL_TEXTURE_2D, scene.guiAt(gui_idx).getTexture()->getID());
				current_bound_texture = scene.guiAt(gui_idx).getTexture()->getID();
			}
			glm::vec4 bounds = scene.guiAt(gui_idx).calculateBounds(_cwidth, _cheight);
			glm::vec4 atlasv = scene.guiAt(gui_idx).calculateAtlas();
			shader_2d->loadUniform4f("trans_values", bounds.x, bounds.y, bounds.z, bounds.w);
			shader_2d->loadUniform4f("atlas_values", atlasv.x, atlasv.y, atlasv.z, atlasv.w);
			glDrawArrays(GL_TRIANGLES, 0, default_gui->getSize());
		}

		// Texts

		int texts = scene.textCnt();
		for (int text_idx = 0; text_idx < texts; text_idx++) {
			if (!scene.textAt(text_idx).hasSameTexture(current_bound_texture)) {
				glBindTexture(GL_TEXTURE_2D, scene.textAt(text_idx).getTexture()->getID());
				current_bound_texture = scene.textAt(text_idx).getTexture()->getID();
			}
			int stlen = scene.textAt(text_idx).getStrlen();
			glm::vec4 baseBounds = scene.textAt(text_idx).calculateBounds(_cwidth, _cheight);
			for (int j = 0; j < stlen; j++) {
				shader_2d->loadUniform4f("trans_values", baseBounds.x, baseBounds.y, baseBounds.z + scene.textAt(text_idx).getPositionOffset(_cwidth, j), baseBounds.w);
				glm::vec4 atlasValues = scene.textAt(text_idx).calculateAtlas(j);
				shader_2d->loadUniform4f("atlas_values", atlasValues.x, atlasValues.y, atlasValues.z, atlasValues.w);
				glDrawArrays(GL_TRIANGLES, 0, default_gui->getSize());
			}
		}

		// End

		glBindTexture(GL_TEXTURE_2D, 0);
		default_gui->unbindModel();

	}

	void renderBasicEntity(BasicEntity& entity) {

		glm::vec4 atlas_values = entity.getTexture()->getAtlasCoordinate(entity.getAtlasIndex());

		shader_3d->loadUniform2f("coordFactors", atlas_values.z, atlas_values.w);
		shader_3d->loadUniform2f("coordOffsets", atlas_values.x, atlas_values.y * entity.atlasYMultiplyer());
		
		shader_3d->loadMatrix4f("transform", entity.getTransformationMatrix());
		entity.getModel()->bindModel();
		glBindTexture(GL_TEXTURE_2D, entity.getTexture()->getID());
		glDrawArrays(GL_TRIANGLES, 0, entity.getModel()->getSize());
		glBindTexture(GL_TEXTURE_2D, 0);
		entity.getModel()->unbindModel();
	}
	
	void renderChunk(Texture* texture, glm::mat4 transform, unsigned int vbolen, unsigned int vao, bool first = false) {
		if (first) {
			glBindTexture(GL_TEXTURE_2D, texture->getID());
			shader_3d->loadUniform2f("coordFactors", 1.0f, 1.0f);
			shader_3d->loadUniform2f("coordOffsets", 0.0f, 0.0f);
		}

		shader_3d->loadMatrix4f("transform", transform);

		glBindVertexArray(vao);

		glDrawArrays(GL_TRIANGLES, 0, vbolen);

		glBindVertexArray(0);
	}

	void updateDisplay() {
		_aspect_ratio_updated = false;

		glfwSwapBuffers(window);
		glfwPollEvents();

		active_shader = 0;
	}

	void destroy() {
		//delete default_gui;

		if (shader_3d) {
			shader_3d->deleteShaderProgram();
			shader_3d->~Shader();
			delete shader_3d;
			shader_3d = nullptr;
		}

		if (shader_2d) {
			shader_2d->deleteShaderProgram();
			shader_2d->~Shader();
			delete shader_2d;
			shader_2d = nullptr;
		}

		if (shader_3dbg) {
			shader_3dbg->deleteShaderProgram();
			delete shader_3dbg;
			shader_3dbg = nullptr;
		}

		if (active) {
			glfwTerminate();
			delete[] _key_status;
		}

		active = false;
	}

	void setCurrentCamera(Camera* camera) {
		camera->updateAspectRatio((float)width / (float)height);
		_ptr_active_camera = camera;
		this->currentCamera = camera;
	}

	bool isCloseRequested() {
		return glfwWindowShouldClose(window);
	}

	GLFWwindow* getWindow() {
		return window;
	}

	void getMouseCoordinate(float& x, float& y) {
		double cx, cy;
		glfwGetCursorPos(window, &cx, &cy);
		x = (float)(cx);
		y = (float)(cy);
	}

	void deactivateShaders() {
		active_shader = 0;
		glUseProgram(0);
	}

	~Renderer() {
		destroy();
	}

};

void inGameCursorPositionCallback(GLFWwindow* window, double xpos, double ypos) {
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
