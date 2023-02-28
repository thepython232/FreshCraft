#pragma once

#include "Device.h"

class Swapchain {
public:
	static constexpr int MAX_FRAMES_IN_FLIGHT = 2;

	Swapchain(Device& device, Window& window);
	~Swapchain();

	VkSwapchainKHR GetSwapchain() const { return swapchain; }

	VkImage GetImage(int i) const { return images[i]; }
	VkImageView GetImageView(int i) const { return views[i]; }

	uint32_t GetImageCount() const { return static_cast<uint32_t>(images.size()); }
	VkExtent2D GetExtent() const { return extent; }
	VkFormat GetFormat() const { return format; }

	uint32_t GetFrameIndex() const { return currentFrame; }

	VkResult AcquireNextImage(uint32_t* imageIndex);
	VkResult SubmitFrame(VkCommandBuffer commandBuffer, uint32_t* imageIndex);

	void Recreate();

private:
	void CreateSwapchain(VkSwapchainKHR oldSwapchain);
	void CreateImageViews();
	void CreateSyncObjects();

	VkPresentModeKHR PickPresentMode(const std::vector<VkPresentModeKHR>& presentModes) const;
	VkSurfaceFormatKHR PickFormat(const std::vector<VkSurfaceFormatKHR>& formats) const;
	VkExtent2D PickExtent(const VkSurfaceCapabilitiesKHR& capabilities) const;
	
	VkSwapchainKHR swapchain;
	std::vector<VkImage> images;
	std::vector<VkImageView> views;
	VkFormat format;
	VkExtent2D extent;

	std::vector<VkSemaphore> imageAvailableSemaphores;
	std::vector<VkSemaphore> renderFinishedSemaphores;
	std::vector<VkFence> inFlightFences;

	uint32_t currentFrame = 0;

	Device& device;
	Window& window;
};