#pragma once

#include "GameData.h"
#include "FastNoiseLite.h"
#include "BlockTicks.h"
#include <vector>

void setNoiseGenerators(FastNoiseLite noisegens[16]);

void generateChunk(unsigned short int* data, int size, int height, int base_x, int base_z, ChunkTimeStamp cts);

void generateTickableChunkBlocks(std::vector<TickableBlock>* tickable_blocks, unsigned short int* data, int size, int height);

int generateSingleBlock(int x, int y, int z, float& temp, float& rain);

double getNoiseResult(int generator_idx, double x, double y, double z);

double getNoiseResult(int generator_idx, double x, double z);

FastNoiseLite& getNoiseGenerator(int idx);
