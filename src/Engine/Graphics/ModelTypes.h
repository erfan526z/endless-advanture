#ifndef ENGINE_GRAPHICS_MODEL_TYPES_H
#define ENGINE_GRAPHICS_MODEL_TYPES_H

#include "GraphicsConfig.h"

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

#endif
