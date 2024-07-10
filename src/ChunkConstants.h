#pragma once

// Chunk dimensions
#define CHUNK_SIZE 16
#define CHUNK_HEIGHT 512
#define CHUNK_AREA CHUNK_SIZE * CHUNK_SIZE

// When there are less than this number of free chunks, delete out of view chunks from memory.
#define DELETE_CHUNKS_THRESHOLD 20