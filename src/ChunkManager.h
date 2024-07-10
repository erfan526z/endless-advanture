#pragma once

#include <cstring>
#include "ChunkThread.h"
#include "GameData.h"
#include "BlockTicks.h"
#include "ChunkGenerator.h"

struct RenderingChunk {
	unsigned int vao;
	int vbo_length;
	bool finish;
	int cx, cz;
	int chunk_reference;
	float dst;
};

/*
The class for controlling the memory chunks to be used in a world.
Firstly, initialize it using initialize(), then you can use updatePlayer() to detect needed chunks around given position and update() to actually load/unload/remesh chunks.
Remember that update() only updates the chunks that already have been proceed on the chunk thread and does not wait for anything.
DO NOT DEFINE TWO OR MORE INSTANCES.
*/
class ChunkManager
{
public:

	void initialize(const char* datadir, const char* name, int memory_chunks, int render_dist, const char* seed) {
		int tmp = strlen(name);
		int tmpp = tmp + 1;
		world_name = new char[tmpp];
		memcpy(world_name, name, tmpp);
		max_memory_chunks = memory_chunks;
		render_distance = render_dist;
		free_chunks = max_memory_chunks;
		active = true;

		chunk_list = new Chunk[max_memory_chunks];
		render_list = new RenderingChunk[max_memory_chunks];

		int seeds[16];
		hashSeed(seeds, seed);

		chunk_thread::initManagerThread(world_name, datadir, seeds);
		terrainCalculationThread = new std::thread(chunkManagerThread);

		for (int i = 0; i < max_memory_chunks; i++) {
			//chunk_list[i]._activate_no_opengl_debug_mode(); // Only debug
			chunk_list[i].wipe();
		}

		while (!chunk_thread::isInitialized())
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}

	void destroy() {
		active = false;
		saveAndStop();
		terrainCalculationThread->join();
		
		delete terrainCalculationThread;
		terrainCalculationThread = nullptr;

		delete[] chunk_list;
		delete[] render_list;
		delete[] world_name;
	}


	bool chunkExists(int chunk_x, int chunk_z) {
		for (int i = 0; i < max_memory_chunks; i++) {
			if (chunk_list[i].getChunkX() == chunk_x && chunk_list[i].getChunkZ() == chunk_z && chunk_list[i].isDataAvailable()) return true;
		}
		return false;
	}

	/* 
	Checks for the needed chunks around player.
	If there is a chunk in view distance and it is not loaded/loading, occupies a memory space for it.
	Also when there is no space, it will ignore new chunks.
	Remember this method is only for checking and assigning memory, see 'void update()'.
	*/
	void updatePlayer(int x, int y, int z, float yaw = 0.0f) {
		int ccx = getChunkNumber(x);
		int ccz = getChunkNumber(z);

		int chunk_matrix_edge = render_distance * 2 + 1;
		int matrix_area = chunk_matrix_edge * chunk_matrix_edge;
		bool* chunk_need_matrix = new bool[matrix_area];

		// Mark all chunks in render distance as needed in relative chunk need matrix.
		for (int ix = 0; ix < chunk_matrix_edge; ix++) {
			for (int iz = 0; iz < chunk_matrix_edge; iz++) {
				chunk_need_matrix[ix * chunk_matrix_edge + iz] = false;
				int relative_x = ix - render_distance;
				int relative_z = iz - render_distance;
				if (quickAbs(relative_x) + quickAbs(relative_z) <= render_distance)
					chunk_need_matrix[ix * chunk_matrix_edge + iz] = true;
			}
		}

		// Unmark those already exist.
		int free_ons = 0;
		for (int i = 0; i < max_memory_chunks; i++) {
			if (chunk_list[i].isFree()) {
				free_ons++;
				continue;
			}
			int xc = chunk_list[i].getChunkX() - ccx + render_distance;
			int zc = chunk_list[i].getChunkZ() - ccz + render_distance;

			if (xc >= 0 && xc < chunk_matrix_edge && zc >= 0 && zc < chunk_matrix_edge) {
				chunk_need_matrix[xc * chunk_matrix_edge + zc] = false;
			}
		}

		// Occupy new chunks for any needed chunks (if possible, else just ignore until there's a free space for new chunks)
		// On each needed chunk (ix, iz), iterrates on main list to find an empty slot, if it reachs the end of list, it will ignore the rest for now.
		int chunk_index = 0;
		bool failure = false;
		for (int ix = 0; ix < chunk_matrix_edge; ix++) {
			if (failure) break;
			for (int iz = 0; iz < chunk_matrix_edge; iz++) {
				if (failure) break;
				if (chunk_need_matrix[ix * chunk_matrix_edge + iz] == false) continue;

				int cx = ix + ccx - render_distance;
				int cz = iz + ccz - render_distance;

				while (!chunk_list[chunk_index].isFree()) {
					chunk_index++;
					if (chunk_index == max_memory_chunks) {
						failure = true;
						break;
					}
				}
				
				if (failure) break;
				// Free chunk found on index 'chunk_index', can be occupied
				free_ons--;
				chunk_list[chunk_index].wipe();
				chunk_list[chunk_index].init(cx, cz);
			}
		}

		free_chunks = free_ons;
		delete[] chunk_need_matrix;
	}

	/*
	This method manages the loading, updating and removing the chunks on memory.
	Loading: The chunks occupied using 'updatePlayer' method which are not yet loaded, will be queued to load.
	Updating: If the chunk is recently loaded or edited, it will be queued to update the mesh.
	Deleting: When the occupied terrain memory is almost full, out of view chunks will be queued to delete.
	All queues are processed with another thread, and data will be updated
	*/
	void update(ChunkTimeStamp now) {

		// Deleting part
		if (free_chunks < DELETE_CHUNKS_THRESHOLD) {
			for (int index = 0; index < max_memory_chunks; index++) {
				if (chunk_list[index].isFree() ||
					chunk_list[index].isLoadRequested() ||
					chunk_list[index].isMeshUpdateRequested() ||
					chunk_list[index].isUnloadRequested() ||
					chunk_list[index].isOutOfRenderRange() < 10)
					continue;

				chunk_thread::enqueueSaveRequest(&chunk_list[index]);
			}
		}

		// Loading part 
		for (int index = 0; index < max_memory_chunks; index++) {
			if (chunk_list[index].isFree() ||
				chunk_list[index].isDataAvailable() ||
				chunk_list[index].isLoadRequested() ||
				chunk_list[index].isUnloadRequested())
				continue;
			
			chunk_list[index].public_chunk_time_stamp = now;
			chunk_thread::enqueueLoadRequest(&chunk_list[index]);
		}

		// Updating part
		for (int index = 0; index < max_memory_chunks; index++) {
			if (chunk_list[index].isFree() ||
				!chunk_list[index].isDataAvailable() ||
				chunk_list[index].isMeshUpdateRequested() ||
				chunk_list[index].isUnloadRequested() ||
				!chunk_list[index].isDataUpdated() ||
				glfwGetTime() < 1.0)
				continue;

			// If present, update a chunk's nearby chunks
			Chunk* xp = nullptr;
			Chunk* xn = nullptr;
			Chunk* zp = nullptr;
			Chunk* zn = nullptr;
			int cx = chunk_list[index].getChunkX();
			int cz = chunk_list[index].getChunkZ();

			for (int index2 = 0; index2 < max_memory_chunks; index2++) {
				if (chunk_list[index2].isFree() ||
					!chunk_list[index2].isDataAvailable() ||
					chunk_list[index2].isUnloadRequested())
					continue;

				if (chunk_list[index2].getChunkX() - cx == 1 && chunk_list[index2].getChunkZ() - cz == 0) {
					xp = &chunk_list[index2];
				}
				if (chunk_list[index2].getChunkX() - cx == 0 && chunk_list[index2].getChunkZ() - cz == 1) {
					zp = &chunk_list[index2];
				}
				if (chunk_list[index2].getChunkX() - cx == -1 && chunk_list[index2].getChunkZ() - cz == 0) {
					xn = &chunk_list[index2];
				}
				if (chunk_list[index2].getChunkX() - cx == 0 && chunk_list[index2].getChunkZ() - cz == -1) {
					zn = &chunk_list[index2];
				}
			}

			chunk_list[index].setAroundChunkPointers(xn, xp, zn, zp);

			//
			chunk_thread::enqueueMeshRequest(&chunk_list[index]);
		}

		// Updating part (send new mesh to GPU)
		for (int index = 0; index < max_memory_chunks; index++) {
			if (chunk_list[index].isFree() ||
				!chunk_list[index].isNewMeshAvailable())
				continue;

			chunk_list[index].updateVRAM();
		}
	}

	// Processing block ticks for blocks affected by time and environment.
	// This will process ONE chunk at a call (Chunk is determined internally)
	void processBlockTicks(ChunkTimeStamp now) {
		static int index = 0;

		int cidx = render_list[index].chunk_reference;

		if (cidx < max_memory_chunks && cidx >= 0 && !chunk_list[cidx].isFree() && chunk_list[cidx].isDataAvailable()) {
			std::vector<TickableBlock>* tickables = chunk_list[cidx].getTickableBlocksPointer();
			std::vector<TickableBlock>::iterator it;
			for (it = tickables->begin(); it != tickables->end(); ++it) {

				if (it->last_update.year == -1) {
					it->last_update = now;
					continue;
				}

				double delta_t = now.distance_since(it->last_update);
				float rain, temp;
				generateSingleBlock(it->x + chunk_list[cidx].getChunkX() * CHUNK_SIZE, it->y, it->z + chunk_list[cidx].getChunkZ() * CHUNK_SIZE, temp, rain);

				// Strawberry Bush Updates
				if (it->block_id == gamedata::blocks.strawberry_bush_dead.getID() && ((now.day > 2 && now.day < 16) || calculateLocalTemperature(now.time, 0.0, now.day, temp, rain) > 15.0f)) {
					chunk_list[cidx].setLocalBlock(it->x, it->y, it->z, gamedata::blocks.strawberry_bush.getID());
					chunk_list[cidx].setUpdateNeededInLayer(it->y / CHUNK_SIZE);
					it->stat3 = 0.0f;
					it->block_id = gamedata::blocks.strawberry_bush.getID();
				}
				else if (it->block_id == gamedata::blocks.strawberry_bush.getID()) {
					if (calculateLocalTemperature(now.time, 0.0, now.day, temp, rain) > 10.0f) {
						it->stat3 += delta_t;
						if(it->stat5 < 100.0f) it->stat5 = (float)(rand() % 400) + 800.0f;
						if (it->stat3 > it->stat5) {
							chunk_list[cidx].setLocalBlock(it->x, it->y, it->z, gamedata::blocks.strawberry_bush_fruit.getID());
							chunk_list[cidx].setUpdateNeededInLayer(it->y / CHUNK_SIZE);
							it->stat3 = 0.0f;
							it->block_id = gamedata::blocks.strawberry_bush_fruit.getID();
						}
					}
					else {
						it->stat3 -= delta_t;
						if (it->stat3 < 0.0f)
							it->stat3 = 0.0f;
					}
				}
				else if ((it->block_id == gamedata::blocks.strawberry_bush.getID() || it->block_id == gamedata::blocks.strawberry_bush_fruit.getID()) && ((now.day < 2 || now.day > 16) || calculateLocalTemperature(now.time, 0.0, now.day, temp, rain) < -5.0f)) {
					chunk_list[cidx].setLocalBlock(it->x, it->y, it->z, gamedata::blocks.strawberry_bush_dead.getID());
					chunk_list[cidx].setUpdateNeededInLayer(it->y / CHUNK_SIZE);
					it->block_id = gamedata::blocks.strawberry_bush_dead.getID();
					it->stat3 = 0.0f;
				}

				// Dwarf Blueberry Bush Update
				else if (it->block_id == gamedata::blocks.dwarf_blueberry_bush_frozen.getID() && ((now.day > 2 && now.day < 16) || calculateLocalTemperature(now.time, 0.0, now.day, temp, rain) > 5.0f)) {
					chunk_list[cidx].setLocalBlock(it->x, it->y, it->z, gamedata::blocks.dwarf_blueberry_bush.getID());
					chunk_list[cidx].setUpdateNeededInLayer(it->y / CHUNK_SIZE);
					it->block_id = gamedata::blocks.dwarf_blueberry_bush.getID();
					it->stat3 = 0.0f;
				}
				else if (it->block_id == gamedata::blocks.dwarf_blueberry_bush.getID()) {
					if (calculateLocalTemperature(now.time, 0.0, now.day, temp, rain) > 0.0f) {
						it->stat3 += delta_t;
						if (it->stat5 < 100.0f) it->stat5 = (float)(rand() % 400) + 800.0f;
						if (it->stat3 > it->stat5) {
							chunk_list[cidx].setLocalBlock(it->x, it->y, it->z, gamedata::blocks.dwarf_blueberry_bush_fruit.getID());
							chunk_list[cidx].setUpdateNeededInLayer(it->y / CHUNK_SIZE);
							it->stat3 = 0.0f;
							it->block_id = gamedata::blocks.dwarf_blueberry_bush_fruit.getID();
						}
					}
					else {
						it->stat3 -= delta_t;
						if (it->stat3 < 0.0f)
							it->stat3 = 0.0f;
					}
				}
				else if ((it->block_id == gamedata::blocks.dwarf_blueberry_bush.getID() || it->block_id == gamedata::blocks.dwarf_blueberry_bush_fruit.getID()) && ((now.day < 2 || now.day > 16) || calculateLocalTemperature(now.time, 0.0, now.day, temp, rain) < -5.0f)) {
					chunk_list[cidx].setLocalBlock(it->x, it->y, it->z, gamedata::blocks.dwarf_blueberry_bush_frozen.getID());
					chunk_list[cidx].setUpdateNeededInLayer(it->y / CHUNK_SIZE);
					it->block_id = gamedata::blocks.dwarf_blueberry_bush_frozen.getID();
					it->stat3 = 0.0f;
				}

				// Bearberry Bush Update
				else if (it->block_id == gamedata::blocks.bearberry_bush_frozen.getID() && ((now.day > 2 && now.day < 16) || calculateLocalTemperature(now.time, 0.0, now.day, temp, rain) > 5.0f)) {
					chunk_list[cidx].setLocalBlock(it->x, it->y, it->z, gamedata::blocks.bearberry_bush.getID());
					chunk_list[cidx].setUpdateNeededInLayer(it->y / CHUNK_SIZE);
					it->block_id = gamedata::blocks.bearberry_bush.getID();
					it->stat3 = 0.0f;
				}
				else if (it->block_id == gamedata::blocks.bearberry_bush.getID()) {
					if (calculateLocalTemperature(now.time, 0.0, now.day, temp, rain) > 0.0f) {
						it->stat3 += delta_t;
						if (it->stat5 < 100.0f) it->stat5 = (float)(rand() % 400) + 800.0f;
						if (it->stat3 > it->stat5) {
							chunk_list[cidx].setLocalBlock(it->x, it->y, it->z, gamedata::blocks.bearberry_bush_fruit.getID());
							chunk_list[cidx].setUpdateNeededInLayer(it->y / CHUNK_SIZE);
							it->stat3 = 0.0f;
							it->block_id = gamedata::blocks.bearberry_bush_fruit.getID();
						}
					}
					else {
						it->stat3 -= delta_t;
						if (it->stat3 < 0.0f)
							it->stat3 = 0.0f;
					}
				}
				else if ((it->block_id == gamedata::blocks.bearberry_bush.getID() || it->block_id == gamedata::blocks.bearberry_bush_fruit.getID()) && ((now.day < 2 || now.day > 16) || calculateLocalTemperature(now.time, 0.0, now.day, temp, rain) < -5.0f)) {
					chunk_list[cidx].setLocalBlock(it->x, it->y, it->z, gamedata::blocks.bearberry_bush_frozen.getID());
					chunk_list[cidx].setUpdateNeededInLayer(it->y / CHUNK_SIZE);
					it->block_id = gamedata::blocks.bearberry_bush_frozen.getID();
					it->stat3 = 0.0f;
				}

				// Dwarf Birch Tree
				else if (it->block_id == gamedata::blocks.dwarf_birch_leaves_frosty.getID() && (now.day > 6 && now.day < 15)) {
					chunk_list[cidx].setLocalBlock(it->x, it->y, it->z, gamedata::blocks.dwarf_birch_leaves.getID());
					chunk_list[cidx].setUpdateNeededInLayer(it->y / CHUNK_SIZE);
					it->block_id = gamedata::blocks.dwarf_birch_leaves.getID();
				}
				else if (it->block_id == gamedata::blocks.dwarf_birch_leaves.getID() && (now.day <= 6 || now.day >= 15)) {
					chunk_list[cidx].setLocalBlock(it->x, it->y, it->z, gamedata::blocks.dwarf_birch_leaves_frosty.getID());
					chunk_list[cidx].setUpdateNeededInLayer(it->y / CHUNK_SIZE);
					it->block_id = gamedata::blocks.dwarf_birch_leaves_frosty.getID();
				}

				// Spruce Tree
				else if (it->block_id == gamedata::blocks.spruce_leaves_frosty.getID() && ((now.day > 6 && now.day < 15 && temp < -5.0f) || (now.day > 2 && now.day < 18 && temp >= -5.0f))) {
					chunk_list[cidx].setLocalBlock(it->x, it->y, it->z, gamedata::blocks.spruce_leaves.getID());
					chunk_list[cidx].setUpdateNeededInLayer(it->y / CHUNK_SIZE);
					it->block_id = gamedata::blocks.spruce_leaves.getID();
				}
				else if (it->block_id == gamedata::blocks.spruce_leaves.getID() && (((now.day <= 6 || now.day >= 15) && temp < -5.0f) || ((now.day <= 2 || now.day >= 18) && temp >= -5.0f))) {
					chunk_list[cidx].setLocalBlock(it->x, it->y, it->z, gamedata::blocks.spruce_leaves_frosty.getID());
					chunk_list[cidx].setUpdateNeededInLayer(it->y / CHUNK_SIZE);
					it->block_id = gamedata::blocks.spruce_leaves_frosty.getID();
				}

				// Pine Tree
				else if (it->block_id == gamedata::blocks.pine_leaves_frosty.getID() && ((now.day > 6 && now.day < 15 && temp < -5.0f) || (now.day > 2 && now.day < 18 && temp >= -5.0f))) {
					chunk_list[cidx].setLocalBlock(it->x, it->y, it->z, gamedata::blocks.pine_leaves.getID());
					chunk_list[cidx].setUpdateNeededInLayer(it->y / CHUNK_SIZE);
					it->block_id = gamedata::blocks.pine_leaves.getID();
				}
				else if (it->block_id == gamedata::blocks.pine_leaves.getID() && (((now.day <= 6 || now.day >= 15) && temp < -5.0f) || ((now.day <= 2 || now.day >= 18) && temp >= -5.0f))) {
					chunk_list[cidx].setLocalBlock(it->x, it->y, it->z, gamedata::blocks.pine_leaves_frosty.getID());
					chunk_list[cidx].setUpdateNeededInLayer(it->y / CHUNK_SIZE);
					it->block_id = gamedata::blocks.pine_leaves_frosty.getID();
				}

				// Maple Tree
				else if (it->block_id == gamedata::blocks.maple_leaves_winter_y.getID() && (now.day < 15)) {
					chunk_list[cidx].setLocalBlock(it->x, it->y, it->z, gamedata::blocks.maple_leaves_green_y.getID());
					chunk_list[cidx].setUpdateNeededInLayer(it->y / CHUNK_SIZE);
					it->block_id = gamedata::blocks.maple_leaves_green_y.getID();
				}
				else if (it->block_id == gamedata::blocks.maple_leaves_green_y.getID() && (now.day <= 21 && now.day >= 15)) {
					chunk_list[cidx].setLocalBlock(it->x, it->y, it->z, gamedata::blocks.maple_leaves_yellow.getID());
					chunk_list[cidx].setUpdateNeededInLayer(it->y / CHUNK_SIZE);
					it->block_id = gamedata::blocks.maple_leaves_yellow.getID();
				}
				else if (it->block_id == gamedata::blocks.maple_leaves_yellow.getID() && (now.day > 21)) {
					chunk_list[cidx].setLocalBlock(it->x, it->y, it->z, gamedata::blocks.maple_leaves_winter_y.getID());
					chunk_list[cidx].setUpdateNeededInLayer(it->y / CHUNK_SIZE);
					it->block_id = gamedata::blocks.maple_leaves_winter_y.getID();
				}

				else if (it->block_id == gamedata::blocks.maple_leaves_winter_o.getID() && (now.day < 15)) {
					chunk_list[cidx].setLocalBlock(it->x, it->y, it->z, gamedata::blocks.maple_leaves_green_o.getID());
					chunk_list[cidx].setUpdateNeededInLayer(it->y / CHUNK_SIZE);
					it->block_id = gamedata::blocks.maple_leaves_green_o.getID();
				}
				else if (it->block_id == gamedata::blocks.maple_leaves_green_o.getID() && (now.day <= 21 && now.day >= 15)) {
					chunk_list[cidx].setLocalBlock(it->x, it->y, it->z, gamedata::blocks.maple_leaves_orange.getID());
					chunk_list[cidx].setUpdateNeededInLayer(it->y / CHUNK_SIZE);
					it->block_id = gamedata::blocks.maple_leaves_orange.getID();
				}
				else if (it->block_id == gamedata::blocks.maple_leaves_orange.getID() && (now.day > 21)) {
					chunk_list[cidx].setLocalBlock(it->x, it->y, it->z, gamedata::blocks.maple_leaves_winter_o.getID());
					chunk_list[cidx].setUpdateNeededInLayer(it->y / CHUNK_SIZE);
					it->block_id = gamedata::blocks.maple_leaves_winter_o.getID();
				}

				else if (it->block_id == gamedata::blocks.maple_leaves_winter_r.getID() && (now.day < 15)) {
					chunk_list[cidx].setLocalBlock(it->x, it->y, it->z, gamedata::blocks.maple_leaves_green_r.getID());
					chunk_list[cidx].setUpdateNeededInLayer(it->y / CHUNK_SIZE);
					it->block_id = gamedata::blocks.maple_leaves_green_r.getID();
				}
				else if (it->block_id == gamedata::blocks.maple_leaves_green_r.getID() && (now.day <= 21 && now.day >= 15)) {
					chunk_list[cidx].setLocalBlock(it->x, it->y, it->z, gamedata::blocks.maple_leaves_red.getID());
					chunk_list[cidx].setUpdateNeededInLayer(it->y / CHUNK_SIZE);
					it->block_id = gamedata::blocks.maple_leaves_red.getID();
				}
				else if (it->block_id == gamedata::blocks.maple_leaves_red.getID() && (now.day > 21)) {
					chunk_list[cidx].setLocalBlock(it->x, it->y, it->z, gamedata::blocks.maple_leaves_winter_r.getID());
					chunk_list[cidx].setUpdateNeededInLayer(it->y / CHUNK_SIZE);
					it->block_id = gamedata::blocks.maple_leaves_winter_r.getID();
				}

				// Birch Tree
				else if (it->block_id == gamedata::blocks.birch_leaves_winter_y.getID() && (now.day < 15)) {
					chunk_list[cidx].setLocalBlock(it->x, it->y, it->z, gamedata::blocks.birch_leaves_green_y.getID());
					chunk_list[cidx].setUpdateNeededInLayer(it->y / CHUNK_SIZE);
					it->block_id = gamedata::blocks.birch_leaves_green_y.getID();
				}
				else if (it->block_id == gamedata::blocks.birch_leaves_green_y.getID() && (now.day <= 21 && now.day >= 15)) {
					chunk_list[cidx].setLocalBlock(it->x, it->y, it->z, gamedata::blocks.birch_leaves_yellow.getID());
					chunk_list[cidx].setUpdateNeededInLayer(it->y / CHUNK_SIZE);
					it->block_id = gamedata::blocks.birch_leaves_yellow.getID();
				}
				else if (it->block_id == gamedata::blocks.birch_leaves_yellow.getID() && (now.day > 21)) {
					chunk_list[cidx].setLocalBlock(it->x, it->y, it->z, gamedata::blocks.birch_leaves_winter_y.getID());
					chunk_list[cidx].setUpdateNeededInLayer(it->y / CHUNK_SIZE);
					it->block_id = gamedata::blocks.birch_leaves_winter_y.getID();
				}

				else if (it->block_id == gamedata::blocks.birch_leaves_winter_o.getID() && (now.day < 15)) {
					chunk_list[cidx].setLocalBlock(it->x, it->y, it->z, gamedata::blocks.birch_leaves_green_o.getID());
					chunk_list[cidx].setUpdateNeededInLayer(it->y / CHUNK_SIZE);
					it->block_id = gamedata::blocks.birch_leaves_green_o.getID();
				}
				else if (it->block_id == gamedata::blocks.birch_leaves_green_o.getID() && (now.day <= 21 && now.day >= 15)) {
					chunk_list[cidx].setLocalBlock(it->x, it->y, it->z, gamedata::blocks.birch_leaves_orange.getID());
					chunk_list[cidx].setUpdateNeededInLayer(it->y / CHUNK_SIZE);
					it->block_id = gamedata::blocks.birch_leaves_orange.getID();
				}
				else if (it->block_id == gamedata::blocks.birch_leaves_orange.getID() && (now.day > 21)) {
					chunk_list[cidx].setLocalBlock(it->x, it->y, it->z, gamedata::blocks.birch_leaves_winter_o.getID());
					chunk_list[cidx].setUpdateNeededInLayer(it->y / CHUNK_SIZE);
					it->block_id = gamedata::blocks.birch_leaves_winter_o.getID();
				}

				else if (it->block_id == gamedata::blocks.birch_leaves_winter_r.getID() && (now.day < 15)) {
					chunk_list[cidx].setLocalBlock(it->x, it->y, it->z, gamedata::blocks.birch_leaves_green_r.getID());
					chunk_list[cidx].setUpdateNeededInLayer(it->y / CHUNK_SIZE);
					it->block_id = gamedata::blocks.birch_leaves_green_r.getID();
				}
				else if (it->block_id == gamedata::blocks.birch_leaves_green_r.getID() && (now.day <= 21 && now.day >= 15)) {
					chunk_list[cidx].setLocalBlock(it->x, it->y, it->z, gamedata::blocks.birch_leaves_red.getID());
					chunk_list[cidx].setUpdateNeededInLayer(it->y / CHUNK_SIZE);
					it->block_id = gamedata::blocks.birch_leaves_red.getID();
				}
				else if (it->block_id == gamedata::blocks.birch_leaves_red.getID() && (now.day > 21)) {
					chunk_list[cidx].setLocalBlock(it->x, it->y, it->z, gamedata::blocks.birch_leaves_winter_r.getID());
					chunk_list[cidx].setUpdateNeededInLayer(it->y / CHUNK_SIZE);
					it->block_id = gamedata::blocks.birch_leaves_winter_r.getID();
				}

				// Tallgrass (Normal & Short)
				else if (it->block_id == gamedata::blocks.tallgrass_dead.getID() && (now.day > 2 && now.day < 17 && temp < 25.0f)) {
				chunk_list[cidx].setLocalBlock(it->x, it->y, it->z, gamedata::blocks.tallgrass.getID());
				chunk_list[cidx].setUpdateNeededInLayer(it->y / CHUNK_SIZE);
				it->block_id = gamedata::blocks.tallgrass.getID();
				}
				else if (it->block_id == gamedata::blocks.tallgrass.getID() && (now.day <= 2 || now.day >= 17) && temp < 25.0f) {
				chunk_list[cidx].setLocalBlock(it->x, it->y, it->z, gamedata::blocks.tallgrass_dead.getID());
				chunk_list[cidx].setUpdateNeededInLayer(it->y / CHUNK_SIZE);
				it->block_id = gamedata::blocks.tallgrass_dead.getID();
				}

				else if (it->block_id == gamedata::blocks.tallgrass_short_dead.getID() && (now.day > 2 && now.day < 17 && temp < 25.0f)) {
				chunk_list[cidx].setLocalBlock(it->x, it->y, it->z, gamedata::blocks.tallgrass_short.getID());
				chunk_list[cidx].setUpdateNeededInLayer(it->y / CHUNK_SIZE);
				it->block_id = gamedata::blocks.tallgrass_short.getID();
				}
				else if (it->block_id == gamedata::blocks.tallgrass_short.getID() && (now.day <= 2 || now.day >= 17) && temp < 25.0f) {
				chunk_list[cidx].setLocalBlock(it->x, it->y, it->z, gamedata::blocks.tallgrass_short_dead.getID());
				chunk_list[cidx].setUpdateNeededInLayer(it->y / CHUNK_SIZE);
				it->block_id = gamedata::blocks.tallgrass_short_dead.getID();
				}

				// Lichen & Moss
				else if (it->block_id == gamedata::blocks.lichen_frosty.getID() && ((now.day > 6 && now.day < 15 && temp < -5.0f) || (now.day > 2 && now.day < 18 && temp >= -5.0f))) {
				chunk_list[cidx].setLocalBlock(it->x, it->y, it->z, gamedata::blocks.lichen.getID());
				chunk_list[cidx].setUpdateNeededInLayer(it->y / CHUNK_SIZE);
				it->block_id = gamedata::blocks.lichen.getID();
				}
				else if (it->block_id == gamedata::blocks.lichen.getID() && (  ((now.day <= 6 || now.day >= 15) && temp < -5.0f) || ((now.day <= 2 || now.day >= 18) && temp >= -5.0f)  )) {
				chunk_list[cidx].setLocalBlock(it->x, it->y, it->z, gamedata::blocks.lichen_frosty.getID());
				chunk_list[cidx].setUpdateNeededInLayer(it->y / CHUNK_SIZE);
				it->block_id = gamedata::blocks.lichen_frosty.getID();
				}

				else if (it->block_id == gamedata::blocks.surface_moss_frosty.getID() && ((now.day > 6 && now.day < 15 && temp < -5.0f) || (now.day > 2 && now.day < 18 && temp >= -5.0f))) {
				chunk_list[cidx].setLocalBlock(it->x, it->y, it->z, gamedata::blocks.surface_moss.getID());
				chunk_list[cidx].setUpdateNeededInLayer(it->y / CHUNK_SIZE);
				it->block_id = gamedata::blocks.surface_moss.getID();
				}
				else if (it->block_id == gamedata::blocks.surface_moss.getID() && (((now.day <= 6 || now.day >= 15) && temp < -5.0f) || ((now.day <= 2 || now.day >= 18) && temp >= -5.0f))) {
				chunk_list[cidx].setLocalBlock(it->x, it->y, it->z, gamedata::blocks.surface_moss_frosty.getID());
				chunk_list[cidx].setUpdateNeededInLayer(it->y / CHUNK_SIZE);
				it->block_id = gamedata::blocks.surface_moss_frosty.getID();
				}

				//
				else {
					// If we get here it means no block has been changed
				}
				it->last_update = now;
			}
		}

		index++;
		if (index >= max_memory_chunks || render_list[index].finish) {
			index = 0;
		}
	}

	bool setBlock(int x, int y, int z, unsigned short int block) {
		
		int xc = getChunkNumber(x);
		int zc = getChunkNumber(z);

		int x_neighbor = (x - xc * CHUNK_SIZE == 0) ? -1 : (x - xc * CHUNK_SIZE == CHUNK_SIZE - 1) ? 1 : 0;
		int z_neighbor = (z - zc * CHUNK_SIZE == 0) ? -1 : (z - zc * CHUNK_SIZE == CHUNK_SIZE - 1) ? 1 : 0;

		bool result = false;
		int placed_idx = 0;
		
		for (int index = 0; index < max_memory_chunks; index++) {
			if (!chunk_list[index].isFree() && chunk_list[index].isDataAvailable() && chunk_list[index].getChunkX() == xc && chunk_list[index].getChunkZ() == zc) {
				int lx = x - (xc * CHUNK_SIZE);
				int lz = z - (zc * CHUNK_SIZE);
				result = chunk_list[index].setLocalBlock(lx, y, lz, block);
				chunk_list[index].setUpdateNeededInLayer(y / CHUNK_SIZE);
				placed_idx = index;
				if (y % CHUNK_SIZE == 0) chunk_list[index].setUpdateNeededInLayer(y / CHUNK_SIZE - 1); // The out of range will be handled inside setUpdateNeededInLayer
				if (y % CHUNK_SIZE == CHUNK_SIZE - 1) chunk_list[index].setUpdateNeededInLayer(y / CHUNK_SIZE + 1); // The out of range will be handled inside setUpdateNeededInLayer
			}
			if (x_neighbor && !chunk_list[index].isFree() && chunk_list[index].isDataAvailable() && chunk_list[index].getChunkX() == xc + x_neighbor && chunk_list[index].getChunkZ() == zc) {
				chunk_list[index].setUpdateNeededInLayer(y / CHUNK_SIZE);
			}
			if (z_neighbor && !chunk_list[index].isFree() && chunk_list[index].isDataAvailable() && chunk_list[index].getChunkX() == xc && chunk_list[index].getChunkZ() == zc + z_neighbor) {
				chunk_list[index].setUpdateNeededInLayer(y / CHUNK_SIZE);
			}
		}
	
		if (result) {
			unsigned short int block_read = 0;
			int local_x = x - (xc * CHUNK_SIZE);
			int local_z = z - (zc * CHUNK_SIZE);
			bool exists = false;
			int idx = 0;
			chunk_list[placed_idx].getLocalBlock(local_x, y, local_z, block_read);
			std::vector<TickableBlock>* tblocks = chunk_list[placed_idx].getTickableBlocksPointer();
			std::vector<TickableBlock>::iterator it;
			for (it = tblocks->begin(); it != tblocks->end(); ++it) {
				if (it->x == local_x && it->y == y && it->z == local_z) {
					exists = true;
					break;
				}
				idx++;
			}
			if (gamedata::blocks.isTickable(block_read)) {
				if (exists) {
					tblocks->at(idx).block_id = block_read;
					tblocks->at(idx).stat1 = tblocks->at(idx).stat2 = 0;
					tblocks->at(idx).stat3 = tblocks->at(idx).stat4 = tblocks->at(idx).stat5 = 0.0f; // Reseting all values
					tblocks->at(idx).stat5 = (float)(rand() % 400) + 800.0f;
					//std::cout << "Tickable block changed at (" << x << "," << y << "," << z << ") to " << gamedata::blocks.indexer[block_read]->getNamePtr() << std::endl;
				}
				else
				{
					TickableBlock tb;
					tb.block_id = block_read;
					tb.last_update.year = -1;
					tb.stat1 = tb.stat2 = 0;
					tb.stat3 = tb.stat4 = tb.stat5 = 0.0f;
					tb.stat5 = (float)(rand() % 400) + 800.0f;
					tb.x = local_x;
					tb.y = y;
					tb.z = local_z;
					tblocks->push_back(tb);
					//std::cout << "Tickable block create at (" << x << "," << y << "," << z << "). block name: " << gamedata::blocks.indexer[block_read]->getNamePtr() << std::endl;
				}
			}
			else {
				if (exists) {
					tblocks->erase(tblocks->begin() + idx);
					//std::cout << "Tickable block removed" << std::endl;
				}
				else {
					//std::cout << "Block changed, not tickable block effect" << std::endl;
				}
			}
		}



		return result;
	}

	bool getBlock(int x, int y, int z, unsigned short int &block) {

		int xc = getChunkNumber(x);
		int zc = getChunkNumber(z);

		for (int index = 0; index < max_memory_chunks; index++) {
			if (!chunk_list[index].isFree() && chunk_list[index].isDataAvailable() && chunk_list[index].getChunkX() == xc && chunk_list[index].getChunkZ() == zc) {
				int lx = x - (xc * CHUNK_SIZE);
				int lz = z - (zc * CHUNK_SIZE);
				return chunk_list[index].getLocalBlock(lx, y, lz, block);
			}
		}

		return false;
	}

	void updateRenderList(int x, int y, int z, float yaw = 0.0f) {
		int px = getChunkNumber(x);
		int pz = getChunkNumber(z);
		int iter = 0;

		for (int index = 0; index < max_memory_chunks; index++) {
			if (!chunk_list[index].isFree() && chunk_list[index].isMeshAvailable()) {
				int cx = chunk_list[index].getChunkX();
				int cz = chunk_list[index].getChunkZ();
				int dif = quickAbs(px - cx) + quickAbs(pz - cz);
				if (dif <= render_distance) {
					int vbo_length;
					unsigned int vao;
					chunk_list[index].getRenderInfo(vbo_length, vao);
					chunk_list[index].setIsInRenderRange(true);
					render_list[iter].vbo_length = vbo_length;
					render_list[iter].vao = vao;
					render_list[iter].cx = cx;
					render_list[iter].cz = cz;
					render_list[iter].dst = sqrt((cx - px) * (cx - px) + (cz - pz) * (cz - pz));
					render_list[iter].chunk_reference = index;

					render_list[iter].finish = false;
					iter++;

					for (int c = iter - 1; c > 0; c--) {
						if (render_list[c].dst > render_list[c - 1].dst) {
							RenderingChunk rc = render_list[c];
							render_list[c] = render_list[c - 1];
							render_list[c - 1] = rc;
						}
					}
					
				}
				else {
					chunk_list[index].setIsInRenderRange(false);
				}
			}
		}

		if(iter != max_memory_chunks)
			render_list[iter].finish = true;
		
		render_counter = 0;
	}
	
	bool getRenderInfoFor(unsigned int& vao, int& vbo_length, int& cx, int& cz) {
		vao = render_list[render_counter].vao;
		vbo_length = render_list[render_counter].vbo_length;
		cx = render_list[render_counter].cx;
		cz = render_list[render_counter].cz;
		return render_list[render_counter++].finish;
	}

private:

	char* world_name;

	bool active = false;

	int render_distance;

	int max_memory_chunks;

	int free_chunks;
	
	int render_counter = 0;

	Chunk* chunk_list;

	RenderingChunk* render_list;

	std::thread* terrainCalculationThread;

	int getChunkNumber(int v) {
		return (v < 0) ? ((v + 1) / 16 - 1) : (v / 16);
	}

	int quickAbs(int source) {
		return (source < 0) ? -source : source;
	}

	void saveAndStop() {
		chunk_thread::saveAndKill(chunk_list, max_memory_chunks);
	}

	void hashSeed(int* buffer16x4, const char* seed) {
		for (int i = 0; i < 16; i++)
			buffer16x4[i] = 0;

		for (int i = 0; i < strlen(seed); i++) {
			char r = seed[i];
			// A number in 0b000000 to 0b111111 range
			char seed_char = (r >= 'A' && 'r' <= 'Z') ? r - 'A' : (r >= 'a' && r <= 'z') ? r - 'a' + 26 : (r >= '0' && r <= '9') ? r - '0' + 52 : (r == ' ') ? 62 : 63;
			buffer16x4[0] = (buffer16x4[0] >> 2) + buffer16x4[0] * 41 + seed_char * 2467 + 6337;
			buffer16x4[1] = (buffer16x4[1] >> 2) + buffer16x4[1] * 3727 + seed_char * 3481 + 29363;
			buffer16x4[2] = buffer16x4[2] * 1709 + seed_char * 149 + 23291;
			buffer16x4[3] = (buffer16x4[3] << 1) + buffer16x4[3] * 491 + seed_char * 2999 + 11953;
			buffer16x4[4] = buffer16x4[4] * 397 + seed_char * 2609 + 3907;
			buffer16x4[5] = (buffer16x4[5] >> 3) + buffer16x4[5] * 383 + seed_char * 1423 + 18719;
			buffer16x4[6] = (buffer16x4[6] << 1) + buffer16x4[6] * 1447 + seed_char * 1733 + 14771;
			buffer16x4[7] = (buffer16x4[7] << 2) + buffer16x4[7] * 3917 + seed_char * 1667 + 26309;
			buffer16x4[8] = (buffer16x4[8] << 1) + buffer16x4[8] * 709 + seed_char * 3821 + 31327;
			buffer16x4[9] = buffer16x4[9] * 673 + seed_char * 3163 + 7717;
			buffer16x4[10] = (buffer16x4[10] << 1) + buffer16x4[10] * 1549 + seed_char * 3659 + 32687;
			buffer16x4[11] = buffer16x4[11] * 859 + seed_char * 727 + 9743;
			buffer16x4[12] = (buffer16x4[12] << 1) + buffer16x4[12] * 317 + seed_char * 3037 + 22193;
			buffer16x4[13] = buffer16x4[13] * 2111 + seed_char * 1049 + 8951;
			buffer16x4[14] = buffer16x4[14] * 3449 + seed_char * 3821 + 15901;
			buffer16x4[15] = (buffer16x4[15] >> 1) + buffer16x4[15] * 3359 + seed_char * 3011 + 31121;
		}
	}
};