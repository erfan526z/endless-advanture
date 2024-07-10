#include <vector>
#include <cmath>
#include <cstring>
#include "ChunkDataFile.h"
#include "ChunkThread.h"
#include "ChunkGenerator.h"

Queue<Chunk*>* load_queue;
Queue<Chunk*>* save_queue;
Queue<Chunk*>* mesh_queue;

bool active = false;

void loadOrGenerate(Chunk* chunk);

void remeshChunk(Chunk* chunk);

void saveAndFreeChunk(Chunk* chunk);

FastNoiseLite noisegen[16];

char* path;

int chunkManagerThread()
{
	load_queue = new Queue<Chunk*>;
	save_queue = new Queue<Chunk*>;
	mesh_queue = new Queue<Chunk*>;

	active = true;
	
	while (active) {

		bool action_done = false;

		if (!mesh_queue->isEmpty()) {
			Chunk* chunk;
			mesh_queue->dequeue(chunk);
			remeshChunk(chunk);
			action_done = true;
		}

		if (!load_queue->isEmpty()) {
			Chunk* chunk;
			load_queue->dequeue(chunk);
			loadOrGenerate(chunk);
			action_done = true;
		}

		if (!save_queue->isEmpty()) {
			Chunk* chunk;
			save_queue->dequeue(chunk);
			saveAndFreeChunk(chunk);
			action_done = true;
		}

		if (!action_done) std::this_thread::sleep_for(std::chrono::milliseconds(20));
	}

	while (!save_queue->isEmpty()) {
		Chunk* chunk;
		save_queue->dequeue(chunk);
		saveAndFreeChunk(chunk);
	}

	delete load_queue;
	load_queue = nullptr;
	delete save_queue;
	save_queue = nullptr;
	delete mesh_queue;
	mesh_queue = nullptr;

	return 0;
}

void chunk_thread::initManagerThread(const char* world_name, const char* save_addr, int seeds[16])
{
	{
		// Average Area Temperature
		noisegen[0].SetSeed(seeds[0]);
		noisegen[0].SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2S);
		noisegen[0].SetFrequency(0.001f);

		// Average Area Rainfall
		noisegen[1].SetSeed(seeds[1]);
		noisegen[1].SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2S);
		noisegen[1].SetFrequency(0.001f);

		// Biome Gen Noise / Plant Gen Noise
		noisegen[2].SetSeed(seeds[2]);
		noisegen[2].SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2S);
		noisegen[2].SetFrequency(0.035f);

		// Plant Decider A
		noisegen[3].SetSeed(seeds[3]);
		noisegen[3].SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2S);
		noisegen[3].SetFrequency(2.0f);

		// Plant Decider B
		noisegen[4].SetSeed(seeds[4]);
		noisegen[4].SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2S);
		noisegen[4].SetFrequency(0.01f);

		noisegen[5].SetSeed(seeds[5]);
		noisegen[5].SetNoiseType(FastNoiseLite::NoiseType_ValueCubic);

		noisegen[6].SetSeed(seeds[6]);
		noisegen[6].SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2S);

		// Low Errosion Height Generator
		noisegen[7].SetSeed(seeds[7]);
		noisegen[7].SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2S);
		noisegen[7].SetFrequency(0.0035f);
		noisegen[7].SetFractalType(FastNoiseLite::FractalType_FBm);
		noisegen[7].SetFractalOctaves(6);
		noisegen[7].SetFractalLacunarity(2.0f);
		noisegen[7].SetFractalGain(0.5f);

		// High Errosion Height Generator
		noisegen[8].SetSeed(seeds[8]);
		noisegen[8].SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2S);
		noisegen[8].SetFrequency(0.003f);
		noisegen[8].SetFractalType(FastNoiseLite::FractalType_FBm);
		noisegen[8].SetFractalOctaves(2);
		noisegen[8].SetFractalLacunarity(2.1f);
		noisegen[8].SetFractalGain(0.3f);

		// Location Base Height Generator
		noisegen[9].SetSeed(seeds[9]);
		noisegen[9].SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2S);
		noisegen[9].SetFrequency(0.0003f);
		noisegen[9].SetFractalType(FastNoiseLite::FractalType_FBm);
		noisegen[9].SetFractalOctaves(1);

		// Errosion Factor
		noisegen[10].SetSeed(seeds[10]);
		noisegen[10].SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2S);
		noisegen[10].SetFrequency(0.0003f);
		noisegen[10].SetFractalType(FastNoiseLite::FractalType_FBm);
		noisegen[10].SetFractalOctaves(1);

		noisegen[14].SetSeed(seeds[14]);
		noisegen[14].SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2S);
		noisegen[14].SetFrequency(0.001f);

		noisegen[15].SetSeed(seeds[15]);
		noisegen[15].SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2S);
		noisegen[15].SetFrequency(0.001f);

		setNoiseGenerators(noisegen);
	}
	int len = strlen(world_name) + strlen(save_addr) + 4;
	path = new char[len];
	sprintf(path, "%s%s/", save_addr, world_name);
}

void chunk_thread::enqueueLoadRequest(Chunk* chunk)
{
	chunk->setLoadRequested();
	if (load_queue)
		load_queue->enqueue(chunk);
}

void chunk_thread::enqueueSaveRequest(Chunk* chunk)
{

	chunk->setUnloadRequested();
	if (save_queue)
		save_queue->enqueue(chunk);
}

void chunk_thread::enqueueMeshRequest(Chunk* chunk)
{
	chunk->setMeshUpdateRequested();
	if (mesh_queue)
		mesh_queue->enqueue(chunk);
}

void chunk_thread::saveAndKill(Chunk* chunklist, int len)
{
	for (int i = 0; i < len; i++) {
		if (chunklist[i].isFree())
			continue;
		chunklist[i].setUnloadRequested();
		if (save_queue)
			save_queue->enqueue(&chunklist[i]);
	}
	active = false;
}

bool chunk_thread::isInitialized()
{
	return active;
}

void loadOrGenerate(Chunk* chunk)
{
	int well = CHUNK_AREA * CHUNK_HEIGHT;
	int cstx = chunk->getChunkX() * 16;
	int cstz = chunk->getChunkZ() * 16;
	unsigned short int* data = new unsigned short int[well];

	ChunkDataFile cdf = ChunkDataFile(path);
	if (cdf.loadChunkData(data, chunk->getChunkX(), chunk->getChunkZ(), CHUNK_SIZE, CHUNK_HEIGHT)) {
		TickableBlock* tbdat = nullptr;
		size_t ln = cdf.loadChunkTData(&tbdat, chunk->getChunkX(), chunk->getChunkZ());
		if(tbdat && ln != 0)
		{
			for (int i = 0; i < ln; i++)
				chunk->getTickableBlocksPointer()->push_back(tbdat[i]);
			delete[] tbdat;
		}
		chunk->loadRequestResponse(data);
		return;
	}

	generateChunk(data, CHUNK_SIZE, CHUNK_HEIGHT, cstx, cstz, chunk->public_chunk_time_stamp);
	generateTickableChunkBlocks(chunk->getTickableBlocksPointer(), data, CHUNK_SIZE, CHUNK_HEIGHT);
	chunk->loadRequestResponse(data);
}

void createTopFace(float* dst, int x, int y, int z, int highest, float s, unsigned short int block, float max_y = 1.0f) {
	float light = (y >= highest) ? 1.0f : 0.5f;
	float sa, sb;
	int blt = gamedata::blocks.indexer[block]->getBlockTexture(gamedata::DIRECTION_TOP);
	sa = s * (blt / 32); 
	sb = s * (blt % 32);
	float cv[] = {
		x    , y + max_y, z    , sb    , sa + s, light,
		x + 1, y + max_y, z + 1, sb + s, sa    , light,
		x    , y + max_y, z + 1, sb    , sa    , light,
		x    , y + max_y, z    , sb    , sa + s, light,
		x + 1, y + max_y, z    , sb + s, sa + s, light,
		x + 1, y + max_y, z + 1, sb + s, sa    , light
	};
	memcpy(dst, cv, 36 * sizeof(float));
}

void createBottomFace(float* dst, int x, int y, int z, int highest, float s, unsigned short int block) {
	float light = 0.3f;
	float sa, sb;
	int blt = gamedata::blocks.indexer[block]->getBlockTexture(gamedata::DIRECTION_BOTTOM);
	sa = s * (blt / 32);
	sb = s * (blt % 32);
	float cv[] = {
		x    , y    , z    , sb    , sa + s, light,
		x + 1, y    , z + 1, sb + s, sa    , light,
		x    , y    , z + 1, sb    , sa    , light,
		x    , y    , z    , sb    , sa + s, light,
		x + 1, y    , z    , sb + s, sa + s, light,
		x + 1, y    , z + 1, sb + s, sa    , light
	};
	memcpy(dst, cv, 36 * sizeof(float));
}

void createNegativeXFace(float* dst, int x, int y, int z, int highest, float s, unsigned short int block, float max_y = 1.0f) {
	float light = (y >= highest) ? 0.8f : 0.4f;
	float sa, sb;
	int blt = gamedata::blocks.indexer[block]->getBlockTexture(gamedata::DIRECTION_NEGATIVE_X);
	sa = s * (blt / 32);
	sb = s * (blt % 32);
	float cv[] = {
		x    , y + max_y, z    , sb + s, sa    , light,
		x    , y    , z    , sb + s, sa + s, light,
		x    , y + max_y, z + 1, sb    , sa    , light,
		x    , y + max_y, z + 1, sb    , sa    , light,
		x    , y    , z    , sb + s, sa + s, light,
		x    , y    , z + 1, sb    , sa + s, light
	};
	memcpy(dst, cv, 36 * sizeof(float));
}

void createPositiveXFace(float* dst, int x, int y, int z, int highest, float s, unsigned short int block, float max_y = 1.0f) {
	float light = (y >= highest) ? 0.8f : 0.4f;
	float sa, sb;
	int blt = gamedata::blocks.indexer[block]->getBlockTexture(gamedata::DIRECTION_POSITIVE_X);
	sa = s * (blt / 32);
	sb = s * (blt % 32);
	float cv[] = {
		x + 1, y + max_y, z    , sb + s, sa    , light,
		x + 1, y    , z    , sb + s, sa + s, light,
		x + 1, y + max_y, z + 1, sb    , sa    , light,
		x + 1, y + max_y, z + 1, sb    , sa    , light,
		x + 1, y    , z    , sb + s, sa + s, light,
		x + 1, y    , z + 1, sb    , sa + s, light
	};
	memcpy(dst, cv, 36 * sizeof(float));
}

void createNegativeZFace(float* dst, int x, int y, int z, int highest, float s, unsigned short int block, float max_y = 1.0f) {
	float light = (y >= highest) ? 0.7f : 0.35f;
	float sa, sb;
	int blt = gamedata::blocks.indexer[block]->getBlockTexture(gamedata::DIRECTION_NEGATIVE_Z);
	sa = s * (blt / 32);
	sb = s * (blt % 32);
	float cv[] = {
		x + 1, y + max_y, z    , sb + s, sa    , light,
		x    , y    , z    , sb    , sa + s, light,
		x    , y + max_y, z    , sb    , sa    , light,
		x + 1, y    , z    , sb + s, sa + s, light,
		x    , y    , z    , sb    , sa + s, light,
		x + 1, y + max_y, z    , sb + s, sa    , light
	};
	memcpy(dst, cv, 36 * sizeof(float));
}

void createPositiveZFace(float* dst, int x, int y, int z, int highest, float s, unsigned short int block, float max_y = 1.0f) {
	float light = (y >= highest) ? 0.7f : 0.35f;
	float sa, sb;
	int blt = gamedata::blocks.indexer[block]->getBlockTexture(gamedata::DIRECTION_POSITIVE_Z);
	sa = s * (blt / 32);
	sb = s * (blt % 32);
	float cv[] = {
		x + 1, y + max_y, z + 1, sb + s, sa    , light,
		x    , y    , z + 1, sb    , sa + s, light,
		x    , y + max_y, z + 1, sb    , sa    , light,
		x + 1, y    , z + 1, sb + s, sa + s, light,
		x    , y    , z + 1, sb    , sa + s, light,
		x + 1, y + max_y, z + 1, sb + s, sa    , light
	};
	memcpy(dst, cv, 36 * sizeof(float));
}

void createDiagonalFaces(float* dst, int x, int y, int z, int highest, float s, unsigned short int block) {
	constexpr float le = 0.8571428657f; // Lower Bound
	constexpr float he = 0.1428571492f; // Upper Bound
	float light = (y >= highest) ? 0.7f : 0.35f;
	float sa, sb;
	int blt = gamedata::blocks.indexer[block]->getBlockTexture(gamedata::DIRECTION_POSITIVE_Z);
	sa = s * (blt / 32);
	sb = s * (blt % 32);
	float cv[] = {
		x + he, y + 1, z + le, sb + s, sa    , light,
		x + le, y    , z + he, sb    , sa + s, light,
		x + le, y + 1, z + he, sb    , sa    , light,
		x + he, y    , z + le, sb + s, sa + s, light,
		x + le, y    , z + he, sb    , sa + s, light,
		x + he, y + 1, z + le, sb + s, sa    , light,

		x + he, y + 1, z + he, sb + s, sa    , light,
		x + le, y    , z + le, sb    , sa + s, light,
		x + le, y + 1, z + le, sb    , sa    , light,
		x + he, y    , z + he, sb + s, sa + s, light,
		x + le, y    , z + le, sb    , sa + s, light,
		x + he, y + 1, z + he, sb + s, sa    , light
	};
	memcpy(dst, cv, 72 * sizeof(float));
}

void remeshChunk(Chunk* chunk)
{

	int low_x, low_y, low_z, high_x, high_y, high_z;
	unsigned short int tempb;
	unsigned short int blt;

	Chunk* chunk_on_xp = chunk->getChunkPointerOnXP();
	Chunk* chunk_on_xn = chunk->getChunkPointerOnXN();
	Chunk* chunk_on_zp = chunk->getChunkPointerOnZP();
	Chunk* chunk_on_zn = chunk->getChunkPointerOnZN();

	if (!chunk_on_xp || chunk_on_xp->isFree() || !chunk_on_xp->isDataAvailable() || chunk_on_xp->isUnloadRequested()) chunk_on_xp = nullptr;
	if (!chunk_on_xn || chunk_on_xn->isFree() || !chunk_on_xn->isDataAvailable() || chunk_on_xn->isUnloadRequested()) chunk_on_xn = nullptr;
	if (!chunk_on_zp || chunk_on_zp->isFree() || !chunk_on_zp->isDataAvailable() || chunk_on_zp->isUnloadRequested()) chunk_on_zp = nullptr;
	if (!chunk_on_zn || chunk_on_zn->isFree() || !chunk_on_zn->isDataAvailable() || chunk_on_zn->isUnloadRequested()) chunk_on_zn = nullptr;

	int max_h = 0;

	float light;
	float s = 1.0f / 32.0f; // Texture atlas thingy
	float sa, sb;

	int*& cheight = chunk->_heights();
	int*& clight_heights = chunk->_light_heights();
	float**& cvertical = chunk->_verticalChunkData();
	bool*& cvertical_flags = chunk->_verticalChunkModified();
	int*& cvertical_size = chunk->_verticalChunkSize();

	if (!cheight) {
		cheight = new int[(CHUNK_SIZE + 2) * (CHUNK_SIZE + 2)];
	}
	if (!clight_heights) {
		clight_heights = new int[(CHUNK_SIZE + 2) * (CHUNK_SIZE + 2)];
	}
	if (!cvertical) {
		cvertical = new float* [CHUNK_HEIGHT / CHUNK_SIZE];
		for (int i = 0; i < CHUNK_HEIGHT / CHUNK_SIZE; i++)
			cvertical[i] = nullptr;
		// New chunk, neighbor chunks need update
		if (chunk_on_xp) chunk_on_xp->setUpdateNeededInAll();
		if (chunk_on_xn) chunk_on_xn->setUpdateNeededInAll();
		if (chunk_on_zp) chunk_on_zp->setUpdateNeededInAll();
		if (chunk_on_zn) chunk_on_zn->setUpdateNeededInAll();
	}
	if (!cvertical_flags) {
		cvertical_flags = new bool[CHUNK_HEIGHT / CHUNK_SIZE];
		for (int i = 0; i < CHUNK_HEIGHT / CHUNK_SIZE; i++)
			cvertical_flags[i] = true; // If cvertical_flags was null, we definitly need to remesh entire chunk
	}
	if (!cvertical_size) {
		cvertical_size = new int[CHUNK_HEIGHT / CHUNK_SIZE];
		for (int i = 0; i < CHUNK_HEIGHT / CHUNK_SIZE; i++)
			cvertical_size[i] = 0;
	}
	
	for (int x = 0; x < CHUNK_SIZE + 2; x++) {
		for (int z = 0; z < CHUNK_SIZE + 2; z++) {
			bool render_height_set = false;
			for (short int y = CHUNK_HEIGHT - 1; y >= 0; y--) {
				if (x == 0 && z > 0 && z <= CHUNK_SIZE && chunk_on_xn) {
					if (chunk_on_xn->getLocalBlock(CHUNK_SIZE - 1, y, z - 1, tempb) && gamedata::blocks.indexer[tempb]->isRenderable()) {
						if (!render_height_set) {
							render_height_set = true;
							cheight[x * (CHUNK_SIZE + 2) + z] = y;
							if (max_h < y) max_h = y;
						}
						if (!gamedata::blocks.indexer[tempb]->hasTransparency()) {
							render_height_set = true;
							clight_heights[x * (CHUNK_SIZE + 2) + z] = y;
							break;
						}
					}
				}
				else if (x == CHUNK_SIZE + 1 && z > 0 && z <= CHUNK_SIZE && chunk_on_xp) {
					if (chunk_on_xp->getLocalBlock(0, y, z - 1, tempb) && gamedata::blocks.indexer[tempb]->isRenderable()) {
						if (!render_height_set) {
							render_height_set = true;
							cheight[x * (CHUNK_SIZE + 2) + z] = y;
							if (max_h < y) max_h = y;
						}
						if (!gamedata::blocks.indexer[tempb]->hasTransparency()) {
							render_height_set = true;
							clight_heights[x * (CHUNK_SIZE + 2) + z] = y;
							break;
						}
					}
				}
				else if (z == 0 && x > 0 && x <= CHUNK_SIZE && chunk_on_zn) {
					if (chunk_on_zn->getLocalBlock(x - 1, y, CHUNK_SIZE - 1, tempb) && gamedata::blocks.indexer[tempb]->isRenderable()) {
						if (!render_height_set) {
							render_height_set = true;
							cheight[x * (CHUNK_SIZE + 2) + z] = y;
							if (max_h < y) max_h = y;
						}
						if (!gamedata::blocks.indexer[tempb]->hasTransparency()) {
							render_height_set = true;
							clight_heights[x * (CHUNK_SIZE + 2) + z] = y;
							break;
						}
					}
				}
				else if (z == CHUNK_SIZE + 1 && x > 0 && x <= CHUNK_SIZE && chunk_on_zp) { 
					if (chunk_on_zp->getLocalBlock(x - 1, y, 0, tempb) && gamedata::blocks.indexer[tempb]->isRenderable()) {
						if (!render_height_set) {
							render_height_set = true;
							cheight[x * (CHUNK_SIZE + 2) + z] = y;
							if (max_h < y) max_h = y;
						}
						if (!gamedata::blocks.indexer[tempb]->hasTransparency()) {
							render_height_set = true;
							clight_heights[x * (CHUNK_SIZE + 2) + z] = y;
							break;
						}
					}
				}
				else if (z > 0 && z <= CHUNK_SIZE && x > 0 && x <= CHUNK_SIZE) {
					if (chunk->getLocalBlock(x - 1, y, z - 1, tempb) && gamedata::blocks.indexer[tempb]->isRenderable()) {
						if (!render_height_set) {
							render_height_set = true;
							cheight[x * (CHUNK_SIZE + 2) + z] = y;
							if (max_h < y) max_h = y;
						}
						if (!gamedata::blocks.indexer[tempb]->hasTransparency()) {
							render_height_set = true;
							clight_heights[x * (CHUNK_SIZE + 2) + z] = y;
							break;
						}
					}
				}
			}
		}
	}

	if (max_h < 512) max_h++;

	chunk->maxHeight() = max_h;

	for (int y_step = 0; y_step < CHUNK_HEIGHT / CHUNK_SIZE; y_step++) {
		if (!cvertical_flags[y_step]) // The vertical section is not updated, so we can skip that
			continue;

		if (max_h < y_step * CHUNK_SIZE) { // The chunk vertical section is updated, but there are no blocks in this section
			if (cvertical[y_step]) {
				delete[] cvertical[y_step];
				cvertical[y_step] = nullptr;
			}
			cvertical_size[y_step] = 0;
			continue;
		}

		bool delete_needed = cvertical[y_step] ? true : false; // If the data exists, we need to replace it.

		float* tempbuffer = new float[147456]; // Maximum Possible Mesh Triangles in a chunk piece (Extremely rare)

		float* templiquidbuffer = new float[36864]; // Liquids should render after other things

		int curr_size = 0;

		int curr_liquid_size = 0;

		int loop_limit = (y_step + 1) * CHUNK_SIZE; // Prevent recalculating

		for (int y = y_step * CHUNK_SIZE; y < loop_limit; y++) {
			low_y = y - 1;
			high_y = y + 1;
			for (int x = 0; x < CHUNK_SIZE; x++) {
				low_x = x - 1;
				high_x = x + 1;
				for (int z = 0; z < CHUNK_SIZE; z++) {
					low_z = z - 1;
					high_z = z + 1;

					if (curr_size >= 147360 || curr_liquid_size >= 36780) // Almost impossible, just in case
						break;

					unsigned short int block;
					if (!chunk->getLocalBlock(x, y, z, block))
						return;
					if (!gamedata::blocks.indexer[block]->isRenderable()) continue;

					if (gamedata::blocks.indexer[block]->getModelType() == gamedata::MODEL_SOLID) {
						chunk->getLocalBlock(x, low_y, z, tempb);
						if (gamedata::blocks.indexer[tempb]->hasTransparency()) { // Down
							createBottomFace(&tempbuffer[curr_size], x, y, z, 0, s, block);
							curr_size += 36;
						}

						chunk->getLocalBlock(x, high_y, z, tempb);
						if (gamedata::blocks.indexer[tempb]->hasTransparency()) { // Up 
							createTopFace(&tempbuffer[curr_size], x, y, z, clight_heights[high_x * (CHUNK_SIZE + 2) + high_z], s, block);
							curr_size += 36;
						}

						tempb = gamedata::blocks.stone.getID();
						if (!chunk->getLocalBlock(low_x, y, z, tempb) && chunk_on_xn)
							chunk_on_xn->getLocalBlock(CHUNK_SIZE - 1, y, z, tempb);
						if (gamedata::blocks.indexer[tempb]->hasTransparency()) { // X-
							createNegativeXFace(&tempbuffer[curr_size], x, y, z, clight_heights[x * (CHUNK_SIZE + 2) + high_z], s, block);
							curr_size += 36;
						}

						tempb = gamedata::blocks.stone.getID();
						if (!chunk->getLocalBlock(high_x, y, z, tempb) && chunk_on_xp)
							chunk_on_xp->getLocalBlock(0, y, z, tempb);
						if (gamedata::blocks.indexer[tempb]->hasTransparency()) { // X+
							createPositiveXFace(&tempbuffer[curr_size], x, y, z, clight_heights[(high_x + 1) * (CHUNK_SIZE + 2) + high_z], s, block);
							curr_size += 36;
						}

						tempb = gamedata::blocks.stone.getID();
						if (!chunk->getLocalBlock(x, y, low_z, tempb) && chunk_on_zn)
							chunk_on_zn->getLocalBlock(x, y, CHUNK_SIZE - 1, tempb);
						if (gamedata::blocks.indexer[tempb]->hasTransparency()) { // Z-
							createNegativeZFace(&tempbuffer[curr_size], x, y, z, clight_heights[high_x * (CHUNK_SIZE + 2) + z], s, block);
							curr_size += 36;
						}

						tempb = gamedata::blocks.stone.getID();
						if (!chunk->getLocalBlock(x, y, high_z, tempb) && chunk_on_zp)
							chunk_on_zp->getLocalBlock(x, y, 0, tempb);
						if (gamedata::blocks.indexer[tempb]->hasTransparency()) { // Z+
							createPositiveZFace(&tempbuffer[curr_size], x, y, z, clight_heights[high_x * (CHUNK_SIZE + 2) + high_z + 1], s, block);
							curr_size += 36;
						}
					}

					if (gamedata::blocks.indexer[block]->getModelType() == gamedata::MODEL_PLANT_2FACE) {
						createDiagonalFaces(&tempbuffer[curr_size], x, y, z, clight_heights[high_x * (CHUNK_SIZE + 2) + high_z], s, block);
						curr_size += 72;
					}

					if (gamedata::blocks.indexer[block]->getModelType() == gamedata::MODEL_SURFACE_ONLY) {
						createTopFace(&tempbuffer[curr_size], x, y, z, clight_heights[high_x * (CHUNK_SIZE + 2) + high_z], s, block, 0.05f);
						curr_size += 36;
					}

					if (gamedata::blocks.indexer[block]->getModelType() == gamedata::MODEL_PLANT_SURFACE_2FACE) {
						createDiagonalFaces(&tempbuffer[curr_size], x, y, z, clight_heights[high_x * (CHUNK_SIZE + 2) + high_z], s, block);
						curr_size += 72;
						createTopFace(&tempbuffer[curr_size], x, y, z, clight_heights[high_x * (CHUNK_SIZE + 2) + high_z], s, block, 0.05f);
						curr_size += 36;
					}
					
					if (gamedata::blocks.indexer[block]->getModelType() == gamedata::MODEL_LIQUID) {
						chunk->getLocalBlock(x, low_y, z, tempb);
						if (!gamedata::blocks.indexer[tempb]->isRenderable()) { // Down
							createBottomFace(&templiquidbuffer[curr_liquid_size], x, y, z, 0, s, block);
							curr_liquid_size += 36;
						}

						chunk->getLocalBlock(x, high_y, z, tempb);
						if (!gamedata::blocks.indexer[tempb]->isRenderable()) { // Up 
							createTopFace(&templiquidbuffer[curr_liquid_size], x, y, z, clight_heights[high_x * (CHUNK_SIZE + 2) + high_z], s, block, 0.9f);
							curr_liquid_size += 36;
						}

						tempb = gamedata::blocks.stone.getID();
						if (!chunk->getLocalBlock(low_x, y, z, tempb) && chunk_on_xn)
							chunk_on_xn->getLocalBlock(CHUNK_SIZE - 1, y, z, tempb);
						if (!gamedata::blocks.indexer[tempb]->isRenderable()) { // X-
							createNegativeXFace(&templiquidbuffer[curr_liquid_size], x, y, z, clight_heights[x * (CHUNK_SIZE + 2) + high_z], s, block, 0.9f);
							curr_liquid_size += 36;
						}

						tempb = gamedata::blocks.stone.getID();
						if (!chunk->getLocalBlock(high_x, y, z, tempb) && chunk_on_xp)
							chunk_on_xp->getLocalBlock(0, y, z, tempb);
						if (!gamedata::blocks.indexer[tempb]->isRenderable()) { // X+
							createPositiveXFace(&templiquidbuffer[curr_liquid_size], x, y, z, clight_heights[(high_x + 1) * (CHUNK_SIZE + 2) + high_z], s, block, 0.9f);
							curr_liquid_size += 36;
						}

						tempb = gamedata::blocks.stone.getID();
						if (!chunk->getLocalBlock(x, y, low_z, tempb) && chunk_on_zn)
							chunk_on_zn->getLocalBlock(x, y, CHUNK_SIZE - 1, tempb);
						if (!gamedata::blocks.indexer[tempb]->isRenderable()) { // Z-
							createNegativeZFace(&templiquidbuffer[curr_liquid_size], x, y, z, clight_heights[high_x * (CHUNK_SIZE + 2) + z], s, block, 0.9f);
							curr_liquid_size += 36;
						}

						tempb = gamedata::blocks.stone.getID();
						if (!chunk->getLocalBlock(x, y, high_z, tempb) && chunk_on_zp)
							chunk_on_zp->getLocalBlock(x, y, 0, tempb);
						if (!gamedata::blocks.indexer[tempb]->isRenderable()) { // Z+
							createPositiveZFace(&templiquidbuffer[curr_liquid_size], x, y, z, clight_heights[high_x * (CHUNK_SIZE + 2) + high_z + 1], s, block, 0.9f);
							curr_liquid_size += 36;
						}
					}

				}
			}
		}

		// Update chunk data
		int total_size = curr_liquid_size + curr_size;

		if (delete_needed) delete[] cvertical[y_step];
		cvertical[y_step] = new float[total_size];
		//std::cout << "Allocating CVERTICAL \"#" << y_step << "\" array for " << chunk->getChunkX() << ", " << chunk->getChunkZ() << std::endl;
		cvertical_size[y_step] = total_size;
		memcpy(&cvertical[y_step][0], tempbuffer, curr_size * sizeof(float));
		memcpy(&cvertical[y_step][curr_size], templiquidbuffer, curr_liquid_size * sizeof(float));
		cvertical_flags[y_step] = false;
		delete[] tempbuffer;
		delete[] templiquidbuffer;
	}

	chunk->meshRequestResponse();
}

void saveAndFreeChunk(Chunk* chunk)
{
	ChunkDataFile cdf = ChunkDataFile(path);
	if (/*chunk->isDataModified() && */chunk->isDataAvailable()) {
		cdf.saveChunkData(chunk->getDataPointer(), chunk->getChunkX(), chunk->getChunkZ(), CHUNK_SIZE, CHUNK_HEIGHT);
		cdf.saveChunkTData(chunk->getTickableBlocksPointer()->data(), chunk->getTickableBlocksPointer()->size(), chunk->getChunkX(), chunk->getChunkZ());
	}
	chunk->wipe();
}
