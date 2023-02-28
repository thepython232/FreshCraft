#pragma once

#include "Chunk.h"
#include "Noise.h"

class TerrainGenerator {
public:
	TerrainGenerator() { }

	float HeightAt(const glm::ivec2& block) const {
		return noise.fractal(14, block.x, block.y) * 26.f + 64.f;
	}

private:
	SimplexNoise noise{ 0.006f, 10.f, 2.1f, 0.45f };
};