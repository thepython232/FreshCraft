#pragma once

#include "Core\Device.h"
#include "Core\Events.h"
#include "ChunkMesh.h"
#include "Block.h"
#include "Noise\Noise.h"

constexpr int RENDER_DISTANCE = 12;
constexpr int MAX_SORTED_CHUNKS = 2;

struct BlockHitInfo {
	glm::ivec2 chunkID;
	glm::ivec3 blockPos;
	Block block;
	BlockID blockID;
	float dist;
	glm::vec3 worldPos;
	glm::vec3 normal;
};

inline glm::ivec3 BlockToChunk(const glm::ivec3& block, glm::ivec2& chunk);

class ChunkManager {
public:
	ChunkManager(Device& device);

	void Update(const UpdateEvent& event);

	bool VoxelRaytrace(glm::vec3 pos, const glm::vec3& dir, float tMax, BlockHitInfo& info) const;
	void BreakBlock(const glm::ivec3& pos, const UpdateEvent& event);
	void PlaceBlock(const glm::ivec3& pos, BlockID block, const UpdateEvent& event);

	inline BlockID BlockAt(const glm::ivec3& pos);
	inline uint32_t NumBlocks(const glm::ivec2& chunk) const;
	uint32_t MaxBlockHeight(const glm::ivec2& chunk) const;

private:
	void GenerateChunk(const glm::ivec2& chunkID);

	//TODO: offload to a file when full
	std::unordered_map<glm::ivec2, std::array<BlockID, CHUNK_SIZE * CHUNK_SIZE * MAX_BLOCK_HEIGHT>> world;
	std::unordered_map<glm::ivec2, bool> loadedChunks;

	//TOOD: because these don't actually have world data, if these get far enough from the player,
	//they could be destroyed to conserve memory
	//Ordered by distance from the camera
	std::unordered_map<glm::ivec2, std::unique_ptr<ChunkMesh>> chunks;
	std::vector<glm::ivec2> chunksToUpdate;
	glm::ivec2 oldPlayerChunk;
	glm::ivec3 oldPlayerPos;
	Device& device;
	SimplexNoise height{ 0.006f, 10.f, 2.1f, 0.45f }, detail{ 1.f, 1.f, 1.8f, 0.6f }, sand{ 0.006f, 1.f };
	friend class ChunkRenderer;
};

inline glm::ivec3 BlockToChunk(const glm::ivec3& block, glm::ivec2& chunk) {
	glm::ivec3 blockPos = block;
	blockPos.x %= CHUNK_SIZE;
	if (blockPos.x < 0) blockPos.x += CHUNK_SIZE;
	blockPos.z %= CHUNK_SIZE;
	if (blockPos.z < 0) blockPos.z += CHUNK_SIZE;

	chunk = glm::ivec2(block.x / CHUNK_SIZE, block.z / CHUNK_SIZE);
	if (block.x < 0 && blockPos.x > 0)
		chunk.x -= 1;
	if (block.z < 0 && blockPos.z > 0)
		chunk.y -= 1;

	return blockPos;
}

inline BlockID ChunkManager::BlockAt(const glm::ivec3& pos) {
	if (pos.y < 0 || pos.y >= MAX_BLOCK_HEIGHT) return 0;

	glm::ivec2 chunkID;
	glm::ivec3 blockPos = BlockToChunk(pos, chunkID);

	if (!world.contains(chunkID)) {
		GenerateChunk(chunkID);
		return BlockAt(pos);
	}

	return world[chunkID][blockPos.y * CHUNK_SIZE * CHUNK_SIZE + blockPos.z * CHUNK_SIZE + blockPos.x];
}

inline uint32_t ChunkManager::NumBlocks(const glm::ivec2& chunk) const {
	if (!world.contains(chunk)) return 0;

	uint32_t num = 0;
	auto& chunkData = world.find(chunk)->second;
	for (int i = 0; i < CHUNK_SIZE * CHUNK_SIZE * MAX_BLOCK_HEIGHT; i++) {
		if (chunkData[i] > 0) num++;
	}

	return num;
}