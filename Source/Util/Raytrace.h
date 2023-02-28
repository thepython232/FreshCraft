#pragma once

#include "Common.h"

struct RectHit {
	glm::vec3 normal;
	float t;
	glm::vec3 pos;
};

bool RectRaytrace(const glm::vec3& pos, const glm::vec3& dir, const glm::vec3& min, const glm::vec3& max, RectHit& hitInfo);