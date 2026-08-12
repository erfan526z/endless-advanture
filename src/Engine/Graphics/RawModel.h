#ifndef ENGINE_GRAPHICS_RAW_MODEL_H
#define ENGINE_GRAPHICS_RAW_MODEL_H

#include "GraphicsConfig.h"
#include "ModelTypes.h"

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
		this->vsize = indices_size;
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

#endif
