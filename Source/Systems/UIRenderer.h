#pragma once

#include "GFX\RenderSystem.h"
#include "GFX\GraphicsPipeline.h"
#include "Core\Descriptors.h"
#include "GFX\Texture.h"

constexpr int MAX_UI_TEXTURES = 8;

class UIRenderer : public RenderSystem<UIRenderer> {
public:
	UIRenderer(Device& device, Renderer& renderer, PipelineCache& cache, class CameraController& camera);
	~UIRenderer();

	void RenderUI(RenderEvent& event);

private:
	std::unique_ptr<GraphicsPipeline> pipeline;
	std::unique_ptr<DescriptorPool> localPool;
	VkDescriptorSetLayout localLayout;
	std::vector<VkDescriptorSet> sets;
	std::vector<std::unique_ptr<Texture>> images;
	glm::mat4 viewProj;
	class CameraController& camera;

	Device& device;
};