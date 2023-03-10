#include "Renderer.h"

Renderer::Pass::Builder::Builder(Renderer& renderer, VkExtent2D extent) : renderer(renderer) {
	if (extent.width == UINT32_MAX || extent.height == UINT32_MAX)
		this->extent = renderer.GetExtent();
	else
		this->extent = extent;
}

Renderer::Pass::Builder& Renderer::Pass::Builder::SetSubpassCount(int numSubpasses) {
	subpassCount = numSubpasses;
	return *this;
}

Renderer::Pass::Builder& Renderer::Pass::Builder::AddDependency(
	uint32_t srcSubpass,
	VkAccessFlags srcAccess,
	VkPipelineStageFlags srcStage,
	uint32_t dstSubpass,
	VkAccessFlags dstAccess,
	VkPipelineStageFlags dstStage
) {
	VkSubpassDependency dependency{};
	dependency.srcSubpass = srcSubpass;
	dependency.srcAccessMask = srcAccess;
	dependency.srcStageMask = srcStage;
	dependency.dstSubpass = dstSubpass;
	dependency.dstAccessMask = dstAccess;
	dependency.dstStageMask = dstStage;
	dependencies.push_back(dependency);
	return *this;
}

Renderer::Pass::Builder& Renderer::Pass::Builder::AddAttachment(
	std::string name,
	VkFormat format,
	VkImageUsageFlags additionalUsage,
	std::initializer_list<std::pair<int, AttachmentInfo::Type>> subpasses,
	uint32_t flags,
	VkClearValue clearValue,
	Texture::SamplerSettings samplerSettings
) {
	AttachmentInfo info{};
	info.flags = flags;
	info.clearValue = clearValue;
	info.subpasses = subpasses;
	info.name = name;

	VkImageAspectFlags aspect = 0;
	VkImageUsageFlags usage = additionalUsage | ((info.flags & AttachmentInfo::Sampled) ? VK_IMAGE_USAGE_SAMPLED_BIT : 0);
	if (IsDepthFormat(format)) {
		aspect |= VK_IMAGE_ASPECT_DEPTH_BIT;
		usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
	}
	if (IsStencilFormat(format)) {
		aspect |= VK_IMAGE_ASPECT_STENCIL_BIT;
		usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
	}
	if (IsColorFormat(format)) {
		aspect |= VK_IMAGE_ASPECT_COLOR_BIT;
		usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	}

	info.textures.resize(renderer.swapchain->GetImageCount());
	for (int i = 0; i < info.textures.size(); i++) {
		info.textures[i] = std::make_unique<Texture>(
			renderer.device,
			extent.width,
			extent.height,
			1,
			1,
			format,
			aspect,
			usage,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			Device::QueueFamilyIndices::Graphics,
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_TILING_OPTIMAL,
			usage & VK_IMAGE_USAGE_SAMPLED_BIT,
			samplerSettings
		);
	}

	attachments.push_back(std::move(info));

	return *this;

}

Renderer::Pass::Builder& Renderer::Pass::Builder::AddSwapAttachment(
	std::string name,
	std::initializer_list<std::pair<int, AttachmentInfo::Type>> subpasses,
	uint32_t flags,
	VkClearColorValue clear
) {
	AttachmentInfo info{};
	info.flags = flags;
	info.flags |= AttachmentInfo::Store;
	VkClearValue value{};
	value.color = clear;
	info.clearValue = value;
	info.subpasses = subpasses;
	info.isSwap = true;
	info.name = name;

	attachments.push_back(std::move(info));

	return *this;
}

Renderer::Pass::Builder& Renderer::Pass::Builder::AddPassAttachment(
	std::string name,
	Pass& pass,
	std::initializer_list<std::pair<int, AttachmentInfo::Type>> subpasses,
	uint32_t flags,
	VkClearValue clearValue
) {
	AttachmentInfo info{};
	info.flags = flags;
	info.clearValue = clearValue;
	info.subpasses = subpasses;
	info.refTextures = &pass.attachments[name].textures;
	info.name = name;

	attachments.push_back(std::move(info));

	return *this;
}

std::unique_ptr<Renderer::Pass> Renderer::Pass::Builder::Create() {
	return std::make_unique<Pass>(renderer, extent, subpassCount, dependencies, attachments);
}

Renderer::Pass::Pass(
	Renderer& renderer,
	VkExtent2D extent,
	uint32_t subpassCount,
	const std::vector<VkSubpassDependency>& dependencies,
	std::vector<AttachmentInfo>& attachments
) : renderer(renderer), extent(extent), numSubpasses(subpassCount) {
	//Basic structure representing what references a subpass has
	struct SubpassInfo {
		std::vector<VkAttachmentReference> colorRefs;
		std::vector<VkAttachmentReference> resolveRefs;
		std::vector<uint32_t> preserveRefs;
		std::optional<VkAttachmentReference> depthRef;
	};

	std::vector<SubpassInfo> subpassInfos(subpassCount);
	std::vector<VkAttachmentDescription> descriptions;

	//Loop over every attachment that was passed to this pass
	for (int i = 0; i < attachments.size(); i++) {
		AttachmentInfo& info = attachments[i];    //<- DO NOT MAKE THIS const! Textures are moved!!
		Attachment attachment{};
		attachment.flags = info.flags;
		attachment.clearValue = info.clearValue;
		//If the attachment is a swap attachment, create a new set of textures pointing to the swap attachments
		if (info.isSwap) {
			for (uint32_t j = 0; j < renderer.swapchain->GetImageCount(); j++) {
				attachment.textures.push_back(std::make_unique<Texture>(
					renderer.device,
					renderer.swapchain->GetImage(j),
					renderer.swapchain->GetImageView(j),
					renderer.GetExtent().width,
					renderer.GetExtent().height,
					1,
					1,
					renderer.swapchain->GetFormat(),
					VK_IMAGE_ASPECT_COLOR_BIT,
					VK_IMAGE_LAYOUT_UNDEFINED
					));
			}
		}
		else {
			//If not, move the old ones (ref attachments have 0 textures)
			for (auto& tex : info.textures) {
				attachment.textures.push_back(std::move(tex));
			}
		}
		//If we are not referencing a different set of attachments, add it to the descriptions
		//These attachments with ref textures still need to be added to the subpasses though
		if (!info.refTextures)
			this->attachments[info.name] = std::move(attachment);

		//Decide what layout the attachment is in
		VkImageLayout attachmentLayout;
		VkFormat imageFormat = (info.refTextures == nullptr) ? this->attachments[info.name].textures.front()->GetFormat() : info.refTextures->front()->GetFormat();
		if (IsDepthFormat(imageFormat)
			|| IsStencilFormat(imageFormat)) {
			attachmentLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		}
		else {
			attachmentLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		}

		//TODO: stencil operations
		VkAttachmentDescription description{};
		//Even if the attachment is stored, it is still required (by me) to be unused for the next frame
		description.initialLayout = (info.flags & AttachmentInfo::Load) ? attachmentLayout : VK_IMAGE_LAYOUT_UNDEFINED;
		//If the image is a swapchain image, it must be optimal for presentation by the end of the renderpass
		//TODO: don't force it to be
		description.finalLayout = (info.flags & AttachmentInfo::Presentable) ? VK_IMAGE_LAYOUT_PRESENT_SRC_KHR : attachmentLayout;
		if (info.flags & AttachmentInfo::Sampled) {
			if (IsDepthFormat(imageFormat) || IsStencilFormat(imageFormat))
				description.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
			else
				description.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		}
		description.loadOp = (info.flags & AttachmentInfo::Load) ? VK_ATTACHMENT_LOAD_OP_LOAD : (info.flags & AttachmentInfo::Clear) ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		description.storeOp = (info.flags & AttachmentInfo::Store) ? VK_ATTACHMENT_STORE_OP_STORE : VK_ATTACHMENT_STORE_OP_DONT_CARE;
		description.format = imageFormat;
		description.samples = VK_SAMPLE_COUNT_1_BIT;

		descriptions.push_back(description);

		for (uint32_t j = 0; j < subpassCount; j++) {			//Loop over every subpass
			for (const auto& kv : info.subpasses) {			//Loop over each subpass setting
				if (j == kv.first) {						//If they match
					VkAttachmentReference reference{};
					reference.attachment = i;
					reference.layout = attachmentLayout;
					
					switch (kv.second) {
					case Renderer::Pass::AttachmentInfo::Color:
						subpassInfos[kv.first].colorRefs.push_back(reference);
						break;
					case Renderer::Pass::AttachmentInfo::Depth:
						if (subpassInfos[kv.first].depthRef.has_value())
							throw std::runtime_error("Error: attempted to create subpass with multiple depth attachments!");
						subpassInfos[kv.first].depthRef = reference;
						break;
					case Renderer::Pass::AttachmentInfo::Resolve:
						subpassInfos[kv.first].resolveRefs.push_back(reference);
						break;
					case Renderer::Pass::AttachmentInfo::Preserve:
						subpassInfos[kv.first].preserveRefs.push_back(i);
						break;
					}
				}
			}
		}
	}

	std::vector<VkSubpassDescription> subpasses;
	for (const auto& subpassInfo : subpassInfos) {
		VkSubpassDescription subpass{};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = static_cast<uint32_t>(subpassInfo.colorRefs.size());
		subpass.pColorAttachments = subpassInfo.colorRefs.data();
		subpass.pResolveAttachments = subpassInfo.resolveRefs.data();
		subpass.preserveAttachmentCount = static_cast<uint32_t>(subpassInfo.preserveRefs.size());
		subpass.pPreserveAttachments = subpassInfo.preserveRefs.data();
		if (subpassInfo.depthRef.has_value())
			subpass.pDepthStencilAttachment = &subpassInfo.depthRef.value();

		subpasses.push_back(subpass);
	}

	VkRenderPassCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	createInfo.attachmentCount = static_cast<uint32_t>(descriptions.size());
	createInfo.pAttachments = descriptions.data();
	createInfo.subpassCount = static_cast<uint32_t>(subpasses.size());
	createInfo.pSubpasses = subpasses.data();
	createInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
	createInfo.pDependencies = dependencies.data();

	if (vkCreateRenderPass(renderer.device.GetDevice(), &createInfo, nullptr, &renderPass) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create render pass!");
	}

	framebuffers.resize(renderer.swapchain->GetImageCount());
	for (int i = 0; i < framebuffers.size(); i++) {
		std::vector<VkImageView> framebufferAttachments;
		for (const auto& info : attachments) {
			if (info.refTextures)
				framebufferAttachments.push_back(info.refTextures->at(i)->GetImageView());
			else
				framebufferAttachments.push_back(this->attachments[info.name].textures[i]->GetImageView());
		}
		VkFramebufferCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		createInfo.width = extent.width;
		createInfo.height = extent.height;
		createInfo.layers = 1;
		createInfo.renderPass = renderPass;
		createInfo.attachmentCount = static_cast<uint32_t>(framebufferAttachments.size());
		createInfo.pAttachments = framebufferAttachments.data();

		if (vkCreateFramebuffer(renderer.device.GetDevice(), &createInfo, nullptr, &framebuffers[i]) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create framebuffer!");
		}
	}
}

Renderer::Pass::~Pass() {
	for (auto& framebuffer : framebuffers) {
		vkDestroyFramebuffer(renderer.device.GetDevice(), framebuffer, nullptr);
	}

	vkDestroyRenderPass(renderer.device.GetDevice(), renderPass, nullptr);
}

void Renderer::Pass::Begin(VkCommandBuffer commandBuffer) {
	VkRenderPassBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	beginInfo.renderPass = renderPass;
	beginInfo.framebuffer = framebuffers[renderer.imageIndex];
	beginInfo.renderArea.extent = this->extent;
	beginInfo.renderArea.offset = { 0, 0 };
	std::vector<VkClearValue> clearValues;
	for (const auto& kv : attachments) {
		clearValues.push_back(kv.second.clearValue);
	}
	beginInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
	beginInfo.pClearValues = clearValues.data();

	vkCmdBeginRenderPass(commandBuffer, &beginInfo, VK_SUBPASS_CONTENTS_INLINE);

	VkViewport viewport{};
	viewport.x = 0;
	viewport.y = 0;
	viewport.width = static_cast<float>(this->extent.width);
	viewport.height = static_cast<float>(this->extent.height);
	viewport.minDepth = 0.f;
	viewport.maxDepth = 1.f;

	vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

	VkRect2D scissor{};
	scissor.offset = { 0, 0 };
	scissor.extent = this->extent;

	vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
}

Renderer::Renderer(Device& device, Window& window) : device(device), window(window) {
	swapchain = std::make_unique<Swapchain>(device, window);
	AllocateCommandBuffers();

	CreateRenderPasses();
}

Renderer::~Renderer() {
	vkFreeCommandBuffers(device.GetDevice(), device.GetCommandPool(), static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());
}

void Renderer::AllocateCommandBuffers() {
	commandBuffers.resize(Swapchain::MAX_FRAMES_IN_FLIGHT);
	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());
	allocInfo.commandPool = device.GetCommandPool();
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

	if (vkAllocateCommandBuffers(device.GetDevice(), &allocInfo, commandBuffers.data()) != VK_SUCCESS) {
		throw std::runtime_error("Failed to allocate command buffers!");
	}
}

#define CREATE_PASS(_name, _pass) passNames.push_back(_name); passes[_name] = _pass

void Renderer::CreateRenderPasses() {
	VkClearValue depthClear{};
	depthClear.depthStencil = { 1.f, 0 };

	Texture::SamplerSettings shadowSamplerSettings{};
	shadowSamplerSettings.filter = VK_FILTER_NEAREST;
	shadowSamplerSettings.enableAnisotropy = VK_FALSE;
	shadowSamplerSettings.addressMode = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
	shadowSamplerSettings.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE; //In case we see something outside of our range...

	CREATE_PASS("Shadow", Pass::Builder(*this, VkExtent2D{ SHADOWMAP_EXTENT, SHADOWMAP_EXTENT })
		.SetSubpassCount(1)
		.AddAttachment("ShadowDepth", VK_FORMAT_D32_SFLOAT, VK_IMAGE_USAGE_SAMPLED_BIT, { {0, Pass::AttachmentInfo::Depth} }, Pass::AttachmentInfo::Clear | Pass::AttachmentInfo::Store | Pass::AttachmentInfo::Sampled, depthClear, shadowSamplerSettings)
		.Create());

	CREATE_PASS("Global", Pass::Builder(*this)
		.SetSubpassCount(1)
		.AddSwapAttachment("Color", { {0, Pass::AttachmentInfo::Color} }, Pass::AttachmentInfo::Clear | Pass::AttachmentInfo::Store, { 0.35f, 0.77f, 0.93f })
		.AddAttachment("Depth", VK_FORMAT_D32_SFLOAT, 0, { {0, Pass::AttachmentInfo::Depth} }, Pass::AttachmentInfo::Clear, depthClear)
		.Create());

	CREATE_PASS("UI", Pass::Builder(*this)
		.SetSubpassCount(1)
		.AddPassAttachment("Color", *passes["Global"], { {0, Pass::AttachmentInfo::Color} }, Pass::AttachmentInfo::Load | Pass::AttachmentInfo::Store | Pass::AttachmentInfo::Presentable)
		.Create());
}

void Renderer::Recreate() {
	VkExtent2D extent = window.GetFramebufferSize();
	while (extent.width == 0 || extent.height == 0) {
		extent = window.GetFramebufferSize();
	}

	vkDeviceWaitIdle(device.GetDevice());  //TODO: better way of doing this

	swapchain->Recreate();
	passNames.clear();
	CreateRenderPasses();
}

VkCommandBuffer Renderer::BeginFrame() {
	//TODO: resizing
	VkResult result = swapchain->AcquireNextImage(&imageIndex);
	if (result == VK_ERROR_OUT_OF_DATE_KHR) {
		Recreate();
	}
	else if (result != VK_SUBOPTIMAL_KHR && result != VK_SUCCESS) {
		throw std::runtime_error("Failed to acquire swapchain image!");
	}

	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	
	if (vkBeginCommandBuffer(commandBuffers[swapchain->GetFrameIndex()], &beginInfo) != VK_SUCCESS) {
		throw std::runtime_error("Failed to begin command buffer");
	}

	return commandBuffers[swapchain->GetFrameIndex()];
}

void Renderer::EndFrame(VkCommandBuffer commandBuffer) {
	vkEndCommandBuffer(commandBuffer);

	//TODO: resizing
	VkResult result = swapchain->SubmitFrame(commandBuffer, &imageIndex);
	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || window.Resized()) {
		Recreate();
		window.ResetFlags();
	}
	else if (result != VK_SUCCESS) {
		throw std::runtime_error("Failed to submit draw command buffer!");
	}
}