#include "Pipeline.h"

PipelineCache::PipelineCache(Device& device, const std::string& filename) : device(device), filename(filename) {
	VkPipelineCacheCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;

	//Check if the file exists
	std::ifstream file(filename, std::ios::in | std::ios::binary | std::ios::ate);
	if (file.good()) {
		createInfo.initialDataSize = static_cast<size_t>(file.tellg());
		//Read the data
		std::vector<char> buffer(file.tellg());
		file.read(buffer.data(), buffer.size());
		file.close();
		createInfo.pInitialData = buffer.data();
	}
	else {
		createInfo.initialDataSize = 0;
	}

	if (vkCreatePipelineCache(device.GetDevice(), &createInfo, nullptr, &cache) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create pipeline cache!");
	}
}

PipelineCache::~PipelineCache() {
	size_t size;
	vkGetPipelineCacheData(device.GetDevice(), cache, &size, nullptr);
	std::vector<char> buffer(size);
	vkGetPipelineCacheData(device.GetDevice(), cache, &size, buffer.data());

	std::ofstream file(filename, std::ios::out | std::ios::binary | std::ios::trunc);
	file.write(buffer.data(), size);
	file.close();

	vkDestroyPipelineCache(device.GetDevice(), cache, nullptr);
}

Pipeline::Pipeline(Device& device, const LayoutSettings& layoutSettings, PipelineCache& cache, VkPipelineBindPoint bindPoint) : device(device), cache(cache), bindPoint(bindPoint) {
	VkPipelineLayoutCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	createInfo.setLayoutCount = static_cast<uint32_t>(layoutSettings.layouts.size());
	createInfo.pSetLayouts = layoutSettings.layouts.data();
	createInfo.pushConstantRangeCount = static_cast<uint32_t>(layoutSettings.pushConstants.size());
	createInfo.pPushConstantRanges = layoutSettings.pushConstants.data();

	if (vkCreatePipelineLayout(device.GetDevice(), &createInfo, nullptr, &layout) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create pipeline cache!");
	}
}

Pipeline::~Pipeline() {
	vkDestroyPipeline(device.GetDevice(), pipeline, nullptr);
	vkDestroyPipelineLayout(device.GetDevice(), layout, nullptr);
}

void Pipeline::Bind(VkCommandBuffer commandBuffer) const {
	vkCmdBindPipeline(commandBuffer, bindPoint, pipeline);
}

//TODO: implement a copy constructor, seems to be an issue
Pipeline::Shader::Shader(Device& device, const std::string& filename, VkShaderStageFlagBits stage) : device(device), stage(stage) {
	VkShaderModuleCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	std::ifstream file(filename, std::ios::in | std::ios::binary | std::ios::ate);
	if (!file.is_open())
		throw std::runtime_error("Failed to load shader " + filename);
	std::vector<char> buffer(file.tellg());
	file.seekg(0);
	file.read(buffer.data(), buffer.size());
	file.close();
	createInfo.codeSize = buffer.size();
	createInfo.pCode = reinterpret_cast<const uint32_t*>(buffer.data());

	if (vkCreateShaderModule(device.GetDevice(), &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create shader module!");
	}
}

Pipeline::Shader::~Shader() {
	vkDestroyShaderModule(device.GetDevice(), shaderModule, nullptr);
}

VkPipelineShaderStageCreateInfo Pipeline::Shader::Create() const {
	VkPipelineShaderStageCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	createInfo.module = shaderModule;
	createInfo.pName = "main";
	createInfo.stage = stage;
	return createInfo;
}