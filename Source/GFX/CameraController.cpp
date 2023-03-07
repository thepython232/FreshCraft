#include "CameraController.h"
#include "glm/gtx/norm.hpp"
#include "Block\ChunkManager.h"

const float MOVE_SPEED = 0.1f;
const float PAN_SPEED = 3.f;
const float SPRINT_COOLDOWN = 0.1f;
const float SPRINT_BOOST = 6.f;
const float BREAK_COOLDOWN = 0.08f;
const float PLACE_COOLDOWN = 0.08f;
const float MAX_INTERACT_DISTANCE = 8.f;

CameraController::CameraController(Camera& camera, ChunkManager& manager) : camera(camera), manager(manager) {
	selectedBlock = 0;
}

void CameraController::Update(const UpdateEvent& event) {
	if (event.input.GetKeyState(GLFW_KEY_UP) == InputSystem::Released
		|| event.input.GetKeyState(GLFW_KEY_W) == InputSystem::Released) {
		sprintCooldown = 0.f;
		sprinting = false;
	}

	//TODO: if sprinting, change the camera's FOV

	if ((event.input.GetKey(GLFW_KEY_UP) || event.input.GetKey(GLFW_KEY_W)) && sprintCooldown < SPRINT_COOLDOWN) {
		sprinting = true;
	}
	else {
		sprintCooldown += event.deltaTime;
	}

	glm::vec3 forward{ -sin(-camera.GetRot().y), 0.f, cos(-camera.GetRot().y) };
	glm::vec3 right{ forward.z, 0.f, -forward.x };
	glm::vec3 up{ 0.f, 1.f, 0.f };

	acc = glm::vec3{ 0.f };
	if (event.input.GetKey(GLFW_KEY_RIGHT) || event.input.GetKey(GLFW_KEY_D)) {
		acc += right;
	}
	else if (event.input.GetKey(GLFW_KEY_LEFT) || event.input.GetKey(GLFW_KEY_A)) {
		acc -= right;
	}
	if (event.input.GetKey(GLFW_KEY_UP) || event.input.GetKey(GLFW_KEY_W)) {
		acc += forward;
	}
	else if (event.input.GetKey(GLFW_KEY_DOWN) || event.input.GetKey(GLFW_KEY_S)) {
		acc -= forward;
	}
	if (event.input.GetKey(GLFW_KEY_SPACE)) {
		acc += up;
	}
	else if (event.input.GetKey(GLFW_KEY_LEFT_SHIFT)) {
		acc -= up;
	}

	if (glm::length2(acc) > 0.f)
		acc = glm::normalize(acc);

	acc *= 0.15f;
	if (sprinting) acc *= SPRINT_BOOST;

	if (event.input.GetButton(GLFW_MOUSE_BUTTON_LEFT) && breakCooldown > BREAK_COOLDOWN) {
		breakCooldown = 0.f;

		//Break a block
		glm::vec3 pos = camera.GetPos();
		glm::vec3 dir = camera.Forward();
		BlockHitInfo info{};
		if (manager.VoxelRaytrace(pos, dir, MAX_INTERACT_DISTANCE, info)) {
			manager.BreakBlock(info.blockPos, event);
		}
	}
	else if (event.input.GetButton(GLFW_MOUSE_BUTTON_RIGHT) && breakCooldown > BREAK_COOLDOWN) {
		breakCooldown = 0.f;

		//Break a block
		glm::vec3 pos = camera.GetPos();
		glm::vec3 dir = camera.Forward();
		BlockHitInfo info{};
		if (manager.VoxelRaytrace(pos, dir, MAX_INTERACT_DISTANCE, info)) {
			manager.PlaceBlock(info.blockPos + glm::ivec3(info.normal), selectedBlock + 1, event);
		}
	}
	else {
		breakCooldown += event.deltaTime;
	}

	selectedBlock += int(event.input.GetScrollDelta().y);
	selectedBlock %= blocks.size();

	//if (event.input.GetScrollDelta().y > 0)
	//	std::cout << blocks[selectedBlock].name << std::endl;
}

void CameraController::Tick(const TickEvent& event) {
	glm::vec3 rot = camera.GetRot();
	rot.y += event.input.GetMouseDelta().x / 1000.f * PAN_SPEED;
	rot.x += event.input.GetMouseDelta().y / 1000.f * PAN_SPEED;
	rot.x = glm::clamp(rot.x, -glm::half_pi<float>(), glm::half_pi<float>());
	rot.y = glm::mod(rot.y, glm::two_pi<float>());

	camera.SetRot(rot);

	vel += acc;
	vel *= 0.85f;
	camera.SetPos(camera.GetPos() + vel * MOVE_SPEED);
}

bool CameraController::GetSelectedBlockPos(glm::ivec3& blockPos) const {
	glm::vec3 pos = camera.GetPos();
	glm::vec3 dir = camera.Forward();
	BlockHitInfo info{};
	if (manager.VoxelRaytrace(pos, dir, MAX_INTERACT_DISTANCE, info)) {
		blockPos = info.blockPos - glm::ivec3(CHUNK_SIZE / 2, 0, CHUNK_SIZE / 2);
		return true;
	}
	else {
		return false;
	}
}