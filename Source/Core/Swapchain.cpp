#include "Swapchain.h"

Swapchain::Swapchain(Device& device, Window& window) : device(device), window(window) {
	CreateSwapchain(VK_NULL_HANDLE);
	CreateImageViews();
	CreateSyncObjects();
}

Swapchain::~Swapchain() {
	for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		vkDestroySemaphore(device.GetDevice(), imageAvailableSemaphores[i], nullptr);
		vkDestroySemaphore(device.GetDevice(), renderFinishedSemaphores[i], nullptr);
		vkDestroyFence(device.GetDevice(), inFlightFences[i], nullptr);
	}

	for (auto imageView : views) {
		vkDestroyImageView(device.GetDevice(), imageView, nullptr);
	}

	vkDestroySwapchainKHR(device.GetDevice(), swapchain, nullptr);
}

void Swapchain::CreateSwapchain(VkSwapchainKHR oldSwapchain) {
	VkSwapchainCreateInfoKHR createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = device.GetSurface();
	const Device::SwapchainSupport support = device.QuerySwapchainSupport();
	createInfo.presentMode = PickPresentMode(support.presentModes);
	createInfo.clipped = VK_TRUE;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.minImageCount = support.capabilities.minImageCount + 1;
	if (support.capabilities.maxImageCount < createInfo.minImageCount)
		createInfo.minImageCount = support.capabilities.maxImageCount;
	createInfo.oldSwapchain = oldSwapchain;     //TODO
	Device::QueueFamilyIndices indices = device.GetQueueFamilyIndices();
	std::array<uint32_t, 2> queueFamilyIndices = { indices.graphicsFamily.value(), indices.presentFamily.value() };
	if (indices.graphicsFamily.value() != indices.presentFamily.value()) {
		createInfo.queueFamilyIndexCount = 2;
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.pQueueFamilyIndices = queueFamilyIndices.data();
	}
	else {
		createInfo.queueFamilyIndexCount = 0;
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	}
	createInfo.preTransform = support.capabilities.currentTransform;
	createInfo.imageArrayLayers = 1;
	VkSurfaceFormatKHR surfaceFormat = PickFormat(support.formats);
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageExtent = PickExtent(support.capabilities);
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	
	if (vkCreateSwapchainKHR(device.GetDevice(), &createInfo, nullptr, &swapchain) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create swapchain!");
	}

	uint32_t count;
	vkGetSwapchainImagesKHR(device.GetDevice(), swapchain, &count, nullptr);
	images.resize(count);
	vkGetSwapchainImagesKHR(device.GetDevice(), swapchain, &count, images.data());

	format = createInfo.imageFormat;
	extent = createInfo.imageExtent;
}

void Swapchain::CreateImageViews() {
	views.resize(images.size());
	for (int i = 0; i < views.size(); i++) {
		//TODO: create an image using the device
		if (device.CreateImageView(
			images[i],
			format,
			VK_IMAGE_ASPECT_COLOR_BIT,
			1,
			VK_IMAGE_VIEW_TYPE_2D,
			views[i]
		) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create swapchain image view!");
		}
	}
}

void Swapchain::CreateSyncObjects() {
	imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
	renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
	inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
	for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		VkSemaphoreCreateInfo semaphoreInfo{};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		
		VkFenceCreateInfo fenceInfo{};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		if (vkCreateSemaphore(device.GetDevice(), &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS
			|| vkCreateSemaphore(device.GetDevice(), &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS
			|| vkCreateFence(device.GetDevice(), &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create sync objects!");
		}
	}
}

void Swapchain::Recreate() {
	VkSwapchainKHR oldSwapchain = swapchain;
	VkFormat oldFormat = format;
	CreateSwapchain(oldSwapchain);
	vkDestroySwapchainKHR(device.GetDevice(), oldSwapchain, nullptr);
	if (oldFormat != format) {
		throw std::runtime_error("Swapchain formats have changed!");
	}

	for (const auto& view : views) {
		vkDestroyImageView(device.GetDevice(), view, nullptr);
	}

	CreateImageViews();
}

VkPresentModeKHR Swapchain::PickPresentMode(const std::vector<VkPresentModeKHR>& presentModes) const {
	for (const auto& mode : presentModes) {
		if (mode == VK_PRESENT_MODE_MAILBOX_KHR)
			return mode;
	}

	return VK_PRESENT_MODE_FIFO_KHR;
}

VkSurfaceFormatKHR Swapchain::PickFormat(const std::vector<VkSurfaceFormatKHR>& formats) const {
	for (const auto& format : formats) {
		if (format.colorSpace == VK_COLORSPACE_SRGB_NONLINEAR_KHR && format.format == VK_FORMAT_B8G8R8A8_SRGB) {
			return format;
		}
	}

	return formats.front();
}

VkExtent2D Swapchain::PickExtent(const VkSurfaceCapabilitiesKHR& capabilities) const {
	if (capabilities.currentExtent.width == UINT32_MAX || capabilities.currentExtent.height == UINT32_MAX) {
		VkExtent2D extent = window.GetFramebufferSize();
		extent.width = std::clamp(capabilities.currentExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
		extent.height = std::clamp(capabilities.currentExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
		return extent;
	}
	else {
		return capabilities.currentExtent;
	}
}

VkResult Swapchain::AcquireNextImage(uint32_t* imageIndex) {
	vkWaitForFences(
		device.GetDevice(),
		1,
		&inFlightFences[currentFrame],
		VK_TRUE,
		UINT64_MAX);

	vkResetFences(device.GetDevice(), 1, &inFlightFences[currentFrame]);

	VkResult result = vkAcquireNextImageKHR(
		device.GetDevice(),
		swapchain,
		UINT64_MAX,
		imageAvailableSemaphores[currentFrame],
		VK_NULL_HANDLE,
		imageIndex
	);

	return result;
}

VkResult Swapchain::SubmitFrame(VkCommandBuffer commandBuffer, uint32_t* imageIndex) {
	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;
	VkPipelineStageFlags waitStages = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	submitInfo.pWaitDstStageMask = &waitStages;
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = &imageAvailableSemaphores[currentFrame];
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = &renderFinishedSemaphores[currentFrame];

	if (vkQueueSubmit(device.GetGraphicsQueue(), 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS) {
		throw std::runtime_error("Failed to submit draw command buffer!");
	}

	VkPresentInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.pImageIndices = imageIndex;
	presentInfo.pSwapchains = &swapchain;
	presentInfo.swapchainCount = 1;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = &renderFinishedSemaphores[currentFrame];

	VkResult result = vkQueuePresentKHR(device.GetPresentQueue(), &presentInfo);
	currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
	return result;
}