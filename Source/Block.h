#pragma once

#include "Common.h"
#undef TRANSPARENT

struct Block {
	std::string name;
	glm::vec2 textureOffsets[6];
	enum Flags {
		HOLES = 1 << 0,
		TRANSPARENT = 1 << 1,
		LIQUID = 1 << 2
	} flags;
};