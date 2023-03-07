#include "Device.h"

Device::Device(Window& window, Instance& instance) : instance(instance) {
	CreateSurface(window);
	PickPhysicalDevice();
	CreateDevice();
	CreateCommandPool();
}

Device::~Device() {
	vkDestroyCommandPool(device, commandPool, nullptr);
	vkDestroySurfaceKHR(instance.GetInstance(), surface, nullptr);
	vkDestroyDevice(device, nullptr);
}

void Device::PickPhysicalDevice() {
	uint32_t count;
	vkEnumeratePhysicalDevices(instance.GetInstance(), &count, nullptr);
	std::vector<VkPhysicalDevice> physicalDevices(count);
	vkEnumeratePhysicalDevices(instance.GetInstance(), &count, physicalDevices.data());

	if (physicalDevices.size() == 0) {
		throw std::runtime_error("No supported vulkan physical device!");
	}

	for (const auto pDevice : physicalDevices) {
		//Check for supported features
		if (IsDeviceSuitable(pDevice)) {
			physicalDevice = pDevice;
			break;
		}
	}
}

bool Device::IsDeviceSuitable(VkPhysicalDevice device) const {
	VkPhysicalDeviceFeatures features;
	VkPhysicalDeviceProperties props;
	vkGetPhysicalDeviceFeatures(device, &features);
	vkGetPhysicalDeviceProperties(device, &props);

	bool featuresSupported = features.samplerAnisotropy && features.fillModeNonSolid && features.wideLines;

	QueueFamilyIndices indices = GetQueueFamilyIndices(device);
	SwapchainSupport swapchainSupport = QuerySwapchainSupport(device);

	bool swapchainAdequate = !swapchainSupport.formats.empty();

	VkPhysicalDeviceDescriptorIndexingFeaturesEXT indexingFeatures{};
	indexingFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES_EXT;
	
	VkPhysicalDeviceFeatures2 extendedFeatures{};
	extendedFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
	extendedFeatures.pNext = &indexingFeatures;
	vkGetPhysicalDeviceFeatures2(device, &extendedFeatures);

	bool extendedFeaturesSupported = indexingFeatures.descriptorBindingPartiallyBound && indexingFeatures.runtimeDescriptorArray;

	return indices.IsComplete() && featuresSupported && swapchainAdequate && extendedFeaturesSupported;
}

bool Device::CheckExtensionSupport() const {
	uint32_t count;
	vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &count, nullptr);
	std::vector<VkExtensionProperties> extensionProperties(count);
	vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &count, extensionProperties.data());
	
	for (const auto extension : deviceExtensions) {
		bool found = false;

		for (const auto& prop : extensionProperties) {
			if (strcmp(extension, prop.extensionName) == 0) {
				found = true;
			}
		}

		if (!found)
			return false;
	}

	return true;
}

void Device::CreateDevice() {
	VkPhysicalDeviceDescriptorIndexingFeaturesEXT indexingFeatures{};
	indexingFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES_EXT;
	indexingFeatures.descriptorBindingPartiallyBound = VK_TRUE;
	indexingFeatures.runtimeDescriptorArray = VK_TRUE;

	VkPhysicalDeviceFeatures features{};
	features.samplerAnisotropy = VK_TRUE;
	features.fillModeNonSolid = VK_TRUE;
	features.wideLines = VK_TRUE;

	VkDeviceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	createInfo.pNext = &indexingFeatures;
	QueueFamilyIndices indices = GetQueueFamilyIndices(physicalDevice);
	std::set<uint32_t> queueFamilies{ indices.graphicsFamily.value(), indices.presentFamily.value() };
	float queuePriority = 1.f;
	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	for (const auto& family : queueFamilies) {
		VkDeviceQueueCreateInfo queueCreateInfo{};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.queueFamilyIndex = family;
		queueCreateInfo.pQueuePriorities = &queuePriority;

		queueCreateInfos.push_back(queueCreateInfo);
	}

	createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
	createInfo.pQueueCreateInfos = queueCreateInfos.data();

	if (enableValidationLayers) {
		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();
	}
	else {
		createInfo.enabledLayerCount = 0;
	}
	if (deviceExtensions.size() > 0 && CheckExtensionSupport()) {
		createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
		createInfo.ppEnabledExtensionNames = deviceExtensions.data();
	}
	else {
		throw std::runtime_error("Device extensions not supported!");
	}
	createInfo.pEnabledFeatures = &features;

	if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create logical device!");
	}

	vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
	vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &presentQueue);
}

void Device::CreateSurface(Window& window) {
	surface = window.GetSurface(instance.GetInstance());
}

void Device::CreateCommandPool() {
	QueueFamilyIndices indices = GetQueueFamilyIndices();
	VkCommandPoolCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	createInfo.queueFamilyIndex = indices.graphicsFamily.value();
	createInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

	if (vkCreateCommandPool(device, &createInfo, nullptr, &commandPool) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create command pool!");
	}
}

Device::QueueFamilyIndices Device::GetQueueFamilyIndices(VkPhysicalDevice device) const {
	uint32_t count;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &count, nullptr);
	std::vector<VkQueueFamilyProperties> queueFamilies(count);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &count, queueFamilies.data());

	QueueFamilyIndices indices{};
	for (int i = 0; i < queueFamilies.size(); i++) {
		if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			indices.graphicsFamily = i;
		}

		VkBool32 supported;
		vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &supported);
		if (supported)
			indices.presentFamily = i;

		if (indices.IsComplete())
			return indices;
	}

	return indices;
}

Device::SwapchainSupport Device::QuerySwapchainSupport(VkPhysicalDevice device) const {
	SwapchainSupport support{};
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &support.capabilities);

	uint32_t count;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &count, nullptr);
	support.formats.resize(count);
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &count, support.formats.data());

	vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &count, nullptr);
	support.formats.resize(count);
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &count, support.presentModes.data());

	return support;
}

Device::SwapchainSupport Device::QuerySwapchainSupport() const {
	if (!physicalDevice)
		throw std::runtime_error("Can't call QuerySwapchainSupport yet! No Physical device!");
	return QuerySwapchainSupport(physicalDevice);
}

Device::QueueFamilyIndices Device::GetQueueFamilyIndices() const {
	if (!physicalDevice)
		throw std::runtime_error("Can't call GetQueueFamilyIndices yet! No Physical device!");
	return GetQueueFamilyIndices(physicalDevice);
}

VkPhysicalDeviceProperties Device::Properties() const {
	VkPhysicalDeviceProperties props;
	vkGetPhysicalDeviceProperties(physicalDevice, &props);
	return props;
}

VkPhysicalDeviceFeatures Device::Features() const {
	VkPhysicalDeviceFeatures features;
	vkGetPhysicalDeviceFeatures(physicalDevice, &features);
	return features;
}

VkPhysicalDeviceLimits Device::Limits() const {
	return Properties().limits;
}

VkResult Device::CreateImage(
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
) const {
	VkImageCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	createInfo.arrayLayers = 1;
	createInfo.extent = VkExtent3D{ width, height, depth };
	createInfo.format = format;
	createInfo.imageType = type;
	createInfo.initialLayout = layout;
	createInfo.mipLevels = mipLevels;
	createInfo.samples = samples;
	createInfo.tiling = tiling;
	createInfo.usage = usage;

	auto indices = GetQueueFamilyIndices();
	std::vector<uint32_t> families;
	if (queueFamilies & QueueFamilyIndices::Graphics) {
		families.push_back(indices.graphicsFamily.value());
	}
	if (queueFamilies & QueueFamilyIndices::Present) {
		families.push_back(indices.presentFamily.value());
	}

	createInfo.queueFamilyIndexCount = static_cast<uint32_t>(families.size());
	createInfo.pQueueFamilyIndices = families.data();
	createInfo.sharingMode = families.size() > 1 ? VK_SHARING_MODE_CONCURRENT : VK_SHARING_MODE_EXCLUSIVE;

	VkResult result = vkCreateImage(device, &createInfo, nullptr, &image);
	if (result != VK_SUCCESS)
		return result;

	VkMemoryRequirements memRequirements;
	vkGetImageMemoryRequirements(device, image, &memRequirements);

	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, memoryTypes);

	result = vkAllocateMemory(device, &allocInfo, nullptr, &memory);
	if (result != VK_SUCCESS)
		return result;

	return vkBindImageMemory(device, image, memory, 0);
}

VkResult Device::CreateImageView(
	const VkImage& image,
	VkFormat format,
	VkImageAspectFlags aspect,
	uint32_t mipLevels,
	VkImageViewType viewType,
	VkImageView& imageView
) const {
	VkImageViewCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	createInfo.format = format;
	createInfo.image = image;
	createInfo.subresourceRange.aspectMask = aspect;
	createInfo.subresourceRange.baseArrayLayer = 0;
	createInfo.subresourceRange.baseMipLevel = 0;
	createInfo.subresourceRange.layerCount = 1;
	createInfo.subresourceRange.levelCount = mipLevels;
	createInfo.viewType = viewType;

	return vkCreateImageView(device, &createInfo, nullptr, &imageView);
}

VkResult Device::CreateBuffer(
	VkDeviceSize size,
	VkBufferUsageFlags usage,
	QueueFamilyIndices::Family queueFamilies,
	VkMemoryPropertyFlags memProps,
	VkBuffer& buffer,
	VkDeviceMemory& memory
) const {
	VkBufferCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	createInfo.size = size;
	createInfo.usage = usage;

	auto indices = GetQueueFamilyIndices();
	std::vector<uint32_t> families;
	if (queueFamilies & QueueFamilyIndices::Graphics) {
		families.push_back(indices.graphicsFamily.value());
	}
	if (queueFamilies & QueueFamilyIndices::Present) {
		families.push_back(indices.presentFamily.value());
	}

	createInfo.queueFamilyIndexCount = static_cast<uint32_t>(families.size());
	createInfo.pQueueFamilyIndices = families.data();
	createInfo.sharingMode = families.size() > 1 ? VK_SHARING_MODE_CONCURRENT : VK_SHARING_MODE_EXCLUSIVE;

	VkResult result = vkCreateBuffer(device, &createInfo, nullptr, &buffer);
	if (result != VK_SUCCESS)
		return result;

	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, memProps);

	result = vkAllocateMemory(device, &allocInfo, nullptr, &memory);
	if (result != VK_SUCCESS)
		return result;

	return vkBindBufferMemory(device, buffer, memory, 0);
}

uint32_t Device::FindMemoryType(uint32_t memoryTypeBits, VkMemoryPropertyFlags types) const {
	VkPhysicalDeviceMemoryProperties props;
	vkGetPhysicalDeviceMemoryProperties(physicalDevice, &props);
	for (uint32_t i = 0; i < 32; i++) {
		if (((1 << i) & memoryTypeBits) && (props.memoryTypes[i].propertyFlags & types)) {
			return i;
		}
	}

	throw std::runtime_error("Failed to find suitable memory type!");
}

VkCommandBuffer Device::BeginSingleTimeCommands() const {
	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandBufferCount = 1;
	allocInfo.commandPool = commandPool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

	VkCommandBuffer commandBuffer;
	if (vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer) != VK_SUCCESS) {
		throw std::runtime_error("Failed to allocate single-time command buffer!");
	}

	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
		throw std::runtime_error("Failed to begin single-time command buffer!");
	}

	return commandBuffer;
}

void Device::EndSingleTimeCommands(VkCommandBuffer commandBuffer) const {
	if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
		throw std::runtime_error("Failed to end single-time command buffer!");
	}

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	VkFence waitFence;
	VkFenceCreateInfo fenceInfo{};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	if (vkCreateFence(device, &fenceInfo, nullptr, &waitFence) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create single-time fence!");
	}
	
	if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, waitFence) != VK_SUCCESS) {
		throw std::runtime_error("Failed to submit single-time command buffer!");
	}

	vkWaitForFences(device, 1, &waitFence, VK_TRUE, UINT64_MAX);
	vkDestroyFence(device, waitFence, nullptr);
}

void Device::CopyBuffer(VkBuffer src, VkBuffer dst, VkDeviceSize size) {
	auto commandBuffer = BeginSingleTimeCommands();
	VkBufferCopy copy{};
	copy.srcOffset = 0;
	copy.dstOffset = 0;
	copy.size = size;
	vkCmdCopyBuffer(commandBuffer, src, dst, 1, &copy);
	EndSingleTimeCommands(commandBuffer);
}

void Device::CopyBufferToImage(VkBuffer src, VkImage dst, VkExtent3D imageExtent, VkImageAspectFlags aspect) {
	auto commandBuffer = BeginSingleTimeCommands();
	VkBufferImageCopy copy{};
	copy.imageExtent = imageExtent;
	copy.imageOffset = { 0, 0, 0 };
	copy.bufferOffset = 0;
	copy.bufferRowLength = 0;
	copy.bufferImageHeight = 0;
	copy.imageSubresource.aspectMask = aspect;
	copy.imageSubresource.baseArrayLayer = 0;
	copy.imageSubresource.layerCount = 1;
	copy.imageSubresource.mipLevel = 0;

	vkCmdCopyBufferToImage(commandBuffer, src, dst, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy);
	EndSingleTimeCommands(commandBuffer);
}

void Device::CopyImageToBuffer(VkImage src, VkBuffer dst, VkExtent3D imageExtent, VkImageAspectFlags aspect) {
	auto commandBuffer = BeginSingleTimeCommands();
	VkBufferImageCopy copy{};
	copy.imageExtent = imageExtent;
	copy.imageOffset = { 0, 0, 0 };
	copy.bufferOffset = 0;
	copy.bufferRowLength = 0;
	copy.bufferImageHeight = 0;
	copy.imageSubresource.aspectMask = aspect;
	copy.imageSubresource.baseArrayLayer = 0;
	copy.imageSubresource.layerCount = 1;
	copy.imageSubresource.mipLevel = 0;

	vkCmdCopyImageToBuffer(commandBuffer, src, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, dst, 1, &copy);
	EndSingleTimeCommands(commandBuffer);
}

void Device::CopyImage(VkImage src, VkImage dst, VkExtent3D extent, VkImageAspectFlags aspect) {
	auto commandBuffer = BeginSingleTimeCommands();
	VkImageCopy copy{};
	copy.extent = extent;
	copy.srcOffset = { 0, 0, 0 };
	copy.dstOffset = { 0, 0, 0 };
	copy.srcSubresource.aspectMask = aspect;
	copy.srcSubresource.baseArrayLayer = 0;
	copy.srcSubresource.layerCount = 1;
	copy.srcSubresource.mipLevel = 0;
	copy.dstSubresource.aspectMask = aspect;
	copy.dstSubresource.baseArrayLayer = 0;
	copy.dstSubresource.layerCount = 1;
	copy.dstSubresource.mipLevel = 0;
}