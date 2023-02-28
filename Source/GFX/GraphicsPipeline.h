#pragma once

#include "Core\Pipeline.h"

class GraphicsPipeline : public Pipeline {
public:
	struct Settings {
        struct {
            VkPipelineVertexInputStateCreateInfo settings;
            std::vector<VkVertexInputBindingDescription> bindingDescriptions;
            std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
        } vertexInput;

        VkPipelineInputAssemblyStateCreateInfo inputAssembly;

        std::optional<VkPipelineTessellationStateCreateInfo> tesselation;

        struct {
            VkPipelineViewportStateCreateInfo info;
            std::vector<VkViewport> viewports;
            std::vector<VkRect2D> scissors;
        } viewport;

        VkPipelineRasterizationStateCreateInfo rasterization;
        std::optional<VkPipelineMultisampleStateCreateInfo> multisample;
        std::optional<VkPipelineDepthStencilStateCreateInfo> depthStencil;

        struct {
            VkPipelineColorBlendStateCreateInfo settings;
            std::vector<VkPipelineColorBlendAttachmentState> colorAttachments;
        } colorBlending;

        struct {
            std::optional<VkPipelineDynamicStateCreateInfo> settings;
            std::vector<VkDynamicState> dynamicStates;
        } dynamicState;

        VkRenderPass renderPass;
        uint32_t subpass;

        std::vector<Shader> shaders;
	};

	GraphicsPipeline(Device& device, const LayoutSettings& layout, Settings& settings, PipelineCache& cache);
	~GraphicsPipeline();

	static void DefaultSettings(Settings& settings);
};