#pragma once

#include "Device.h"
#include "Events.h"
#include "ChunkMesh.h"
#include "Block.h"
#include "Noise.h"

constexpr int RENDER_DISTANCE = 8;

struct ChunkComparator {
	bool operator()(const glm::ivec2& a, const glm::ivec2& b) const {
		return glm::length(glm::vec2(a)) < glm::length(glm::vec2(b));
	}
};

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

private:
	void GenerateChunk(const glm::ivec2& chunkID);

	//TODO: offload to a file when full
	std::unordered_map<glm::ivec2, std::array<BlockID, CHUNK_SIZE * CHUNK_SIZE * MAX_BLOCK_HEIGHT>> world;
	std::unordered_map<glm::ivec2, bool> loadedChunks;

	//TOOD: because these don't actually have world data, if these get far enough from the player,
	//they could be destroyed to conserve memory
	std::unordered_map<glm::ivec2, std::unique_ptr<ChunkMesh>> chunks;
	std::multiset<glm::ivec2, ChunkComparator> chunksToUpdate;
	Device& device;
	SimplexNoise height{ 0.006f, 10.f, 2.1f, 0.45f }, detail{ 0.002f, 10.f, 1.8f, 0.6f }, sand;
	unsigned int seed;
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

	return world.find(chunkID)->second[blockPos.y * CHUNK_SIZE * CHUNK_SIZE + blockPos.z * CHUNK_SIZE + blockPos.x];
}