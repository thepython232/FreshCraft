#pragma once

#include "Instance.h"
#include "Window.h"

const std::vector<const char*> deviceExtensions = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

class Device {
public:
	struct QueueFamilyIndices {
		std::optional<uint32_t> graphicsFamily;
		std::optional<uint32_t> presentFamily;

		bool IsComplete() const {
			return graphicsFamily.has_value() && presentFamily.has_value();
		}

		enum Family {
			Graphics = 1,
			Present = 2
		};
	};

	struct SwapchainSupport {
		VkSurfaceCapabilitiesKHR capabilities;
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> presentModes;
	};

	Device(Window& window, Instance& instance);
	~Device();

	VkDevice GetDevice() const { return device; }
	VkPhysicalDevice GetPhysicalDevice() const { return physicalDevice; }
	VkSurfaceKHR GetSurface() const { return surface; }
	VkQueue GetGraphicsQueue() const { return graphicsQueue; }
	VkQueue GetPresentQueue() const { return presentQueue; }
	VkCommandPool GetCommandPool() const { return commandPool; }

	SwapchainSupport QuerySwapchainSupport() const;
	QueueFamilyIndices GetQueueFamilyIndices() const;

	VkPhysicalDeviceProperties Properties() const;
	VkPhysicalDeviceFeatures Features() const;
	VkPhysicalDeviceLimits Limits() const;

	//Helper functions
	VkResult CreateImage(
		VkFormat format,
		uint32_t width,
		uint32_t height,
		uint32_t depth,
		VkImageType type,
		VkImageLayout layout,
		uint32_t mipLevels,
		VkSampleCountFlagBits samples,
		VkImageTiling tiling,
		VkImageUsageFlags usage,
		VkMemoryPropertyFlags memoryTypes,
		QueueFamilyIndices::Family queueFamilies,
		VkImage& image,
		VkDeviceMemory& memory
	) const;

	VkResult CreateImageView(
		const VkImage& image,
		VkFormat format,
		VkImageAspectFlags aspect,
		uint32_t mipLevels,
		VkImageViewType viewType,
		VkImageView& imageView
	) const;

	VkResult CreateBuffer(
		VkDeviceSize size,
		VkBufferUsageFlags usage,
		QueueFamilyIndices::Family queueFamilies,
		VkMemoryPropertyFlags memProps,
		VkBuffer& buffer,
		VkDeviceMemory& memory
	) const;

	uint32_t FindMemoryType(uint32_t memoryTypeBits, VkMemoryPropertyFlags types) const;

	VkCommandBuffer BeginSingleTimeCommands() const;
	void EndSingleTimeCommands(VkCommandBuffer commandBuffer) const;

	void CopyBuffer(VkBuffer src, VkBuffer dst, VkDeviceSize size);
	void CopyBufferToImage(VkBuffer src, VkImage dst, VkExtent3D imageExtent, VkImageAspectFlags aspect);
	void CopyImageToBuffer(VkImage src, VkBuffer dst, VkExtent3D imageExtent, VkImageAspectFlags aspect);
	void CopyImage(VkImage src, VkImage dst, VkExtent3D extent, VkImageAspectFlags aspect);

private:
	Instance& instance;

	VkSurfaceKHR surface;
	VkDevice device;
	VkPhysicalDevice physicalDevice;
	VkQueue graphicsQueue;
	VkQueue presentQueue;
	VkCommandPool commandPool;

	void PickPhysicalDevice();
	void CreateDevice();
	void CreateSurface(Window& window);
	void CreateCommandPool();

	bool IsDeviceSuitable(VkPhysicalDevice device) const;
	bool CheckExtensionSupport() const;

	QueueFamilyIndices GetQueueFamilyIndices(VkPhysicalDevice device) const;
	SwapchainSupport QuerySwapchainSupport(VkPhysicalDevice device) const;
};