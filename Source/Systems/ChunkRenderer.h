#pragma once

#include "GFX\RenderSystem.h"
#include "Block\ChunkManager.h"
#include "GFX\GraphicsPipeline.h"
#include "Core\Descriptors.h"
#include "GFX\Texture.h"
#include "Core\Buffer.h"

class CameraController;

class ChunkRenderer : public RenderSystem<ChunkRenderer> {
public:
	ChunkRenderer(Device& device, Renderer& renderer, PipelineCache& cache, VkDescriptorSetLayout globalSetLayout, CameraController& camera, ChunkManager& chunks);
	~ChunkRenderer() = default;

	void ShadowRender(RenderEvent& event);
	void GlobalRender(RenderEvent& event);

	void Update(UpdateEvent& event) override;

private:
	std::unique_ptr<GraphicsPipeline> pipeline;
	std::unique_ptr<GraphicsPipeline> wireframePipeline;
	std::unique_ptr<GraphicsPipeline> transparentPipeline;
	std::unique_ptr<GraphicsPipeline> hoverPipeline;
	std::unique_ptr<GraphicsPipeline> shadowPipeline;
	std::unique_ptr<GraphicsPipeline> debugPipeline;
	std::unique_ptr<DescriptorSetLayout> shadowMapLayout;
	std::vector<VkDescriptorSet> shadowMapSets;
	std::vector<std::unique_ptr<Buffer>> shadowUniforms;
	std::unique_ptr<DescriptorSetLayout> shadowLayout;
	std::vector<VkDescriptorSet> shadowSets;
	bool wireframe = false;
	std::unique_ptr<DescriptorPool> pool;
	std::unique_ptr<DescriptorSetLayout> layout;
	std::vector<VkDescriptorSet> sets;
	std::unique_ptr<Texture> textureAtlas;
	ChunkManager& manager;
	CameraController& camera;
};