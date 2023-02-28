#pragma once

#include "Device.h"

class DescriptorPool {
public:
	DescriptorPool(Device& device, const std::vector<VkDescriptorPoolSize> poolSizes, uint32_t maxSets);
	~DescriptorPool();

	VkResult AllocateDescriptorSet(VkDescriptorSetLayout layout, VkDescriptorSet& set);

	class Builder {
	public:
		Builder(Device& device);
		Builder& AddPoolSize(VkDescriptorType type, uint32_t count);
		Builder& SetMaxSets(uint32_t maxSets);
		std::unique_ptr<DescriptorPool> Build();

	private:
		std::vector<VkDescriptorPoolSize> poolSizes;
		uint32_t maxSets = 0;
		Device& device;
	};

private:
	VkDescriptorPool pool;
	Device& device;
};

class DescriptorSetLayout {
public:
	DescriptorSetLayout(Device& device, const std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding>& bindings);
	~DescriptorSetLayout();

	VkDescriptorSetLayout GetLayout() const { return layout; }

	class Builder {
	public:
		Builder(Device& device);
		Builder& AddBinding(uint32_t binding, VkDescriptorType type, VkShaderStageFlags stage);
		std::unique_ptr<DescriptorSetLayout> Build();

	private:
		std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings;
		Device& device;
	};

private:
	VkDescriptorSetLayout layout;
	friend class DescriptorBuilder;
	std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings;
	Device& device;
};

class DescriptorBuilder {
public:
	DescriptorBuilder(DescriptorPool& pool, DescriptorSetLayout& layout);
	
	DescriptorBuilder& WriteBuffer(uint32_t binding, VkDescriptorBufferInfo& bufferInfo);
	DescriptorBuilder& WriteImage(uint32_t binding, VkDescriptorImageInfo& imageInfo);

	void Overwrite(VkDescriptorSet& set);
	VkResult Build(VkDescriptorSet& set);

private:
	std::vector<VkWriteDescriptorSet> writes;
	DescriptorSetLayout& layout;
	DescriptorPool& pool;
};