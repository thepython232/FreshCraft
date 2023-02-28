#include "Buffer.h"

VkDeviceSize Buffer::GetAlignment(VkDeviceSize instanceSize, VkDeviceSize minOffsetAlignment) {
	if (minOffsetAlignment > 0) {
		return (instanceSize + minOffsetAlignment - 1) & ~(minOffsetAlignment - 1);
	}
	else {
		return instanceSize;
	}
}

Buffer::Buffer(
	Device& device,
	VkDeviceSize instanceSize,
	VkDeviceSize instanceCount,
	VkBufferUsageFlags usage,
	VkMemoryPropertyFlags memProps,
	Device::QueueFamilyIndices::Family families,
	VkDeviceSize minOffsetAlignment
) : device(device) {
	this->instanceSize = GetAlignment(instanceSize, minOffsetAlignment);
	bufferSize = instanceCount * this->instanceSize;
	if (device.CreateBuffer(
		bufferSize,
		usage,
		families,
		memProps,
		buffer,
		memory
	) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create buffer!");
	}
}

Buffer::~Buffer() {
	UnMap();
	vkFreeMemory(device.GetDevice(), memory, nullptr);
	vkDestroyBuffer(device.GetDevice(), buffer, nullptr);
}

VkDescriptorBufferInfo Buffer::GetDescriptorInfo() const {
	VkDescriptorBufferInfo info{};
	info.buffer = buffer;
	info.offset = 0;
	info.range = bufferSize;
	return info;
}

VkResult Buffer::Map(VkDeviceSize size, VkDeviceSize offset) {
	isMapped = true;
	return vkMapMemory(device.GetDevice(), memory, offset, size, 0, &mapped);
}

void Buffer::UnMap() {
	if (isMapped) {
		isMapped = false;
		vkUnmapMemory(device.GetDevice(), memory);
	}
}

void Buffer::WriteToBuffer(void* data, VkDeviceSize size, VkDeviceSize offset) {
	if (!isMapped)
		throw std::runtime_error("Buffer not mapped!");

	VkDeviceSize copySize = (size == VK_WHOLE_SIZE) ? bufferSize : size;
	if (offset > 0u) {
		char* offsetted = (char*)mapped;
		offsetted += offset;
		memcpy(offsetted, data, copySize);
	}
	else {
		memcpy(mapped, data, copySize);
	}
}

void Buffer::WriteToIndex(void* data, int index) {
	if (!isMapped)
		throw std::runtime_error("Buffer not mapped!");

	char* location = (char*)mapped + index * instanceSize;
	memcpy(location, data, instanceSize);
}

VkResult Buffer::Invalidate(VkDeviceSize size, VkDeviceSize offset) {
	VkMappedMemoryRange range{};
	range.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
	range.memory = memory;
	range.offset = offset;
	range.size = size;
	return vkInvalidateMappedMemoryRanges(device.GetDevice(), 1, &range);
}

VkResult Buffer::InvalidateIndex(int index) {
	VkMappedMemoryRange range{};
	range.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
	range.memory = memory;
	range.offset = index * instanceSize;
	range.size = instanceSize;
	return vkInvalidateMappedMemoryRanges(device.GetDevice(), 1, &range);
}


VkResult Buffer::Flush(VkDeviceSize size, VkDeviceSize offset) {
	VkMappedMemoryRange range{};
	range.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
	range.memory = memory;
	range.offset = offset;
	range.size = size;
	return vkFlushMappedMemoryRanges(device.GetDevice(), 1, &range);
}

VkResult Buffer::FlushIndex(int index) {
	VkMappedMemoryRange range{};
	range.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
	range.memory = memory;
	range.offset = index * instanceSize;
	range.size = instanceSize;
	return vkFlushMappedMemoryRanges(device.GetDevice(), 1, &range);
}
