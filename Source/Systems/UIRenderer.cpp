#include "UIRenderer.h"
#include "GFX\CameraController.h"
#include "Block\ChunkMesh.h"
#include "Block\Block.h"

struct RectPush {
	glm::mat4 transform;
	uint32_t texID;
	float uvScale = 1.f;
	alignas(8) glm::vec2 uvOffset;
};

//TODO: better way of doing this
const std::vector<std::string> uiImages = {
	"Resources\\crosshair.png",
	"Resources\\atlas.png",
	"Resources\\hotbar.png"
};

UIRenderer::UIRenderer(Device& device, Renderer& renderer, PipelineCache& cache, CameraController& camera) : RenderSystem(renderer), device(device), camera(camera) {
	RegisterRenderHandler("UI", 0, 10.f, &UIRenderer::RenderUI);

	Texture::SamplerSettings samplerSettings{};
	samplerSettings.filter = VK_FILTER_NEAREST;
	samplerSettings.enableAnisotropy = VK_FALSE;
	for (const auto& imageName : uiImages) {
		images.push_back(Texture::Load(device, imageName, VK_IMAGE_USAGE_SAMPLED_BIT, Device::QueueFamilyIndices::Graphics, false, false, true, samplerSettings));
	}

	//Setup the descriptor sets
	localPool = DescriptorPool::Builder(device)
		.SetMaxSets(Swapchain::MAX_FRAMES_IN_FLIGHT)
		.AddPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, MAX_UI_TEXTURES * Swapchain::MAX_FRAMES_IN_FLIGHT)
		.Build();

	VkDescriptorSetLayoutBinding binding{};
	binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	binding.descriptorCount = MAX_UI_TEXTURES;
	binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	VkDescriptorBindingFlagsEXT bindingFlag = VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT_EXT;

	VkDescriptorSetLayoutBindingFlagsCreateInfoEXT extendedInfo{};
	extendedInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO_EXT;
	extendedInfo.bindingCount = 1;
	extendedInfo.pBindingFlags = &bindingFlag;

	VkDescriptorSetLayoutCreateInfo layoutInfo{};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.pNext = &extendedInfo;
	layoutInfo.bindingCount = 1;
	layoutInfo.pBindings = &binding;

	if (vkCreateDescriptorSetLayout(device.GetDevice(), &layoutInfo, nullptr, &localLayout) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create UI descriptor set layout!");
	}

	sets.resize(Swapchain::MAX_FRAMES_IN_FLIGHT);
	for (int i = 0; i < sets.size(); i++) {
		if (localPool->AllocateDescriptorSet(localLayout, sets[i]) != VK_SUCCESS) {
			throw std::runtime_error("Failed to allocate descriptor set!");
		}

		std::vector<VkDescriptorImageInfo> imageInfos;

		for (int j = 0; j < uiImages.size(); j++) {
			imageInfos.push_back(images[j]->GetDescriptorInfo());
		}

		VkWriteDescriptorSet write{};
		write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		write.descriptorCount = uiImages.size();
		write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		write.dstArrayElement = 0;
		write.dstBinding = 0;
		write.dstSet = sets[i];
		write.pImageInfo = imageInfos.data();

		vkUpdateDescriptorSets(device.GetDevice(), 1, &write, 0, nullptr);
	}

	GraphicsPipeline::Settings pipelineSettings{};
	GraphicsPipeline::DefaultSettings(pipelineSettings);
	pipelineSettings.depthStencil.reset();
	pipelineSettings.vertexInput.bindingDescriptions.clear();
	pipelineSettings.vertexInput.attributeDescriptions.clear();
	pipelineSettings.renderPass = renderer["UI"].GetRenderPass();
	pipelineSettings.subpass = 0;
	pipelineSettings.colorBlending.colorAttachments[0].blendEnable = VK_TRUE;
	pipelineSettings.shaders.push_back(Pipeline::Shader{ device, "Shaders\\rect.vert.spv", VK_SHADER_STAGE_VERTEX_BIT });
	pipelineSettings.shaders.push_back(Pipeline::Shader{ device, "Shaders\\rect.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT });

	Pipeline::LayoutSettings layoutSettings{};
	VkPushConstantRange pushRange{};
	pushRange.offset = 0;
	pushRange.size = sizeof(RectPush);
	pushRange.stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS;
	layoutSettings.pushConstants.push_back(pushRange);
	layoutSettings.layouts.push_back(localLayout);

	pipeline = std::make_unique<GraphicsPipeline>(device, layoutSettings, pipelineSettings, cache);
}

UIRenderer::~UIRenderer() {
	vkDestroyDescriptorSetLayout(device.GetDevice(), localLayout, nullptr);
}

void UIRenderer::RenderUI(RenderEvent& event) {
	//Update view projection matrix (in case of screen resize)
	viewProj = glm::translate(glm::scale(glm::mat4{ 1.f }, glm::vec3{ 2.f / renderer.GetExtent().width, 2.f / renderer.GetExtent().height, 1.f }), glm::vec3{ -1.f, -1.f, 0.f });

	pipeline->Bind(event.commandBuffer);
	vkCmdBindDescriptorSets(event.commandBuffer, pipeline->GetBindPoint(), pipeline->GetLayout(), 0, 1, &sets[event.frameIndex], 0, nullptr);

	//TODO: actual OOP heirarchical UI system
	//Crosshair
	RectPush push{};
	push.texID = 0;
	push.transform = glm::scale(glm::mat4{ 1.f }, glm::vec3{ 64.f / renderer.GetExtent().width, 64.f / renderer.GetExtent().height, 0.f });
	vkCmdPushConstants(event.commandBuffer, pipeline->GetLayout(), VK_SHADER_STAGE_ALL_GRAPHICS, 0, sizeof(push), &push);
	vkCmdDraw(event.commandBuffer, 6, 1, 0, 0);

	push.texID = 2;
	push.transform = glm::translate(glm::mat4{ 1.f }, glm::vec3{ (renderer.GetExtent().width - 256.f) / renderer.GetExtent().width,
		(renderer.GetExtent().height - 256.f) / renderer.GetExtent().height, 0.f})
		* glm::scale(glm::mat4{1.f}, glm::vec3{80.f / renderer.GetExtent().width, 80.f / renderer.GetExtent().height, 0.f});
	vkCmdPushConstants(event.commandBuffer, pipeline->GetLayout(), VK_SHADER_STAGE_ALL_GRAPHICS, 0, sizeof(push), &push);
	vkCmdDraw(event.commandBuffer, 6, 1, 0, 0);

	push.texID = 1;
	push.transform = glm::translate(glm::mat4{ 1.f }, glm::vec3{ (renderer.GetExtent().width - 256.f) / renderer.GetExtent().width,
		(renderer.GetExtent().height - 256.f) / renderer.GetExtent().height, 0.f })
		* glm::scale(glm::mat4{ 1.f }, glm::vec3{ 50.f / renderer.GetExtent().width, 50.f / renderer.GetExtent().height, 0.f });
	push.uvScale = 1.f / 8.f;
	push.uvOffset = blocks[camera.SelectedBlock()].textureOffsets[3];
	vkCmdPushConstants(event.commandBuffer, pipeline->GetLayout(), VK_SHADER_STAGE_ALL_GRAPHICS, 0, sizeof(push), &push);
	vkCmdDraw(event.commandBuffer, 6, 1, 0, 0);
}