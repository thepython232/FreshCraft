#pragma once

#include "Core\Device.h"
#include "Core\Swapchain.h"
#include "Texture.h"

constexpr int SHADOWMAP_EXTENT = 4096;

class Renderer {
public:
	//Defines a specific rendering pass, and includes associated renderPass, framebuffers, and other resources/functions
	class Pass {
	public:
		struct AttachmentInfo {
			enum Type {
				Color,
				Depth,
				Resolve,
				Preserve
			};

			enum Flags {
				Clear = 1 << 0,
				Store = 1 << 1,
				Load = 1 << 2,
				Sampled = 1 << 3,
				Presentable = 1 << 4
			};

			bool isSwap = false;
			std::vector<std::unique_ptr<Texture>> textures;
			std::vector<std::unique_ptr<Texture>>* refTextures = nullptr;
			uint32_t flags;
			VkClearValue clearValue;
			std::initializer_list<std::pair<int, Type>> subpasses;
			std::string name;
		};

		struct Attachment {
			std::vector<std::unique_ptr<Texture>> textures;
			VkExtent2D extent;
			uint32_t flags;
			VkClearValue clearValue;
		};

		class Builder {
		public:
			Builder(Renderer& renderer, VkExtent2D extent = { UINT32_MAX, UINT32_MAX });

			Builder& SetSubpassCount(int numSubpasses);

			Builder& AddDependency(
				uint32_t srcSubpass,
				VkAccessFlags srcAccess,
				VkPipelineStageFlags srcStage,
				uint32_t dstSubpass,
				VkAccessFlags dstAccess,
				VkPipelineStageFlags dstStage);

			Builder& AddAttachment(
				std::string name,
				VkFormat format,
				VkImageUsageFlags additionalUsage,
				std::initializer_list<std::pair<int, AttachmentInfo::Type>> subpasses,
				uint32_t flags,
				VkClearValue clearValue = {},
				Texture::SamplerSettings samplerSettings
				= { VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_FALSE, VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK, VK_FILTER_NEAREST, 0.f }
		);

			Builder& AddSwapAttachment(
				std::string name,
				std::initializer_list<std::pair<int, AttachmentInfo::Type>> subpasses,
				uint32_t flags,
				VkClearColorValue clear = {});

			Builder& AddPassAttachment(
				std::string name,
				Pass& pass,
				std::initializer_list<std::pair<int, AttachmentInfo::Type>> subpasses,
				uint32_t flags,
				VkClearValue clearValue = {});

			std::unique_ptr<Pass> Create();

		private:
			uint32_t subpassCount;
			std::vector<VkSubpassDependency> dependencies;
			std::vector<AttachmentInfo> attachments;
			VkExtent2D extent;
			Renderer& renderer;
		};

		Pass(
			Renderer& renderer,
			VkExtent2D extent,
			uint32_t subpassCount,
			const std::vector<VkSubpassDependency>& dependencies,
			std::vector<AttachmentInfo>& attachments);

		~Pass();

		void Begin(VkCommandBuffer commandBuffer);

		VkDescriptorImageInfo GetAttachmentInfo(std::string name, uint32_t frameIndex) {
			return attachments[name].textures[frameIndex]->GetDescriptorInfo();
		}

		const Attachment& operator[](std::string name) {
			return attachments[name];
		}

		uint32_t NumSubasses() const { return numSubpasses; }
		VkRenderPass GetRenderPass() const { return renderPass; }

	private:
		VkExtent2D extent;
		VkRenderPass renderPass;
		uint32_t numSubpasses;
		std::vector<VkFramebuffer> framebuffers;

		std::unordered_map<std::string, Attachment> attachments;
		Renderer& renderer;
	};

	Renderer(Device& device, Window& window);
	~Renderer();

	VkCommandBuffer BeginFrame();
	void EndFrame(VkCommandBuffer commandBuffer);

	Pass& operator[](const std::string& name) {
		return *passes[name];
	}

	bool HasPass(std::string name) { return passes.contains(name); }
	bool HasSubpass(std::string name, uint32_t subpass) const {
		if (!passes.contains(name))
			return false;
		return subpass < passes.find(name)->second->NumSubasses();
	}

	std::vector<std::string>::iterator begin() {
		return passNames.begin();
	}

	std::vector<std::string>::iterator end() {
		return passNames.end();
	}

	float GetAspect() const { return (float)swapchain->GetExtent().width / swapchain->GetExtent().height; }
	uint32_t GetFrameIndex() const { return swapchain->GetFrameIndex(); }
	uint32_t GetImageIndex() const { return imageIndex; }
	uint32_t GetImageCount() const { return swapchain->GetImageCount(); }
	VkExtent2D GetExtent() const { return swapchain->GetExtent(); }

private:
	void AllocateCommandBuffers();
	void CreateRenderPasses();

	void Recreate();

	std::vector<VkCommandBuffer> commandBuffers;
	std::unique_ptr<Swapchain> swapchain;
	std::unordered_map<std::string, std::unique_ptr<Pass>> passes;
	std::vector<std::string> passNames;
	uint32_t imageIndex;
	Device& device;
	Window& window;

	friend Pass;
	friend Pass::Builder;
};