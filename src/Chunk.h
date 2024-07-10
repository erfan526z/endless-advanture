#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>

#include "BlockTicks.h"
#include "ChunkConstants.h"

class Chunk
{
public:

	void init(int chunk_x, int chunk_z) {
		this->occupied = true;
		this->chunk_x = chunk_x;
		this->chunk_z = chunk_z;
		tickable_blocks.clear();
	}

	void wipe() {
		if (data_load_requested) {
			return;
		}

		if (mesh_update_requested) {
			return;
		}
		if (verticalPieces) {
			for (int i = 0; i < CHUNK_HEIGHT / CHUNK_SIZE; i++) {
				if (verticalPieces[i]) {
					delete[] verticalPieces[i];
				}
			}
			delete[] verticalPieces;
		}
		if (verticalPiecesModified) {
			delete[] verticalPiecesModified;
		}
		if (heights) {
			delete[] heights;
		}
		if (verticalPiecesSize) {
			delete[] verticalPiecesSize;
		}
		if (light_heights) {
			delete[] light_heights;
		}
		verticalPieces = nullptr;
		verticalPiecesModified = nullptr;
		heights = nullptr;
		verticalPiecesSize = nullptr;
		light_heights = nullptr;
		deleteMesh();
		deleteData();
		data_modified = unload_requested = data_load_requested = mesh_update_requested = new_mesh_ready = false;
		chunk_x = chunk_z = vbo_length = max_height = 0;
		in_render_range = 0;
		vao = vbo = 0;
		tickable_blocks.clear();
		occupied = false;
	}

	void loadRequestResponse(unsigned short int* data) {
		this->data = data;
		data_modified = false;
		data_available = true;
		data_load_requested = false;
	}

	void deleteData() {
		data_available = false;
		if (data) {
			delete[] data;
			data = nullptr;
		}
		tickable_blocks.clear();
	}

	void meshRequestResponse() {
		new_mesh_ready = true;
		if (verticalPiecesModified)
			for (int i = 0; i < CHUNK_HEIGHT / CHUNK_SIZE; i++)
				verticalPiecesModified[i] = false;
	}

	// Need to be called from the same thread OpenGL initialized (Disable OpenGL calls with _activate_no_opengl_debug_mode(); USE ONLY FOR DEBUG)
	void updateVRAM() {
		if (mesh_available) {
			deleteMesh();
		}

		// Generate
		int size = 0;

		for (int i = 0; i < CHUNK_HEIGHT / CHUNK_SIZE; i++)
			size += verticalPiecesSize[i];

		//if (size == 0); // Should be impossible, may be handled later

		float* temp_buffer = new float[size];

		int iterrator = 0;
		for (int i = 0; i < CHUNK_HEIGHT / CHUNK_SIZE; i++) {
			if (!verticalPiecesSize[i]) continue;
			int src_size = size - iterrator;
			if (src_size < verticalPiecesSize[i]) {
				std::cout << "That's wierd" << std::endl;
			}
			memcpy(&temp_buffer[iterrator], verticalPieces[i], verticalPiecesSize[i] * sizeof(float));
			iterrator += verticalPiecesSize[i];
		}

		vbo_length = size;

		// Update VRAM
		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);

		glGenBuffers(1, &vbo);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, vbo_length * sizeof(float), temp_buffer, GL_STATIC_DRAW);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);

		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(1);

		glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(5 * sizeof(float)));
		glEnableVertexAttribArray(2);

		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		
		this->vbo_length = vbo_length;

		// Delete & Finalize
		delete[] temp_buffer;
		temp_buffer = nullptr;
		mesh_available = true;
		new_mesh_ready = false;
		mesh_update_requested = false;
	}

	// Need to be called from the same thread OpenGL initialized (Disable OpenGL calls with _activate_no_opengl_debug_mode(); USE ONLY FOR DEBUG)
	void deleteMesh() {
		mesh_available = false;
		if (vao) {
			glDeleteVertexArrays(1, &vao);
			//std::cout << this << " Deleted VAO:" << vao << std::endl;
		}
		if (vbo) {
			glDeleteBuffers(1, &vbo);
			//std::cout << this << " Deleted VBO:" << vbo << std::endl;
		}
		vao = vbo = 0;
	}

	bool isFree() {
		return !occupied;
	}

	bool isDataAvailable() {
		return data_available;
	}

	bool isMeshAvailable() {
		return mesh_available;
	}

	bool isLoadRequested() {
		return data_load_requested;
	}

	bool isMeshUpdateRequested() {
		return mesh_update_requested;
	}

	bool isNewMeshAvailable() {
		return new_mesh_ready;
	}

	bool isUnloadRequested() {
		return unload_requested;
	}

	bool isDataUpdated() {
		if(verticalPiecesModified)
			for (int i = 0; i < CHUNK_HEIGHT / CHUNK_SIZE; i++)
				if (verticalPiecesModified[i]) return true;
		if (data_available && !verticalPiecesModified)
			return true;
		return false;
	}

	bool isDataModified() {
		return data_modified;
	}

	void setLoadRequested() {
		data_load_requested = true;
	}

	void setMeshUpdateRequested() {
		mesh_update_requested = true;
	}

	void setUnloadRequested() {
		unload_requested = true;
	}

	int getChunkX() {
		return chunk_x;
	}

	int getChunkZ() {
		return chunk_z;
	}

	bool getLocalBlock(int x, int y, int z, unsigned short int& block) {
		if (x < 0 || x >= CHUNK_SIZE || y < 0 || y >= CHUNK_HEIGHT || z < 0 || z >= CHUNK_SIZE || !data_available)
			return false;
		int index = y * CHUNK_AREA + x * CHUNK_SIZE + z;
		block = data[index];
		return true;
	}

	// Does not include tickable_blocks checking.
	bool setLocalBlock(int x, int y, int z, unsigned short int block) {
		if (x < 0 || x >= CHUNK_SIZE || y < 0 || y >= CHUNK_HEIGHT || z < 0 || z >= CHUNK_SIZE || !data_available)
			return false;
		int index = y * CHUNK_AREA + x * CHUNK_SIZE + z;
		data[index] = block;
		data_modified = true;
		return true;
	}

	bool getRenderInfo(int& vbo_len, unsigned int& vao) {
		if (!mesh_available) return false;
		vbo_len = vbo_length;
		vao = this->vao;
		return true;
	}

	void setIsInRenderRange(bool is_in) {
		if (is_in)
			in_render_range = 0;
		else
			if (in_render_range < 18000) in_render_range++;
	}

	// If the chunk is in render range, 0 will be returned, else number of ticks which the chunk was outside of view will be returned.
	int isOutOfRenderRange() {
		return in_render_range;
	}

	void setAroundChunkPointers(Chunk* xn, Chunk* xp, Chunk* zn, Chunk* zp) {
		tmp_xn = xn;
		tmp_xp = xp;
		tmp_zn = zn;
		tmp_zp = zp;
	}

	Chunk* getChunkPointerOnXP() {
		return tmp_xp;
	}

	Chunk* getChunkPointerOnXN() {
		return tmp_xn;
	}

	Chunk* getChunkPointerOnZP() {
		return tmp_zp;
	}

	Chunk* getChunkPointerOnZN() {
		return tmp_zn;
	}

	void tickableModified(int layer) {
		setUpdateNeededInLayer(layer);
		data_modified = true;
	}

	// Careful with this
	unsigned short int* getDataPointer() {
		return data;
	}

	// Vector of tickable blocks
	std::vector<TickableBlock>* getTickableBlocksPointer() {
		return &tickable_blocks;
	}

	void setUpdateNeededInLayer(int layer) {
		if (verticalPiecesModified && layer >= 0 && layer < CHUNK_HEIGHT / CHUNK_SIZE)
			verticalPiecesModified[layer] = true;
	}

	void setUpdateNeededInAll() {
		if (!verticalPiecesModified) return;
		for (int layer = 0; layer < /*max_height*/ CHUNK_HEIGHT / CHUNK_SIZE; layer++)
			verticalPiecesModified[layer] = true;
	}

	float**& _verticalChunkData() {
		return verticalPieces;
	}

	bool*& _verticalChunkModified() {
		return verticalPiecesModified;
	}

	int*& _heights() {
		return heights;
	}

	int*& _light_heights() {
		return light_heights;
	}

	int*& _verticalChunkSize() { // Mesh Data Buffer Size
		return verticalPiecesSize;
	}

	int& maxHeight() {
		return max_height;
	}

	ChunkTimeStamp public_chunk_time_stamp; // temporary Variable for generation purposes

private:

	bool occupied = false;

	bool data_available = false;

	bool data_load_requested = false;

	//bool data_updated = false; // This means if data was updated and a mesh request may be needed

	bool data_modified = false; // This means if the data modified since load/generate

	bool mesh_available = false;

	bool mesh_update_requested = false;

	bool new_mesh_ready = false;

	bool unload_requested = false;

	Chunk* tmp_xn;

	Chunk* tmp_xp;

	Chunk* tmp_zn;

	Chunk* tmp_zp;

	int in_render_range;

	int chunk_x = 0;

	int chunk_z = 0;

	unsigned short int* data = nullptr;

	unsigned int vao = 0;

	unsigned int vbo = 0;

	int vbo_length = 0;

	int max_height = 0;

	std::vector<TickableBlock> tickable_blocks;

	float** verticalPieces = nullptr;

	bool* verticalPiecesModified = nullptr;

	int* heights = nullptr;

	int* light_heights = nullptr;

	int* verticalPiecesSize = nullptr;

};
