#include "ChunkRenderer.h"

struct ChunkPushConstants {
	glm::ivec2 pos;
};

ChunkRenderer::ChunkRenderer(Device& device, Renderer& renderer, PipelineCache& cache, VkDescriptorSetLayout globalSetLayout, ChunkManager& chunks)
	: RenderSystem(renderer, this), manager(chunks) {
	RegisterRenderHandler("Global", 0, 10.f, &ChunkRenderer::GlobalRender);

	pool = DescriptorPool::Builder(device)
		.SetMaxSets(Swapchain::MAX_FRAMES_IN_FLIGHT)
		.AddPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, Swapchain::MAX_FRAMES_IN_FLIGHT)
		.Build();

	layout = DescriptorSetLayout::Builder(device)
		.AddBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
		.Build();

	Pipeline::LayoutSettings layoutSettings{};
	VkPushConstantRange range{};
	range.stageFlags = VK_SHADER_STAGE_ALL;
	range.offset = 0;
	range.size = sizeof(ChunkPushConstants);
	layoutSettings.pushConstants.push_back(range);
	layoutSettings.layouts.push_back(globalSetLayout);
	layoutSettings.layouts.push_back(layout->GetLayout());

	GraphicsPipeline::Settings pipelineSettings{};
	GraphicsPipeline::DefaultSettings(pipelineSettings);
	pipelineSettings.renderPass = renderer["Global"].GetRenderPass();
	pipelineSettings.subpass = 0;
	pipelineSettings.shaders.push_back(Pipeline::Shader{ device, "Shaders\\chunk.vert.spv", VK_SHADER_STAGE_VERTEX_BIT });
	pipelineSettings.shaders.push_back(Pipeline::Shader{ device, "Shaders\\chunk.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT });

	pipeline = std::make_unique<GraphicsPipeline>(device, layoutSettings, pipelineSettings, cache);

	pipelineSettings.rasterization.polygonMode = VK_POLYGON_MODE_LINE;
	pipelineSettings.rasterization.lineWidth = 1.f;

	pipelineSettings.shaders.pop_back();
	pipelineSettings.shaders.push_back(Pipeline::Shader{ device, "Shaders\\wireframe.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT });

	layoutSettings.layouts.pop_back();

	wireframePipeline = std::make_unique<GraphicsPipeline>(device, layoutSettings, pipelineSettings, cache);

	Texture::SamplerSettings samplerSettings{};
	samplerSettings.enableAnisotropy = VK_FALSE;
	samplerSettings.filter = VK_FILTER_NEAREST;
	textureAtlas = Texture::Load(device, "Resources\\atlas.png", 0, Device::QueueFamilyIndices::Graphics, false, false, true, samplerSettings);

	sets.resize(Swapchain::MAX_FRAMES_IN_FLIGHT);
	for (int i = 0; i < sets.size(); i++) {
		auto imageInfo = textureAtlas->GetDescriptorInfo();
		DescriptorBuilder(*pool, *layout)
			.WriteImage(0, imageInfo)
			.Build(sets[i]);
	}
}

void ChunkRenderer::GlobalRender(RenderEvent& event) {
	if (wireframe) {
		wireframePipeline->Bind(event.commandBuffer);
		vkCmdBindDescriptorSets(event.commandBuffer, wireframePipeline->GetBindPoint(), wireframePipeline->GetLayout(), 0, 1, &event.globalSet, 0, nullptr);
	}
	else {
		pipeline->Bind(event.commandBuffer);
		vkCmdBindDescriptorSets(event.commandBuffer, pipeline->GetBindPoint(), pipeline->GetLayout(), 0, 1, &event.globalSet, 0, nullptr);
		vkCmdBindDescriptorSets(event.commandBuffer, pipeline->GetBindPoint(), pipeline->GetLayout(), 1, 1, &sets[event.frameIndex], 0, nullptr);
	}

	for (const auto& kv : manager.chunks) {
		glm::ivec2 chunkPos = kv.first;
		chunkPos -= glm::vec2(event.mainCamera.GetPos().x, event.mainCamera.GetPos().z) / float(CHUNK_SIZE);
		if (kv.second->Loaded()){//}&& glm::length(glm::vec2(chunkPos)) < RENDER_DISTANCE) {
			ChunkPushConstants push{};
			push.pos = kv.first;
			vkCmdPushConstants(event.commandBuffer, wireframePipeline->GetLayout(), VK_SHADER_STAGE_ALL, 0, sizeof(push), &push);
			kv.second->Draw(event);
		}
	}
}

void ChunkRenderer::Update(UpdateEvent& event) {
	if (event.input.GetKeyState(GLFW_KEY_T) == InputSystem::Pressed) {
		wireframe = !wireframe;
	}
}