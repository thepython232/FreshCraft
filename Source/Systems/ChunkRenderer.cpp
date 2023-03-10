#include "ChunkRenderer.h"
#include "GFX/CameraController.h"

struct ChunkPushConstants {
	glm::ivec2 pos;
};

struct ShadowUBO {
	glm::mat4 lightTransform;
};

ChunkRenderer::ChunkRenderer(Device& device, Renderer& renderer, PipelineCache& cache, VkDescriptorSetLayout globalSetLayout, CameraController& camera, ChunkManager& chunks)
	: RenderSystem(renderer), manager(chunks), camera(camera) {
	RegisterRenderHandler("Global", 0, 10.f, &ChunkRenderer::GlobalRender);
	RegisterRenderHandler("Shadow", 0, 10.f, &ChunkRenderer::ShadowRender);

	pool = DescriptorPool::Builder(device)
		.SetMaxSets(Swapchain::MAX_FRAMES_IN_FLIGHT * 2 + renderer.GetImageCount())
		.AddPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, Swapchain::MAX_FRAMES_IN_FLIGHT + renderer.GetImageCount())
		.AddPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, Swapchain::MAX_FRAMES_IN_FLIGHT * 2)
		.Build();

	layout = DescriptorSetLayout::Builder(device)
		.AddBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT) //Texture Atlas
		.AddBinding(1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)           //Shadow Uniforms
		.Build();

	shadowMapLayout = DescriptorSetLayout::Builder(device)
		.AddBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT) //Shadow images
		.Build();

	shadowLayout = DescriptorSetLayout::Builder(device)
		.AddBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)           //Shadow Uniforms
		.Build();

	Pipeline::LayoutSettings layoutSettings{};
	VkPushConstantRange range{};
	range.stageFlags = VK_SHADER_STAGE_ALL;
	range.offset = 0;
	range.size = sizeof(ChunkPushConstants);
	layoutSettings.pushConstants.push_back(range);
	layoutSettings.layouts.push_back(globalSetLayout);
	layoutSettings.layouts.push_back(layout->GetLayout());
	layoutSettings.layouts.push_back(shadowMapLayout->GetLayout());

	GraphicsPipeline::Settings pipelineSettings{};
	GraphicsPipeline::DefaultSettings(pipelineSettings);
	pipelineSettings.renderPass = renderer["Global"].GetRenderPass();
	pipelineSettings.subpass = 0;
	pipelineSettings.shaders.push_back(Pipeline::Shader{ device, "Shaders\\chunk.vert.spv", VK_SHADER_STAGE_VERTEX_BIT });
	pipelineSettings.shaders.push_back(Pipeline::Shader{ device, "Shaders\\chunk.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT });

	pipeline = std::make_unique<GraphicsPipeline>(device, layoutSettings, pipelineSettings, cache);

	layoutSettings.layouts.pop_back();

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

	//layoutSettings.layouts.pop_back();

	wireframePipeline = std::make_unique<GraphicsPipeline>(device, layoutSettings, pipelineSettings, cache);

	pipelineSettings.inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
	pipelineSettings.depthStencil->depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
	pipelineSettings.vertexInput.bindingDescriptions.clear();
	pipelineSettings.vertexInput.attributeDescriptions.clear();
	pipelineSettings.rasterization.lineWidth = std::min(1.5f, device.Limits().lineWidthRange[1]);

	pipelineSettings.shaders.clear();
	pipelineSettings.shaders.push_back(Pipeline::Shader{ device, "Shaders\\hover.vert.spv", VK_SHADER_STAGE_VERTEX_BIT });
	pipelineSettings.shaders.push_back(Pipeline::Shader{ device, "Shaders\\hover.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT });

	layoutSettings.pushConstants.clear();
	range.size = sizeof(glm::ivec3);
	layoutSettings.pushConstants.push_back(range);
	
	hoverPipeline = std::make_unique<GraphicsPipeline>(device, layoutSettings, pipelineSettings, cache);

	range.size = sizeof(ChunkPushConstants);
	range.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	layoutSettings.pushConstants[0] = range;
	layoutSettings.layouts.clear();
	layoutSettings.layouts.push_back(shadowLayout->GetLayout());

	GraphicsPipeline::DefaultSettings(pipelineSettings);
	pipelineSettings.shaders.push_back(Pipeline::Shader{ device, "Shaders\\shadow.vert.spv", VK_SHADER_STAGE_VERTEX_BIT });
	pipelineSettings.renderPass = renderer["Shadow"].GetRenderPass();
	pipelineSettings.subpass = 0;
	pipelineSettings.rasterization.cullMode = VK_CULL_MODE_NONE;
	pipelineSettings.rasterization.depthBiasEnable = VK_TRUE;
	pipelineSettings.rasterization.depthBiasConstantFactor = 1.25f;
	pipelineSettings.rasterization.depthBiasSlopeFactor = 1.75f;
	
	shadowPipeline = std::make_unique<GraphicsPipeline>(device, layoutSettings, pipelineSettings, cache);

	layoutSettings.pushConstants.clear();
	layoutSettings.layouts[0] = shadowMapLayout->GetLayout();

	pipelineSettings.shaders.clear();
	pipelineSettings.shaders.push_back(Pipeline::Shader{ device, "Shaders\\preview.vert.spv", VK_SHADER_STAGE_VERTEX_BIT });
	pipelineSettings.shaders.push_back(Pipeline::Shader{ device, "Shaders\\preview.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT });
	pipelineSettings.vertexInput.bindingDescriptions.clear();
	pipelineSettings.vertexInput.attributeDescriptions.clear();
	pipelineSettings.depthStencil->depthTestEnable = VK_FALSE;
	pipelineSettings.depthStencil->depthWriteEnable = VK_FALSE;
	pipelineSettings.rasterization.cullMode = VK_CULL_MODE_BACK_BIT;
	pipelineSettings.renderPass = renderer["Global"].GetRenderPass();
	
	debugPipeline = std::make_unique<GraphicsPipeline>(device, layoutSettings, pipelineSettings, cache);

	//For sampling the shadow depth image
	shadowMapSets.resize(renderer.GetImageCount());
	for (int i = 0; i < shadowMapSets.size(); i++) {
		auto& texture = renderer["Shadow"]["ShadowDepth"].textures[i];
		auto imageInfo = texture->GetDescriptorInfo();
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
		if (DescriptorBuilder(*pool, *shadowMapLayout)
			.WriteImage(0, imageInfo)
			.Build(shadowMapSets[i]) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create debug descriptor sets!");
		}
	}

	for (int i = 0; i < Swapchain::MAX_FRAMES_IN_FLIGHT; i++) {
		shadowUniforms.push_back(std::make_unique<Buffer>(
			device,
			sizeof(ShadowUBO),
			1,
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
			Device::QueueFamilyIndices::Graphics,
			device.Limits().minUniformBufferOffsetAlignment
			));
		shadowUniforms[i]->Map();
	}

	Texture::SamplerSettings samplerSettings{};
	samplerSettings.enableAnisotropy = VK_FALSE;
	samplerSettings.filter = VK_FILTER_NEAREST;
	textureAtlas = Texture::Load(device, "Resources\\atlas.png", 0, Device::QueueFamilyIndices::Graphics, false, false, true, samplerSettings);

	sets.resize(Swapchain::MAX_FRAMES_IN_FLIGHT);
	for (int i = 0; i < sets.size(); i++) {
		auto imageInfo = textureAtlas->GetDescriptorInfo();
		auto bufferInfo = shadowUniforms[i]->GetDescriptorInfo();
		if (DescriptorBuilder(*pool, *layout)
			.WriteImage(0, imageInfo)
			.WriteBuffer(1, bufferInfo)
			.Build(sets[i]) != VK_SUCCESS) {
			throw std::runtime_error("Failed to allocate descriptor sets!");
		}
	}

	shadowSets.resize(Swapchain::MAX_FRAMES_IN_FLIGHT);
	for (int i = 0; i < shadowSets.size(); i++) {
		auto bufferInfo = shadowUniforms[i]->GetDescriptorInfo();
		if (DescriptorBuilder(*pool, *shadowLayout)
			.WriteBuffer(0, bufferInfo)
			.Build(shadowSets[i]) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create shadow descriptor sets!");
		}
	}
}

void ChunkRenderer::ShadowRender(RenderEvent& event) {
	if (wireframe) return; //No shadows for wireframe

	//Configure light transformation
	glm::mat4 lightProj = glm::ortho(-80.f, 80.f, -80.f, 80.f, 0.1f, 400.f);
	lightProj[1][1] = -lightProj[1][1];
	glm::mat4 lightView = glm::lookAt(
		-event.ubo.lightDir * 80.f
		+ event.mainCamera.GetPos(),
		event.mainCamera.GetPos(),
		glm::vec3{ 0.f, 1.f, 0.f }
	);
	glm::mat4 lightTransform = lightProj * lightView;

	shadowPipeline->Bind(event.commandBuffer);

	ShadowUBO shadowUBO{};
	shadowUBO.lightTransform = lightTransform;
	shadowUniforms[event.frameIndex]->WriteToBuffer(&shadowUBO);
	shadowUniforms[event.frameIndex]->Flush();
	vkCmdBindDescriptorSets(
		event.commandBuffer,
		shadowPipeline->GetBindPoint(),
		shadowPipeline->GetLayout(),
		0,
		1, &shadowSets[event.frameIndex],
		0, nullptr
	);

	for (const auto& chunkID : manager.sortedChunks) {
		if (manager.chunks[chunkID]->Loaded()) {
			glm::ivec2 chunkPos = chunkID;
			chunkPos -= glm::vec2(event.mainCamera.GetPos().x, event.mainCamera.GetPos().z) / float(CHUNK_SIZE);
			if (glm::length(glm::vec2(chunkPos)) < RENDER_DISTANCE) {
				ChunkPushConstants push{};
				push.pos = chunkID;
				vkCmdPushConstants(event.commandBuffer, shadowPipeline->GetLayout(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(push), &push);
				ChunkMesh& mesh = *manager.chunks[chunkID];
				mesh.Draw(event);
			}
		}
	}
}

void ChunkRenderer::GlobalRender(RenderEvent& event) {
	if (!manager.sortedChunks.empty()) {
		if (wireframe) {
			wireframePipeline->Bind(event.commandBuffer);
			vkCmdBindDescriptorSets(event.commandBuffer, wireframePipeline->GetBindPoint(), wireframePipeline->GetLayout(), 0, 1, &event.globalSet, 0, nullptr);
		}
		else {
			pipeline->Bind(event.commandBuffer);
			vkCmdBindDescriptorSets(event.commandBuffer, pipeline->GetBindPoint(), pipeline->GetLayout(), 0, 1, &event.globalSet, 0, nullptr);               //Global UBO
			vkCmdBindDescriptorSets(event.commandBuffer, pipeline->GetBindPoint(), pipeline->GetLayout(), 1, 1, &sets[event.frameIndex], 0, nullptr);        //Texture Atlas and Shadow uniforms
			vkCmdBindDescriptorSets(event.commandBuffer, pipeline->GetBindPoint(), pipeline->GetLayout(), 2, 1, &shadowMapSets[renderer.GetImageIndex()], 0, nullptr);        //Shadow Maps
		}

		const glm::vec2 camForward = glm::vec2{ event.mainCamera.Forward().x, event.mainCamera.Forward().z };

		for (auto iter = manager.sortedChunks.rbegin(); iter != manager.sortedChunks.rend(); ++iter) {
			if (manager.chunks[*iter]->Loaded()) {
				glm::ivec2 chunkPos = *iter;
				chunkPos -= glm::vec2(event.mainCamera.GetPos().x, event.mainCamera.GetPos().z) / float(CHUNK_SIZE);
				ChunkMesh& chunk = *manager.chunks[*iter];
				if (glm::length(glm::vec2(chunkPos)) < RENDER_DISTANCE) {
					ChunkPushConstants push{};
					push.pos = *iter;
					vkCmdPushConstants(event.commandBuffer, pipeline->GetLayout(), VK_SHADER_STAGE_ALL, 0, sizeof(push), &push);
					chunk.Draw(event);
					//For wireframe view, just render transparent meshes normally
					if (wireframe) {
						chunk.DrawTransparent(event);
					}
				}
			}
		}

		if (!wireframe) {
			transparentPipeline->Bind(event.commandBuffer);
			vkCmdBindDescriptorSets(event.commandBuffer, transparentPipeline->GetBindPoint(), transparentPipeline->GetLayout(), 0, 1, &event.globalSet, 0, nullptr);
			vkCmdBindDescriptorSets(event.commandBuffer, transparentPipeline->GetBindPoint(), transparentPipeline->GetLayout(), 1, 1, &sets[event.frameIndex], 0, nullptr);

			for (auto iter = manager.sortedChunks.rbegin(); iter != manager.sortedChunks.rend(); ++iter) {
				if (manager.chunks[*iter]->Loaded()) {
					glm::ivec2 chunkPos = *iter;
					chunkPos -= glm::vec2(event.mainCamera.GetPos().x, event.mainCamera.GetPos().z) / float(CHUNK_SIZE);
					ChunkMesh& mesh = *manager.chunks[*iter];
					if (mesh.Loaded() && glm::length(glm::vec2(chunkPos)) < RENDER_DISTANCE) {
						ChunkPushConstants push{};
						push.pos = *iter;
						vkCmdPushConstants(event.commandBuffer, transparentPipeline->GetLayout(), VK_SHADER_STAGE_ALL, 0, sizeof(push), &push);
						mesh.DrawTransparent(event);
					}
				}
			}
		}

		//Draw the block outline
		glm::ivec3 blockPos;
		if (camera.GetSelectedBlockPos(blockPos)) {
			hoverPipeline->Bind(event.commandBuffer);
			vkCmdBindDescriptorSets(event.commandBuffer, hoverPipeline->GetBindPoint(), hoverPipeline->GetLayout(), 0, 1, &event.globalSet, 0, nullptr);
			vkCmdPushConstants(event.commandBuffer, hoverPipeline->GetLayout(), VK_SHADER_STAGE_ALL, 0, sizeof(blockPos), &blockPos);
			vkCmdDraw(event.commandBuffer, 8 * 6, 1, 0, 0);
		}
	}

	/*
	//Render mini-shadow map
	debugPipeline->Bind(event.commandBuffer);
	vkCmdBindDescriptorSets(
		event.commandBuffer,
		debugPipeline->GetBindPoint(),
		debugPipeline->GetLayout(),
		0, 
		1, &shadowMapSets[renderer.GetImageIndex()],
		0, nullptr
	);

	vkCmdDraw(event.commandBuffer, 6, 1, 0, 0);
	*/
}

void ChunkRenderer::Update(UpdateEvent& event) {
	if (event.input.GetKeyState(GLFW_KEY_T) == InputSystem::Pressed) {
		wireframe = !wireframe;
	}
}