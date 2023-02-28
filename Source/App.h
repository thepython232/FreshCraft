#pragma once

#include "Common.h"
#include "Renderer.h"
#include "Descriptors.h"
#include "RenderSystem.h"
#include "Buffer.h"
#include "Pipeline.h"
#include "ChunkManager.h"
#include "Camera.h"
#include "CameraController.h"

class App {
public:
	App();
	~App();

	void Run();

	void ReDraw();

	bool IsInitialized() const { return initialized; }

private:
	bool initialized = false;
	Window window{ "FreshCraft", 1280, 720, *this };
	Instance instance;
	Device device{ window, instance };
	Renderer renderer{ device, window };
	Camera camera{ 60.f, renderer.GetAspect(), 0.1f, 1000.f, {0.f, 90.f, 0.f} };
	float oldAspect = 0.f;
	PipelineCache cache{ device, "pipeline.cache" };
	ChunkManager chunkManager{ device };
	CameraController cameraController{ camera, chunkManager };
	std::vector<std::unique_ptr<RenderSystemBase>> systems;
	std::vector<std::unique_ptr<Buffer>> uniformBuffers;
	std::unique_ptr<DescriptorPool> pool;
	std::unique_ptr<DescriptorSetLayout> layout;
	std::vector<VkDescriptorSet> sets;
	std::chrono::high_resolution_clock::time_point startTime;
	std::chrono::high_resolution_clock::time_point lastFrameUpdate;
	std::chrono::high_resolution_clock::time_point lastTick;
	float lastTickError;
};