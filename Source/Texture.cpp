#include "Texture.h"
#include "Buffer.h"

#ifdef _MSC_VER
#pragma warning (push, 0)
#endif
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#ifdef _MSC_VER
#pragma warning (pop, 0)
#endif

Texture::Texture(
	Device& device,
	uint32_t width,
	uint32_t height,
	uint32_t depth,
	uint32_t mipLevels,
	VkFormat format,
	VkImageAspectFlags aspect,
	VkImageUsageFlags usage,
	VkMemoryPropertyFlags memProps,
	Device::QueueFamilyIndices::Family families,
	VkImageLayout initialLayout,
	VkImageTiling tiling,
	bool createSampler,
	SamplerSettings samplerSettings
) : device(device), width(width), height(height), depth(depth), mipLevels(mipLevels), format(format), aspect(aspect), currentLayout(initialLayout) {
	//std::cout << "Creating a " << width << " by " << height << " texture with format " << format << std::endl;

	if (depth == 1 && height == 1) {
		imageType = VK_IMAGE_TYPE_1D;
		imageViewType = VK_IMAGE_VIEW_TYPE_1D;
	}
	else if (depth == 1) {
		imageType = VK_IMAGE_TYPE_2D;
		imageViewType = VK_IMAGE_VIEW_TYPE_2D;
	}
	else {
		imageType = VK_IMAGE_TYPE_3D;
		imageViewType = VK_IMAGE_VIEW_TYPE_3D;
	}

	usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	
	if (createSampler)
		usage |= VK_IMAGE_USAGE_SAMPLED_BIT;
	if (mipLevels > 1)
		usage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

	VkResult result = device.CreateImage(
		format,
		width, height, depth,
		imageType,
		initialLayout,
		mipLevels,
		VK_SAMPLE_COUNT_1_BIT,
		tiling,
		usage,
		memProps,
		families,
		image,
		memory
	);
	if (result != VK_SUCCESS) {
		std::stringstream ss;
		ss << "Failed to create texture image!" << std::endl;
		ss << "Error: " << result << std::endl;
		throw std::runtime_error(ss.str());
	}

	if (device.CreateImageView(
		image,
		format,
		aspect,
		mipLevels,
		imageViewType,
		imageView
	) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create texture image view!");
	}

	if (createSampler) {
		VkSamplerCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		createInfo.addressModeU = samplerSettings.addressMode;
		createInfo.addressModeV = samplerSettings.addressMode;
		createInfo.addressModeW = samplerSettings.addressMode;
		createInfo.anisotropyEnable = samplerSettings.enableAnisotropy;
		createInfo.maxAnisotropy = device.Limits().maxSamplerAnisotropy;
		createInfo.borderColor = samplerSettings.borderColor;
		createInfo.compareEnable = VK_FALSE;
		createInfo.unnormalizedCoordinates = VK_FALSE;
		createInfo.minFilter = samplerSettings.filter;
		createInfo.magFilter = samplerSettings.filter;
		createInfo.minLod = 0.f;
		createInfo.maxLod = static_cast<float>(mipLevels);
		createInfo.mipLodBias = samplerSettings.mipLodBias;
		createInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		
		if (vkCreateSampler(device.GetDevice(), &createInfo, nullptr, &sampler) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create texture sampler!");
		}
	}
}

Texture::Texture(
	Device& device,
	VkImage image,
	VkImageView imageView,
	uint32_t width,
	uint32_t height,
	uint32_t depth,
	uint32_t mipLevels,
	VkFormat format,
	VkImageAspectFlags aspect,
	VkImageLayout imageLayout
)
	: device(device),
	width(width),
	height(height),
	depth(depth),
	mipLevels(mipLevels),
	format(format),
	image(image),
	imageView(imageView),
	aspect(aspect),
	currentLayout(imageLayout)
{
	if (depth == 1 && height == 1) {
		imageType = VK_IMAGE_TYPE_1D;
		imageViewType = VK_IMAGE_VIEW_TYPE_1D;
	}
	else if (depth == 1) {
		imageType = VK_IMAGE_TYPE_2D;
		imageViewType = VK_IMAGE_VIEW_TYPE_2D;
	}
	else {
		imageType = VK_IMAGE_TYPE_3D;
		imageViewType = VK_IMAGE_VIEW_TYPE_3D;
	}
}

Texture::~Texture() {
	if (memory != VK_NULL_HANDLE) {
		vkDestroySampler(device.GetDevice(), sampler, nullptr);
		vkDestroyImageView(device.GetDevice(), imageView, nullptr);
		vkFreeMemory(device.GetDevice(), memory, nullptr);
		vkDestroyImage(device.GetDevice(), image, nullptr);
	}
}

VkDescriptorImageInfo Texture::GetDescriptorInfo() const {
	VkDescriptorImageInfo info{};
	info.imageLayout = currentLayout;
	info.imageView = imageView;
	info.sampler = sampler;
	return info;
}

void Texture::TransitionLayout(VkImageLayout newLayout) {
	VkImageMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.image = image;
	barrier.oldLayout = currentLayout;
	barrier.newLayout = newLayout;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.subresourceRange.aspectMask = aspect;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.layerCount = 1;
	barrier.subresourceRange.levelCount = mipLevels;
	
	VkPipelineStageFlags srcStage, dstStage;

	if (currentLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
		srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;

		barrier.srcAccessMask = VK_ACCESS_NONE;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	}
	else if (currentLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
		srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		dstStage = VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;

		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
	}
	else if (currentLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL) {
		srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;

		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
	}
	else if (currentLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
		srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;

		barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	}
	else {
		throw std::runtime_error("Unsupported layout transition!");
	}

	auto commandBuffer = device.BeginSingleTimeCommands();

	vkCmdPipelineBarrier(
		commandBuffer,
		srcStage, dstStage,
		0,
		0, nullptr,
		0, nullptr,
		1, &barrier
	);

	device.EndSingleTimeCommands(commandBuffer);

	currentLayout = newLayout;
}

void Texture::FillMipMaps() {
	VkFormatProperties props;
	vkGetPhysicalDeviceFormatProperties(device.GetPhysicalDevice(), format, &props);

	if (!(props.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)) {
		std::stringstream ss;
		ss << "Texture Format " << (int)format << " does not support linear blitting!";
		throw std::runtime_error(ss.str());
	}
	auto commandBuffer = device.BeginSingleTimeCommands();

	//Common for all barriers
	VkImageMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.image = image;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.subresourceRange.aspectMask = aspect;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;
	barrier.subresourceRange.levelCount = 1;

	int32_t mipWidth = width;
	int32_t mipHeight = height;
	//Loop through each layer that we are blitting to
	for (uint32_t i = 1; i < mipLevels; i++) {
		//Step one: transition the source of image (up one mip level) to VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL
		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		barrier.subresourceRange.baseMipLevel = i - 1;

		vkCmdPipelineBarrier(
			commandBuffer,
			VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
			0,
			0, nullptr,
			0, nullptr,
			1, &barrier
		);

		//Step two: blit data from the previous layer onto this layer
		VkImageBlit blit{};
		//Blit from the entire previous level
		blit.srcOffsets[0] = { 0, 0, 0 };
		blit.srcOffsets[1] = { mipWidth, mipHeight, 1 };
		blit.srcSubresource.aspectMask = aspect;
		blit.srcSubresource.baseArrayLayer = 0;
		blit.srcSubresource.layerCount = 1;
		blit.srcSubresource.mipLevel = i - 1;
		
		blit.dstOffsets[0] = { 0, 0, 0 };
		blit.dstOffsets[1] = { mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1 };
		blit.dstSubresource.aspectMask = aspect;
		blit.dstSubresource.baseArrayLayer = 0;
		blit.dstSubresource.layerCount = 1;
		blit.dstSubresource.mipLevel = i;

		vkCmdBlitImage(
			commandBuffer,
			image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			1, &blit,
			VK_FILTER_LINEAR
		);

		//Step 3: transition the previous level to VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		barrier.subresourceRange.baseMipLevel = i - 1;

		vkCmdPipelineBarrier(
			commandBuffer,
			VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_VERTEX_SHADER_BIT,
			0,
			0, nullptr,
			0, nullptr,
			1, &barrier
		);

		//Divide width and height (can't be zero)
		if (mipWidth > 1) mipWidth /= 2;
		if (mipHeight > 1) mipHeight /= 2;
	}

	//Step 4: transition the last level (never blitted from)
	barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
	barrier.subresourceRange.baseMipLevel = mipLevels - 1;

	vkCmdPipelineBarrier(
		commandBuffer,
		VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_VERTEX_SHADER_BIT,
		0,
		0, nullptr,
		0, nullptr,
		1, &barrier
	);

	device.EndSingleTimeCommands(commandBuffer);

	currentLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
}

std::unique_ptr<Texture> Texture::Load(
	Device& device,
	const std::string& filename,
	VkImageUsageFlags usage,
	Device::QueueFamilyIndices::Family families,
	bool enableMipmapping,
	bool loadHDR,
	bool createSampler,
	SamplerSettings samplerSettings
) {
	int width, height, channels;
	void* pixels;
	if (loadHDR) {
		pixels = (void*)stbi_loadf(filename.c_str(), &width, &height, &channels, STBI_rgb_alpha);
	}
	else {
		pixels = (void*)stbi_load(filename.c_str(), &width, &height, &channels, STBI_rgb_alpha);
	}

	if (!pixels) {
		throw std::runtime_error("Failed to load texture " + filename + "!");
	}

	VkFormat format;
	if (loadHDR) {
		format = VK_FORMAT_R32G32B32A32_SFLOAT;
	}
	else {
		format = VK_FORMAT_R8G8B8A8_SRGB;
	}

	uint32_t levels;
	if (enableMipmapping)
		levels = static_cast<uint32_t>(std::floor(std::log2(std::max(width, height)))) + 1;
	else
		levels = 1;

	std::unique_ptr<Texture> texture = std::make_unique<Texture>(
		device,
		width,
		height,
		1,
		levels,
		format,
		VK_IMAGE_ASPECT_COLOR_BIT,
		usage,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		families,
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_TILING_OPTIMAL,
		createSampler,
		samplerSettings
		);

	texture->TransitionLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

	Buffer buffer(
		device,
		loadHDR ? sizeof(float) : sizeof(stbi_uc),
		width * height * 4,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		Device::QueueFamilyIndices::Graphics
	);

	buffer.Map();
	buffer.WriteToBuffer(pixels);
	buffer.UnMap();

	device.CopyBufferToImage(buffer.GetBuffer(), texture->GetImage(), { static_cast<uint32_t>(width), static_cast<uint32_t>(height), 1 }, VK_IMAGE_ASPECT_COLOR_BIT);

	if (enableMipmapping) {
		texture->FillMipMaps();
	}
	else {
		texture->TransitionLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	}
	return texture;
}