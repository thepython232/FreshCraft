#include "App.h"
#include "Events.h"
#include "Systems\ChunkRenderer.h"

constexpr int TPS = 100;

App::App() {
	pool = DescriptorPool::Builder(device)
		.SetMaxSets(Swapchain::MAX_FRAMES_IN_FLIGHT)
		.AddPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, Swapchain::MAX_FRAMES_IN_FLIGHT)
		.Build();

	layout = DescriptorSetLayout::Builder(device)
		.AddBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL)
		.Build();

	uniformBuffers.resize(Swapchain::MAX_FRAMES_IN_FLIGHT);

	for (int i = 0; i < uniformBuffers.size(); i++) {
		uniformBuffers[i] = std::make_unique<Buffer>(
			device,
			sizeof(GlobalUBO),
			1,
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
			Device::QueueFamilyIndices::Graphics,
			device.Limits().minUniformBufferOffsetAlignment
			);
		uniformBuffers[i]->Map();
	}

	sets.resize(Swapchain::MAX_FRAMES_IN_FLIGHT);
	for (int i = 0; i < sets.size(); i++) {
		auto bufferInfo = uniformBuffers[i]->GetDescriptorInfo();
		DescriptorBuilder(*pool, *this->layout)
			.WriteBuffer(0, bufferInfo)
			.Build(sets[i]);
	}

	systems.push_back(std::make_unique<ChunkRenderer>(device, renderer, cache, layout->GetLayout(), chunkManager));

	lastTick = lastFrameUpdate = startTime = std::chrono::high_resolution_clock::now();

	initialized = true;
}

App::~App() {
	initialized = false;
}

void App::Run() {
	startTime = std::chrono::high_resolution_clock::now();

	while (!window.ShouldClose()) {
		glfwPollEvents();
		ReDraw();
	}

	vkDeviceWaitIdle(device.GetDevice());
}

void App::ReDraw() {
	if (auto commandBuffer = renderer.BeginFrame()) {
		uint32_t frameIndex = renderer.GetFrameIndex();
		float elapsedTime = std::chrono::duration<float, std::chrono::seconds::period>(std::chrono::high_resolution_clock::now() - startTime).count();

		if (oldAspect != renderer.GetAspect()) {
			camera.UpdateProj(renderer.GetAspect());
		}
		oldAspect = renderer.GetAspect();

		GlobalUBO ubo{};
		ubo.view = camera.View();
		ubo.invView = camera.InvView();
		ubo.proj = camera.Proj();
		ubo.fogClose = (RENDER_DISTANCE - 2.5f) * (float)CHUNK_SIZE;
		ubo.fogDist = 10.f;
		const float* color = renderer["Global"]["Color"].clearValue.color.float32;
		ubo.fogColor = glm::vec4(color[0], color[1], color[2], 1.f);

		uniformBuffers[frameIndex]->WriteToBuffer(&ubo);
		uniformBuffers[frameIndex]->Flush();

		float deltaTime = std::chrono::duration<float, std::chrono::seconds::period>(std::chrono::high_resolution_clock::now() - lastFrameUpdate).count();
		lastFrameUpdate = std::chrono::high_resolution_clock::now();

		UpdateEvent updateEvent{
			elapsedTime,
			deltaTime,
			frameIndex,
			window.GetInputSystem(),
			camera
		};

		window.PreUpdate(updateEvent);

		cameraController.Update(updateEvent);

		chunkManager.Update(updateEvent);

		std::sort(systems.begin(), systems.end(), [](const std::unique_ptr<RenderSystemBase>& a, const std::unique_ptr<RenderSystemBase>& b) {
			return a->UpdateWeight() < b->UpdateWeight();
			});

		for (auto& system : systems) {
			system->Update(updateEvent);
		}

		float frameTime = std::chrono::duration<float, std::chrono::seconds::period>(std::chrono::high_resolution_clock::now() - lastTick).count();
		lastTick = std::chrono::high_resolution_clock::now();

		float numTicks = std::floor(frameTime * TPS + lastTickError);
		lastTickError = frameTime * TPS + lastTickError - numTicks;

		TickEvent tickEvent{
			elapsedTime,
			window.GetInputSystem(),
			camera
		};

		std::sort(systems.begin(), systems.end(), [](const std::unique_ptr<RenderSystemBase>& a, const std::unique_ptr<RenderSystemBase>& b) {
			return a->TickWeight() < b->TickWeight();
			});

		for (int i = 0; i < (int)numTicks; i++) {
			cameraController.Tick(tickEvent);
			for (auto& system : systems) {
				system->Tick(tickEvent);
			}
		}

		window.PostUpdate(updateEvent);

		for (auto& passName : renderer) {
			Renderer::Pass& pass = renderer[passName];
			pass.Begin(commandBuffer);

			for (uint32_t i = 0; i < pass.NumSubasses(); i++) {
				//Sort systems by their weight for this subpass
				std::sort(systems.begin(), systems.end(), [&passName, &i](const std::unique_ptr<RenderSystemBase>& a, const std::unique_ptr<RenderSystemBase>& b) {
					return a->GetWeight(passName, i) < b->GetWeight(passName, i);
					});

				RenderEvent renderEvent{
					elapsedTime,
					frameIndex,
					commandBuffer,
					pass,
					passName,
					i,
					ubo,
					sets[frameIndex],
					camera
				};

				for (auto& system : systems) {
					system->Render(renderEvent);
				}

				if (i < pass.NumSubasses() - 1)
					vkCmdNextSubpass(commandBuffer, VK_SUBPASS_CONTENTS_INLINE);
			}
			vkCmdEndRenderPass(commandBuffer);
		}
		renderer.EndFrame(commandBuffer);
	}
}