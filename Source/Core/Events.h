#pragma once

#include "GFX\Renderer.h"
#include "GFX\Camera.h"

struct GlobalUBO {
	glm::mat4 view;
	glm::mat4 invView;
	glm::mat4 proj;
};

struct UpdateEvent {
	const float elapsedTime;
	const float deltaTime;
	const uint32_t frameIndex;
	const class InputSystem& input;
	const Camera& mainCamera;
};

struct TickEvent {
	const float elapsedTime;
	const class InputSystem& input;
	const Camera& mainCamera;
};

struct RenderEvent {
	const float elapsedTime;
	const uint32_t frameIndex;
	const VkCommandBuffer commandBuffer;
	const Renderer::Pass& pass;
	const std::string passName;
	const uint32_t subpass;
	GlobalUBO& ubo;
	const VkDescriptorSet globalSet;
	const Camera& mainCamera;
};