#pragma once

#include "Device.h"

class Buffer {
public:
	Buffer(
		Device& device,
		VkDeviceSize instanceSize,
		VkDeviceSize instanceCount,
		VkBufferUsageFlags usage,
		VkMemoryPropertyFlags memProps,
		Device::QueueFamilyIndices::Family families,
		VkDeviceSize minOffsetAlignment = 0u
	);

	~Buffer();

	VkBuffer GetBuffer() const { return buffer; }
	VkDeviceSize GetInstanceSize() const { return instanceSize; }
	VkDeviceSize GetInstanceCount() const { return bufferSize / instanceSize; }
	VkDeviceSize GetBufferSize() const { return bufferSize; }
	void* GetMappedMemory() const {
		if (!isMapped) return nullptr;
		return mapped;
	}
	bool IsMapped() const { return isMapped; }

	VkDescriptorBufferInfo GetDescriptorInfo() const;

	VkResult Map(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0u);
	void UnMap();

	void WriteToBuffer(void* data, VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0u);
	void WriteToIndex(void* data, int index);

	VkResult Invalidate(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0u);
	VkResult InvalidateIndex(int index);

	VkResult Flush(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0u);
	VkResult FlushIndex(int index);

	static VkDeviceSize GetAlignment(VkDeviceSize instanceSize, VkDeviceSize minOffsetAlignment);

private:
	VkBuffer buffer;
	VkDeviceMemory memory;
	void* mapped;
	bool isMapped = false;
	VkDeviceSize bufferSize;
	VkDeviceSize instanceSize;

	Device& device;
};