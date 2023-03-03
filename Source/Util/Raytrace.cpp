#include "Raytrace.h"

bool RectRaytrace(const glm::vec3& pos, const glm::vec3& dir, const glm::vec3& min, const glm::vec3& max, RectHit& hitInfo) {
	glm::vec3 invDir = 1.f / dir;

	//Check for zeros
	if (std::isnan(invDir.x)) invDir.x = 0.f;
	if (std::isnan(invDir.z)) invDir.z = 0.f;

	float tx1 = (min.x - pos.x) * invDir.x;
	float tx2 = (max.x - pos.x) * invDir.x;

	float tmin = std::min(tx1, tx2);
	float tmax = std::max(tx1, tx2);

	float ty1 = (min.y - pos.y) * invDir.y;
	float ty2 = (max.y - pos.y) * invDir.y;

	tmin = std::max(tmin, std::min(ty1, ty2));
	tmax = std::min(tmax, std::max(ty1, ty2));

	float tz1 = (min.z - pos.z) * invDir.z;
	float tz2 = (max.z - pos.z) * invDir.z;

	tmin = std::max(tmin, std::min(tz1, tz2));
	tmax = std::min(tmax, std::max(tz1, tz2));

	if (tmax >= tmin) {
		hitInfo.t = tmin;
		hitInfo.pos = pos + dir * hitInfo.t;
		if (hitInfo.pos.x == min.x) {
			hitInfo.normal = glm::vec3(-1.f, 0.f, 0.f);
		}
		else if (hitInfo.pos.x == max.x) {
			hitInfo.normal = glm::vec3(1.f, 0.f, 0.f);
		}
		else if (hitInfo.pos.y == min.y) {
			hitInfo.normal = glm::vec3(0.f, -1.f, 0.f);
		}
		else if (hitInfo.pos.y == max.y) {
			hitInfo.normal = glm::vec3(0.f, 1.f, 0.f);
		}
		else if (hitInfo.pos.z == min.z) {
			hitInfo.normal = glm::vec3(0.f, 0.f, -1.f);
		}
		else {
			hitInfo.normal = glm::vec3(0.f, 0.f, 1.f);
		}

		return true;
	}

	return false;
}