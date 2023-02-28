#pragma once

#include "Core\Device.h"
#include "Core\Swapchain.h"
#include "Texture.h"

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

			bool isSwap = false;
			std::vector<std::unique_ptr<Texture>> textures;
			std::vector<std::unique_ptr<Texture>>* refTextures = nullptr;
			bool clearAtStart;
			bool store;
			VkClearValue clearValue;
			std::initializer_list<std::pair<int, Type>> subpasses;
			std::string name;
		};

		struct Attachment {
			std::vector<std::unique_ptr<Texture>> textures;
			bool clearAtStart;
			VkClearValue clearValue;
		};

		class Builder {
		public:
			Builder(Renderer& renderer);

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
				bool clearAtStart,
				bool store,
				bool sampled,
				VkClearValue clearValue = {});

			Builder& AddSwapAttachment(
				std::string name,
				std::initializer_list<std::pair<int, AttachmentInfo::Type>> subpasses,
				VkClearColorValue clear = {});

			Builder& AddPassAttachment(
				std::string name,
				Pass& pass,
				std::initializer_list<std::pair<int, AttachmentInfo::Type>> subpasses,
				bool clear,
				bool store,
				VkClearValue clearValue = {});

			std::unique_ptr<Pass> Create();

		private:
			uint32_t subpassCount;
			std::vector<VkSubpassDependency> dependencies;
			std::vector<AttachmentInfo> attachments;
			Renderer& renderer;
		};

		Pass(
			Renderer& renderer,
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

	//Used to iterate over every pass (mostly wraps over std::unordered_map<std::string, Pass>::iterator)
	class Iterator {
	public:
		Iterator(std::unordered_map<std::string, std::unique_ptr<Pass>>::iterator iter)
			: iter(iter) { }

		Iterator& operator++() {
			++iter;
			return *this;
		}

		Iterator& operator++(int) {
			iter++;
			return *this;
		}

		bool operator!=(const Iterator& other) const {
			return iter != other.iter;
		}

		const std::string& operator*() {
			return (*iter).first;
		}

		std::unordered_map<std::string, std::unique_ptr<Pass>>::iterator iter;
	};

	Renderer(Device& device, Window& window);
	~Renderer();

	VkCommandBuffer BeginFrame();
	void EndFrame(VkCommandBuffer commandBuffer);

	//TODO: overload operator[] to access RenderPass struct, that can include each renderPass and associated framebuffers
	//TODO: set up so that RenderSystem inherited classes can set a RenderPass when they should render
	Pass& operator[](const std::string& name) {
		return *passes[name];
	}

	Iterator begin() {
		return Iterator(passes.begin());
	}
	Iterator end() {
		return Iterator(passes.end());
	}

	float GetAspect() const { return (float)swapchain->GetExtent().width / swapchain->GetExtent().height; }
	uint32_t GetFrameIndex() const { return swapchain->GetFrameIndex(); }

private:
	void AllocateCommandBuffers();
	void CreateRenderPasses();

	void Recreate();

	std::vector<VkCommandBuffer> commandBuffers;
	std::unique_ptr<Swapchain> swapchain;
	std::unordered_map<std::string, std::unique_ptr<Pass>> passes;
	uint32_t imageIndex;
	Device& device;
	Window& window;

	friend Pass;
	friend Pass::Builder;
};