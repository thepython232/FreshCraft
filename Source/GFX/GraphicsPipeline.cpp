#include "GraphicsPipeline.h"
#include "Vertex.h"

#define OPT_VAL_OR_NULLPTR(_v) (_v).has_value() ? &(_v).value() : VK_NULL_HANDLE

GraphicsPipeline::GraphicsPipeline(Device& device, const LayoutSettings& layout, Settings& settings, PipelineCache& cache)
: Pipeline(device, layout, cache, VK_PIPELINE_BIND_POINT_GRAPHICS) {
	VkGraphicsPipelineCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	createInfo.layout = this->layout;
	createInfo.basePipelineHandle = VK_NULL_HANDLE;
	createInfo.basePipelineIndex = -1;
	settings.colorBlending.settings.attachmentCount = static_cast<uint32_t>(settings.colorBlending.colorAttachments.size());
	settings.colorBlending.settings.pAttachments = settings.colorBlending.colorAttachments.data();
	createInfo.pColorBlendState = &settings.colorBlending.settings;
	createInfo.pDepthStencilState = OPT_VAL_OR_NULLPTR(settings.depthStencil);
	if (settings.dynamicState.settings.has_value()) {
		settings.dynamicState.settings->dynamicStateCount = static_cast<uint32_t>(settings.dynamicState.dynamicStates.size());
		settings.dynamicState.settings->pDynamicStates = settings.dynamicState.dynamicStates.data();
	}
	createInfo.pDynamicState = OPT_VAL_OR_NULLPTR(settings.dynamicState.settings);
	createInfo.pInputAssemblyState = &settings.inputAssembly;
	createInfo.pMultisampleState = OPT_VAL_OR_NULLPTR(settings.multisample);
	createInfo.pRasterizationState = &settings.rasterization;
	std::vector<VkPipelineShaderStageCreateInfo> stages;
	for (const auto& shader : settings.shaders) {
		stages.push_back(shader.Create());
	}
	createInfo.pStages = stages.data();
	createInfo.stageCount = static_cast<uint32_t>(stages.size());
	createInfo.pTessellationState = OPT_VAL_OR_NULLPTR(settings.tesselation);
	settings.vertexInput.settings.vertexBindingDescriptionCount = static_cast<uint32_t>(settings.vertexInput.bindingDescriptions.size());
	settings.vertexInput.settings.vertexAttributeDescriptionCount = static_cast<uint32_t>(settings.vertexInput.attributeDescriptions.size());
	settings.vertexInput.settings.pVertexBindingDescriptions = settings.vertexInput.bindingDescriptions.data();
	settings.vertexInput.settings.pVertexAttributeDescriptions = settings.vertexInput.attributeDescriptions.data();
	createInfo.pVertexInputState = &settings.vertexInput.settings;
	settings.viewport.info.viewportCount = static_cast<uint32_t>(settings.viewport.viewports.size());
	settings.viewport.info.scissorCount = static_cast<uint32_t>(settings.viewport.scissors.size());
	if (std::find(settings.dynamicState.dynamicStates.begin(), settings.dynamicState.dynamicStates.end(), VK_DYNAMIC_STATE_VIEWPORT) != settings.dynamicState.dynamicStates.end())
		settings.viewport.info.viewportCount += 1;
	if (std::find(settings.dynamicState.dynamicStates.begin(), settings.dynamicState.dynamicStates.end(), VK_DYNAMIC_STATE_SCISSOR) != settings.dynamicState.dynamicStates.end())
		settings.viewport.info.scissorCount += 1;
	settings.viewport.info.pViewports = settings.viewport.viewports.data();
	settings.viewport.info.pScissors = settings.viewport.scissors.data();
	createInfo.pViewportState = &settings.viewport.info;
	createInfo.renderPass = settings.renderPass;
	createInfo.subpass = settings.subpass;

	if (vkCreateGraphicsPipelines(device.GetDevice(), cache.GetCache(), 1, &createInfo, nullptr, &pipeline) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create graphics pipeline!");
	}
}

GraphicsPipeline::~GraphicsPipeline() {

}

void GraphicsPipeline::DefaultSettings(Settings& settings) {
	settings = {};
	settings.vertexInput = {};
	settings.vertexInput.settings.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	settings.vertexInput.bindingDescriptions.push_back(Vertex::GetBinding());
	settings.vertexInput.attributeDescriptions = Vertex::GetAttributes();

	settings.inputAssembly = {};
	settings.inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	settings.inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	settings.inputAssembly.primitiveRestartEnable = VK_FALSE;

	settings.tesselation = std::nullopt;

	settings.viewport.info = {};
	settings.viewport.info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	//The viewport and scissor are (by default) provided via a dynamic state
	settings.viewport.info.viewportCount = 1;
	settings.viewport.info.scissorCount = 1;

	settings.rasterization = {};
	settings.rasterization.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	settings.rasterization.cullMode = VK_CULL_MODE_BACK_BIT; //TODO
	settings.rasterization.depthBiasEnable = VK_FALSE;
	settings.rasterization.frontFace = VK_FRONT_FACE_CLOCKWISE;
	settings.rasterization.lineWidth = 1.f;
	settings.rasterization.polygonMode = VK_POLYGON_MODE_FILL;
	settings.rasterization.rasterizerDiscardEnable = VK_FALSE;
	
	settings.multisample = std::nullopt;

	settings.depthStencil = VkPipelineDepthStencilStateCreateInfo{};
	settings.depthStencil->sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	settings.depthStencil->minDepthBounds = 0.f;
	settings.depthStencil->maxDepthBounds = 1.f;
	settings.depthStencil->front = {};
	settings.depthStencil->back = {};
	settings.depthStencil->depthBoundsTestEnable = VK_FALSE;
	settings.depthStencil->depthCompareOp = VK_COMPARE_OP_LESS;
	settings.depthStencil->depthTestEnable = VK_TRUE;
	settings.depthStencil->depthWriteEnable = VK_TRUE;
	settings.depthStencil->stencilTestEnable = VK_FALSE;

	settings.colorBlending.settings = {};
	settings.colorBlending.settings.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	//Color attachments are set in the GraphicsPipeline's ctor
	settings.colorBlending.settings.blendConstants[0] = 1.f;
	settings.colorBlending.settings.blendConstants[1] = 1.f;
	settings.colorBlending.settings.blendConstants[2] = 1.f;
	settings.colorBlending.settings.blendConstants[3] = 1.f;
	settings.colorBlending.settings.logicOpEnable = VK_FALSE;
	settings.colorBlending.colorAttachments.push_back(
		VkPipelineColorBlendAttachmentState{
			VK_FALSE,
			VK_BLEND_FACTOR_SRC_ALPHA,
			VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
			VK_BLEND_OP_ADD,
			VK_BLEND_FACTOR_ONE,
			VK_BLEND_FACTOR_ONE,
			VK_BLEND_OP_ADD,
			VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT
		}
	);

	//Dynamic States set in GraphicsPipeline's ctor
	settings.dynamicState.settings = VkPipelineDynamicStateCreateInfo{};
	settings.dynamicState.settings->sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	settings.dynamicState.dynamicStates = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
}