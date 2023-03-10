#pragma once

#include "GFX\RenderSystem.h"
#include "GFX\GraphicsPipeline.h"
#include "GFX\Texture.h"
#include "GFX\Mesh.h"
#include "Core\Descriptors.h"

class SimpleRenderSystem : public RenderSystem<SimpleRenderSystem> {
public:
	SimpleRenderSystem(Device& device, Renderer& renderer, PipelineCache& cache, VkDescriptorSetLayout layout);

	void RenderGlobal(RenderEvent& event);

private:
	std::unique_ptr<GraphicsPipeline> pipeline;
	std::unique_ptr<DescriptorPool> setPool;
	std::unique_ptr<DescriptorSetLayout> setLayout;
	std::vector<VkDescriptorSet> sets;
	std::unique_ptr<Mesh> mesh;
	std::unique_ptr<Texture> texture;
};