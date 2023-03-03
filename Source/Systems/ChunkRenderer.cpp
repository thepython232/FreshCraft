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

	pipelineSettings.rasterization.cullMode = VK_CULL_MODE_NONE;
	pipelineSettings.colorBlending.colorAttachments[0].srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	pipelineSettings.colorBlending.colorAttachments[0].dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	pipelineSettings.colorBlending.colorAttachments[0].blendEnable = VK_TRUE;
	pipelineSettings.colorBlending.colorAttachments[0].srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	pipelineSettings.colorBlending.colorAttachments[0].dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE;

	pipelineSettings.shaders.pop_back();
	pipelineSettings.shaders.push_back(Pipeline::Shader{ device, "Shaders\\transparent.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT });
	pipelineSettings.depthStencil->depthWriteEnable = VK_FALSE;

	transparentPipeline = std::make_unique<GraphicsPipeline>(device, layoutSettings, pipelineSettings, cache);

	pipelineSettings.rasterization.polygonMode = VK_POLYGON_MODE_LINE;
	pipelineSettings.rasterization.cullMode = VK_CULL_MODE_BACK_BIT;
	pipelineSettings.rasterization.lineWidth = 1.f;
	pipelineSettings.depthStencil->depthWriteEnable = VK_TRUE;

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

	//Sort according to the distance from the camera
	std::vector<std::pair<glm::ivec2, ChunkMesh*>> meshes;
	for (const auto& kv : manager.chunks) {
		meshes.push_back(std::make_pair(kv.first, kv.second.get()));
	}

	std::sort(meshes.begin(), meshes.end(), [&event](const std::pair<glm::ivec2, ChunkMesh*>& a, const std::pair<glm::ivec2, ChunkMesh*>& b) {
		return glm::length(glm::vec2(a.first.x * CHUNK_SIZE - event.mainCamera.GetPos().x,
			a.first.y * CHUNK_SIZE - event.mainCamera.GetPos().z))
			> glm::length(glm::vec2(b.first.x * CHUNK_SIZE - event.mainCamera.GetPos().x,
				b.first.y * CHUNK_SIZE  - event.mainCamera.GetPos().z));
		});

	for (const auto& kv : meshes) {
		glm::ivec2 chunkPos = kv.first;
		chunkPos -= glm::vec2(event.mainCamera.GetPos().x, event.mainCamera.GetPos().z) / float(CHUNK_SIZE);
		if (kv.second->Loaded() && glm::length(glm::vec2(chunkPos)) < RENDER_DISTANCE) {
			ChunkPushConstants push{};
			push.pos = kv.first;
			vkCmdPushConstants(event.commandBuffer, pipeline->GetLayout(), VK_SHADER_STAGE_ALL, 0, sizeof(push), &push);
			kv.second->Draw(event);
			//For wireframe view, just render transparent meshes normally
			if (wireframe) {
				kv.second->DrawTransparent(event);
			}
		}
	}

	if (!wireframe) {
		transparentPipeline->Bind(event.commandBuffer);
		vkCmdBindDescriptorSets(event.commandBuffer, transparentPipeline->GetBindPoint(), transparentPipeline->GetLayout(), 0, 1, &event.globalSet, 0, nullptr);
		vkCmdBindDescriptorSets(event.commandBuffer, transparentPipeline->GetBindPoint(), transparentPipeline->GetLayout(), 1, 1, &sets[event.frameIndex], 0, nullptr);

		for (const auto& kv : meshes) {
			glm::ivec2 chunkPos = kv.first;
			chunkPos -= glm::vec2(event.mainCamera.GetPos().x, event.mainCamera.GetPos().z) / float(CHUNK_SIZE);
			if (kv.second->Loaded() && glm::length(glm::vec2(chunkPos)) < RENDER_DISTANCE) {
				ChunkPushConstants push{};
				push.pos = kv.first;
				vkCmdPushConstants(event.commandBuffer, transparentPipeline->GetLayout(), VK_SHADER_STAGE_ALL, 0, sizeof(push), &push);
				kv.second->DrawTransparent(event);
			}
		}
	}
}

void ChunkRenderer::Update(UpdateEvent& event) {
	if (event.input.GetKeyState(GLFW_KEY_T) == InputSystem::Pressed) {
		wireframe = !wireframe;
	}
}