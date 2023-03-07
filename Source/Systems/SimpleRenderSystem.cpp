#include "SimpleRenderSystem.h"
#include "GFX\Vertex.h"

const std::vector<Vertex> vertices = {
	{ {-0.5f, 0.f, -0.5f}, {0.f, 1.f, 0.f}, {1.f, 0.f, 0.f}, {0.f, 0.f} },
	{ {-0.5f, 0.f, 0.5f}, {0.f, 1.f, 0.f}, {0.f, 1.f, 0.f}, {0.f, 1.f} },
	{ {0.5f, 0.f, -0.5f}, {0.f, 1.f, 0.f}, {0.f, 0.f, 1.f}, {1.f, 0.f} },
	{ {0.5f, 0.f, 0.5f}, {0.f, 1.f, 0.f}, {1.f, 1.f, 1.f}, {1.f, 1.f} },

	{ {-0.5f, -0.5f, -0.5f}, {0.f, -1.f, 0.f}, {1.f, 0.f, 0.f}, {0.f, 0.f} },
	{ {-0.5f, -0.5f, 0.5f}, {0.f, -1.f, 0.f}, {0.f, 1.f, 0.f}, {0.f, 1.f} },
	{ {0.5f, -0.5f, -0.5f}, {0.f, -1.f, 0.f}, {0.f, 0.f, 1.f}, {1.f, 0.f} },
	{ {0.5f, -0.5f, 0.5f}, {0.f, -1.f, 0.f}, {1.f, 1.f, 1.f}, {1.f, 1.f} }
};

const std::vector<uint32_t> indices = {
	0, 1, 2, 3, 2, 1,
	4, 5, 6, 7, 6, 5
};

SimpleRenderSystem::SimpleRenderSystem(Device& device, Renderer& renderer, PipelineCache& cache, VkDescriptorSetLayout globalSetLayout)
	: RenderSystem(renderer) {
	RegisterRenderHandler("Global", 0, 10.f, &SimpleRenderSystem::RenderGlobal);

	setPool = DescriptorPool::Builder(device)
		.SetMaxSets(Swapchain::MAX_FRAMES_IN_FLIGHT)
		.AddPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, Swapchain::MAX_FRAMES_IN_FLIGHT)
		.Build();

	setLayout = DescriptorSetLayout::Builder(device)
		.AddBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
		.Build();

	texture = Texture::Load(
		device,
		"Resources\\statue.jpg",
		VK_IMAGE_USAGE_SAMPLED_BIT,
		Device::QueueFamilyIndices::Graphics
	);

	sets.resize(Swapchain::MAX_FRAMES_IN_FLIGHT);
	for (int i = 0; i < sets.size(); i++) {
		auto imageInfo = texture->GetDescriptorInfo();
		DescriptorBuilder(*setPool, *setLayout)
			.WriteImage(0, imageInfo)
			.Build(sets[i]);
	}

	Pipeline::LayoutSettings layoutSettings{};
	layoutSettings.layouts.push_back(globalSetLayout);
	layoutSettings.layouts.push_back(setLayout->GetLayout());
	GraphicsPipeline::Settings pipelineSettings{};
	GraphicsPipeline::DefaultSettings(pipelineSettings);
	pipelineSettings.renderPass = renderer["Global"].GetRenderPass();
	pipelineSettings.subpass = 0;
	pipelineSettings.shaders.push_back(Pipeline::Shader{ device, "Shaders\\simple.vert.spv", VK_SHADER_STAGE_VERTEX_BIT });
	pipelineSettings.shaders.push_back(Pipeline::Shader{ device, "Shaders\\simple.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT });

	pipeline = std::make_unique<GraphicsPipeline>(device, layoutSettings, pipelineSettings, cache);

	mesh = std::make_unique<Mesh>(device, vertices, indices);
}

void SimpleRenderSystem::RenderGlobal(RenderEvent& event) {
	pipeline->Bind(event.commandBuffer);
	vkCmdBindDescriptorSets(event.commandBuffer, pipeline->GetBindPoint(), pipeline->GetLayout(), 0, 1, &event.globalSet, 0, nullptr);
	vkCmdBindDescriptorSets(event.commandBuffer, pipeline->GetBindPoint(), pipeline->GetLayout(), 1, 1, &sets[event.frameIndex], 0, nullptr);
	mesh->Draw(event.commandBuffer);
}