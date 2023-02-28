#pragma once

#include "RenderSystem.h"
#include "ChunkManager.h"
#include "GraphicsPipeline.h"
#include "Descriptors.h"
#include "Texture.h"

class ChunkRenderer : public RenderSystem<ChunkRenderer> {
public:
	ChunkRenderer(Device& device, Renderer& renderer, PipelineCache& cache, VkDescriptorSetLayout globalSetLayout, ChunkManager& chunks);
	~ChunkRenderer() = default;

	void GlobalRender(RenderEvent& event);

	void Update(UpdateEvent& event) override;

private:
	std::unique_ptr<GraphicsPipeline> pipeline;
	std::unique_ptr<GraphicsPipeline> wireframePipeline;
	bool wireframe = false;
	std::unique_ptr<DescriptorPool> pool;
	std::unique_ptr<DescriptorSetLayout> layout;
	std::vector<VkDescriptorSet> sets;
	std::unique_ptr<Texture> textureAtlas;
	ChunkManager& manager;
};