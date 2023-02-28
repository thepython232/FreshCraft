#pragma once

#include "Device.h"

class PipelineCache {
public:
	PipelineCache(Device& device, const std::string& filename);
	~PipelineCache();

	VkPipelineCache GetCache() const { return cache; }

private:
	VkPipelineCache cache;
	std::string filename;
	Device& device;
};

class Pipeline {
public:
	struct LayoutSettings {
		std::vector<VkDescriptorSetLayout> layouts;
		std::vector<VkPushConstantRange> pushConstants;
	};

	class Shader {
	public:
		Shader(Device& device, const std::string& filename, VkShaderStageFlagBits stage);
		~Shader();

		Shader(const Shader& other) = delete;
		Shader& operator=(const Shader& other) = delete;

		Shader(Shader&& other) noexcept : shaderModule(other.shaderModule), stage(other.stage), device(other.device) {
			other.shaderModule = VK_NULL_HANDLE;
		}

		Shader& operator=(Shader&& other) noexcept {
			shaderModule = other.shaderModule;
			stage = other.stage;
			other.shaderModule = VK_NULL_HANDLE;
			return *this;
		}

		VkPipelineShaderStageCreateInfo Create() const;

	private:
		VkShaderModule shaderModule;
		VkShaderStageFlagBits stage;
		Device& device;
	};

	~Pipeline();

	VkPipeline GetPipeline() const { return pipeline; }
	VkPipelineLayout GetLayout() const { return layout; }
	VkPipelineBindPoint GetBindPoint() const { return bindPoint; }

	void Bind(VkCommandBuffer commandBuffer) const;

protected:
	//This cannot be created except through inheritance
	Pipeline(Device& device, const LayoutSettings& layoutSettings, PipelineCache& cache, VkPipelineBindPoint bindPoint);

	VkPipeline pipeline;
	VkPipelineLayout layout;
	const VkPipelineBindPoint bindPoint;
	PipelineCache& cache;

private:
	Device& device;
};