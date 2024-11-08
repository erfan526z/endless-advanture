#pragma once

#include <iostream>
#include <filesystem>
#include <string>

#include "BlockTicks.h"

class ChunkDataFile 
{
public:

	ChunkDataFile(const char* save_folder, bool debug_messages = false) {
		this->save_folder = save_folder;
		this->debug_messages = debug_messages;
		verify_folder();
	}

	// Writes the chunk data into storage ('data' size is not being checked in the method, be aware).
	bool saveChunkData(unsigned short int* data, int chunk_x, int chunk_z, int chunk_size, int chunk_height) {

		if (!folder_availabe) return false;

		int len = strlen(save_folder);
		char* path = new char[len + 64LL];
		sprintf(path, "%s%08x%08x.bin", save_folder, chunk_x, chunk_z);
		
		FILE* fp;
		fp = fopen(path, "wb");
		delete[] path;
		path = nullptr;

		if (fp == nullptr) return false;

		int area = chunk_size * chunk_size;

		ChunkHeader ch;
		ch.x = chunk_x;
		ch.z = chunk_z;

		fwrite(&ch, sizeof(ChunkHeader), 1, fp);

		for (int layer = 0; layer < chunk_height; layer++) {
			LayerHeader lh;
			lh.layer = layer;
			if (is_data_flat(&data[layer * area], area)) {
				lh.storing = STORE_FLAT;
				fwrite(&lh, sizeof(LayerHeader), 1, fp);
				fwrite(&data[layer * area], sizeof(unsigned short int), 1, fp);
			}
			else {
				lh.storing = STORE_FULL;
				fwrite(&lh, sizeof(LayerHeader), 1, fp);
				fwrite(&data[layer * area], sizeof(unsigned short int), area, fp);
			}
		}

		fclose(fp);
		return true;
	}

	// Saves tickable blocks
	void saveChunkTData(TickableBlock* buffer, size_t item_count, int chunk_x, int chunk_z) {
		if (!folder_availabe) return;

		int len = strlen(save_folder);
		char* path = new char[len + 64LL];
		sprintf(path, "%s%08x%08x0.bin", save_folder, chunk_x, chunk_z);

		FILE* fp;
		fp = fopen(path, "wb");
		delete[] path;
		path = nullptr;

		if (fp == nullptr) return;
		
		fwrite(&item_count, sizeof(size_t), 1, fp);
		fwrite(buffer, sizeof(TickableBlock), item_count, fp);

		fclose(fp);
		return;
	}

	// Loads the chunk data from the storage ('data' size is not being checked in the method, be aware).
	bool loadChunkData(unsigned short int* data, int chunk_x, int chunk_z, int chunk_size, int chunk_height) {

		int area = chunk_size * chunk_size;

		for (int i = 0; i < area * chunk_height; i++) {
			data[i] = 65535;
		}

		if (!folder_availabe) return false;

		int len = strlen(save_folder);
		char* path = new char[len + 64LL];
		sprintf(path, "%s%08x%08x.bin", save_folder, chunk_x, chunk_z);

		FILE* fp;
		fp = fopen(path, "rb");

		if (fp == nullptr) return false;

		ChunkHeader ch;
		fread(&ch, sizeof(ChunkHeader), 1, fp);
		for (int layer = 0; layer < chunk_height; layer++) {
			LayerHeader lh;
			fread(&lh, sizeof(LayerHeader), 1, fp);
			if (lh.storing == STORE_FLAT) {
				unsigned short int all;
				fread(&all, sizeof(unsigned short int), 1, fp);
				for (int i = 0; i < area; i++) {
					data[layer * area + i] = all;
				}
			}
			else if (lh.storing == STORE_FULL) {
				fread(&data[layer * area], sizeof(unsigned short int), area, fp);
			}

		}
		
		fclose(fp);
		return true;
	}

	// Loads tickable blocks (Allocates data, call 'delete[] buffer;' after you are done.
	size_t loadChunkTData(TickableBlock** buffer, int chunk_x, int chunk_z) {
		if (!folder_availabe) return 0;

		int len = strlen(save_folder);
		char* path = new char[len + 64LL];
		sprintf(path, "%s%08x%08x0.bin", save_folder, chunk_x, chunk_z);

		FILE* fp;
		fp = fopen(path, "rb");
		delete[] path;
		path = nullptr;

		if (fp == nullptr) return 0;

		size_t item_count = 0;

		fread(&item_count, sizeof(size_t), 1, fp);

		(*buffer) = new TickableBlock[item_count];

		fread(*buffer, sizeof(TickableBlock), item_count, fp);

		fclose(fp);
		return item_count;
	}

private:

	const int STORE_FULL = 0;

	const int STORE_FLAT = 1;

	void verify_folder() {
		std::filesystem::path folderPath(save_folder);

		if (std::filesystem::exists(folderPath) && std::filesystem::is_directory(folderPath)) {
			folder_availabe = true;
			if (debug_messages)
				std::cout << "The game save folder was found. " << std::endl;
		}
		else if (!std::filesystem::exists(folderPath)) {
			if (std::filesystem::create_directories(save_folder)) {
				folder_availabe = true;
				if (debug_messages)
					std::cout << "The game save folder was created successfuly." << std::endl;
			}
			else {
				folder_availabe = false;
				if (debug_messages)
					std::cout << "The program failed to create the game save folder." << std::endl;
			}
		}
		else {
			folder_availabe = false;
			if (debug_messages)
				std::cout << "The specified path for game save file exists but it is not a folder." << std::endl;
		}
	}

	bool is_data_flat(unsigned short int* start, int len) {
		unsigned short int beg = start[0];
		for (int i = 0; i < len; i++) {
			if (start[i] != beg)
				return false;
		}
		return true;
	}

	struct LayerHeader {
		char L = 'L';
		int layer = 0;
		int storing = 0;
	};

	struct ChunkHeader
	{
		int ver = 1;
		int x = 0;
		int z = 0;
	};

	const char* save_folder;

	bool debug_messages;

	bool folder_availabe;

};
