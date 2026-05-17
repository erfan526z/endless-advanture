#ifndef ENGINE_GRAPHICS_SHADER_H
#define ENGINE_GRAPHICS_SHADER_H

#include "GraphicsConfig.h"

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

		char* vertexShaderText = nullptr;
		char* fragmentShaderText = nullptr;
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
			
			vertexShaderText[fileLength] = '\0';
			
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
			fragmentShaderText[fileLength] = '\0';
			
			fclose(shaderFile);
		}
		else {
			std::cout << "Could not open fragment shader file : " << fragmentShaderPath << std::endl;
		}

		if (!vertexShaderText || !fragmentShaderText) {
			delete[] vertexShaderText;
			delete[] fragmentShaderText;
			compiled = false;
			linked = false;
			vertexShader = fragmentShader = shaderProgram = 0;
			return;
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
		if (shaderProgram) {
			glDeleteProgram(shaderProgram);
			shaderProgram = 0;
		}
		linked = false;
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
		deleteShaderProgram();
	}
};

#endif
