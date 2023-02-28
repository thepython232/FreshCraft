#include "Descriptors.h"

DescriptorPool::DescriptorPool(Device& device, const std::vector<VkDescriptorPoolSize> poolSizes, uint32_t maxSets) : device(device) {
	VkDescriptorPoolCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	createInfo.maxSets = maxSets;
	createInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
	createInfo.pPoolSizes = poolSizes.data();

	if (vkCreateDescriptorPool(device.GetDevice(), &createInfo, nullptr, &pool) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create descriptor pool!");
	}
}

DescriptorPool::~DescriptorPool() {
	vkDestroyDescriptorPool(device.GetDevice(), pool, nullptr);
}

VkResult DescriptorPool::AllocateDescriptorSet(VkDescriptorSetLayout layout, VkDescriptorSet& set) {
	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = pool;
	allocInfo.descriptorSetCount = 1;
	allocInfo.pSetLayouts = &layout;
	return vkAllocateDescriptorSets(device.GetDevice(), &allocInfo, &set);
}

DescriptorPool::Builder::Builder(Device& device) : device(device) {

}

DescriptorPool::Builder& DescriptorPool::Builder::AddPoolSize(VkDescriptorType type, uint32_t count) {
	VkDescriptorPoolSize poolSize{};
	poolSize.type = type;
	poolSize.descriptorCount = count;
	poolSizes.push_back(poolSize);
	return *this;
}

DescriptorPool::Builder& DescriptorPool::Builder::SetMaxSets(uint32_t maxSets) {
	this->maxSets = maxSets;
	return *this;
}

std::unique_ptr<DescriptorPool> DescriptorPool::Builder::Build() {
	return std::make_unique<DescriptorPool>(device, poolSizes, maxSets);
}

DescriptorSetLayout::DescriptorSetLayout(Device& device, const std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding>& bindings) : device(device), bindings(bindings) {
	VkDescriptorSetLayoutCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	std::vector<VkDescriptorSetLayoutBinding> unorderedBindings;
	for (const auto& kv : bindings) {
		unorderedBindings.push_back(kv.second);
	}
	createInfo.bindingCount = static_cast<uint32_t>(unorderedBindings.size());
	createInfo.pBindings = unorderedBindings.data();

	if (vkCreateDescriptorSetLayout(device.GetDevice(), &createInfo, nullptr, &layout) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create descriptor set layout!");
	}
}

DescriptorSetLayout::~DescriptorSetLayout() {
	vkDestroyDescriptorSetLayout(device.GetDevice(), layout, nullptr);
}

DescriptorSetLayout::Builder::Builder(Device& device) : device(device) {

}

DescriptorSetLayout::Builder& DescriptorSetLayout::Builder::AddBinding(uint32_t binding, VkDescriptorType type, VkShaderStageFlags stage) {
	VkDescriptorSetLayoutBinding layoutBinding{};
	layoutBinding.binding = binding;
	layoutBinding.descriptorCount = 1;
	layoutBinding.descriptorType = type;
	layoutBinding.stageFlags = stage;

	bindings[binding] = layoutBinding;
	return *this;
}

std::unique_ptr<DescriptorSetLayout> DescriptorSetLayout::Builder::Build() {
	return std::make_unique<DescriptorSetLayout>(device, bindings);
}

DescriptorBuilder::DescriptorBuilder(DescriptorPool& pool, DescriptorSetLayout& layout) : pool(pool), layout(layout) {

}

DescriptorBuilder& DescriptorBuilder::WriteBuffer(uint32_t binding, VkDescriptorBufferInfo& bufferInfo) {
	VkWriteDescriptorSet write{};
	write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	write.descriptorCount = 1;
	write.descriptorType = layout.bindings[binding].descriptorType;
	write.dstBinding = binding;
	write.pBufferInfo = &bufferInfo;
	writes.push_back(write);
	return *this;
}

DescriptorBuilder& DescriptorBuilder::WriteImage(uint32_t binding, VkDescriptorImageInfo& imageInfo) {
	VkWriteDescriptorSet write{};
	write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	write.descriptorCount = 1;
	write.descriptorType = layout.bindings[binding].descriptorType;
	write.dstBinding = binding;
	write.pImageInfo = &imageInfo;
	writes.push_back(write);
	return *this;
}

void DescriptorBuilder::Overwrite(VkDescriptorSet& set) {
	for (auto& write : writes) {
		write.dstSet = set;
	}

	vkUpdateDescriptorSets(layout.device.GetDevice(), static_cast<uint32_t>(writes.size()), writes.data(), 0, nullptr);
}

VkResult DescriptorBuilder::Build(VkDescriptorSet& set) {
	VkResult result = pool.AllocateDescriptorSet(layout.GetLayout(), set);
	if (result != VK_SUCCESS)
		return result;

	Overwrite(set);
	return VK_SUCCESS;
}