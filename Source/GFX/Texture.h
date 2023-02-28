#pragma once

#include "Core\Device.h"

class Texture {
public:
	struct SamplerSettings {
		VkSamplerAddressMode addressMode = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		VkBool32 enableAnisotropy = VK_TRUE;
		VkBorderColor borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
		VkFilter filter = VK_FILTER_LINEAR;
		float mipLodBias = 0.f;
	};

	Texture(
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
		VkImageLayout initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
		VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL,
		bool createSampler = true,
		SamplerSettings samplerSettings = {}
	);

	Texture(
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
		);

	~Texture();

	VkImage GetImage() const { return image; }
	VkImageView GetImageView() const { return imageView; }
	VkSampler GetSampler() const { return sampler; }
	VkFormat GetFormat() const { return format; }
	uint32_t Width() const { return width; }
	uint32_t Height() const { return height; }
	uint32_t Depth() const { return depth; }
	uint32_t MipLevels() const { return mipLevels; }
	bool HasSampler() const { return sampler != VK_NULL_HANDLE; }

	VkDescriptorImageInfo GetDescriptorInfo() const;

	void TransitionLayout(VkImageLayout newLayout);
	void FillMipMaps();

	static std::unique_ptr<Texture> Load(
		Device& device,
		const std::string& filename,
		VkImageUsageFlags usage,
		Device::QueueFamilyIndices::Family families,
		bool enableMipmapping = true,
		bool loadHDR = false,
		bool createSampler = true,
		SamplerSettings samplerSettings = {}
	);

private:
	VkImage image;
	VkImageView imageView;
	VkDeviceMemory memory = VK_NULL_HANDLE;
	VkSampler sampler = VK_NULL_HANDLE;

	uint32_t width, height, depth, mipLevels;
	VkImageAspectFlags aspect;
	VkImageType imageType;
	VkImageViewType imageViewType;
	VkImageLayout currentLayout;
	VkFormat format;

	Device& device;
};