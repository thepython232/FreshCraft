#pragma once

#include "Camera.h"
#include "Core\Events.h"

class CameraController {
public:
	CameraController(Camera& camera, class ChunkManager& manager);

	void Update(const UpdateEvent& event);
	void Tick(const TickEvent& event);

private:
	Camera& camera;
	class ChunkManager& manager;
	glm::vec3 vel, acc;
	float sprintCooldown = 0.f;
	bool sprinting = false;
	float breakCooldown = 0.f;
	int selectedBlock = 0;
};