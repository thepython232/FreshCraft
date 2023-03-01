#pragma once

#include "Core\Buffer.h"
#include "Core\Events.h"

constexpr int CHUNK_SIZE = 16;
constexpr int MAX_BLOCK_HEIGHT = 256;

using BlockID = unsigned char;

class TerrainGenerator;

extern const std::vector<struct Block> blocks;

class ChunkMesh {
public:
	ChunkMesh(Device& device, glm::ivec2 pos, class ChunkManager& manager);
	~ChunkMesh();

	void Draw(const RenderEvent& event);
	void DrawTransparent(const RenderEvent& event);
	void Update(const UpdateEvent& event);

	bool ShouldUpdate() const { return shouldUpdate; }
	bool Loaded() const { return loaded; }

private:
	std::vector<std::unique_ptr<Buffer>> meshData;
	VkDeviceSize indexOffset, transparentVertexOffset, transparentIndexOffset;

	uint32_t mostRecentMesh;
	glm::ivec2 pos;
	bool shouldUpdate = true;
	bool loaded = false;
	Device& device;
	class ChunkManager& manager;
	friend class ChunkManager;
};