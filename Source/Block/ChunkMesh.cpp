#include "ChunkMesh.h"
#include "Block.h"
#include "ChunkManager.h"
#include "GFX/Vertex.h"

constexpr int TEXTURE_ATLAS_SIZE = 8;

const std::vector<Block> blocks = {
	{ "Grass", { { 2.f / (float)TEXTURE_ATLAS_SIZE, 0.f }, { 1.f / (float)TEXTURE_ATLAS_SIZE, 0.f }, { 0.f, 0.f }, { 0.f, 0.f }, { 0.f, 0.f }, { 0.f, 0.f } } },
	{ "Dirt", { { 1.f / (float)TEXTURE_ATLAS_SIZE, 0.f }, { 1.f / (float)TEXTURE_ATLAS_SIZE, 0.f }, { 1.f / (float)TEXTURE_ATLAS_SIZE, 0.f }, { 1.f / (float)TEXTURE_ATLAS_SIZE, 0.f }, { 1.f / (float)TEXTURE_ATLAS_SIZE, 0.f }, { 1.f / (float)TEXTURE_ATLAS_SIZE, 0.f } } },
	{ "Stone", { { 4.f / (float)TEXTURE_ATLAS_SIZE, 0.f }, { 4.f / (float)TEXTURE_ATLAS_SIZE, 0.f }, { 4.f / (float)TEXTURE_ATLAS_SIZE, 0.f }, { 4.f / (float)TEXTURE_ATLAS_SIZE, 0.f }, { 4.f / (float)TEXTURE_ATLAS_SIZE, 0.f }, { 4.f / (float)TEXTURE_ATLAS_SIZE, 0.f } } },
	{ "Water", { { 3.f / (float)TEXTURE_ATLAS_SIZE, 0.f }, { 3.f / (float)TEXTURE_ATLAS_SIZE, 0.f }, { 3.f / (float)TEXTURE_ATLAS_SIZE, 0.f }, { 3.f / (float)TEXTURE_ATLAS_SIZE, 0.f }, { 3.f / (float)TEXTURE_ATLAS_SIZE, 0.f }, { 3.f / (float)TEXTURE_ATLAS_SIZE, 0.f } }, Block::Flags(Block::TRANSPARENT | Block::LIQUID) },
	{ "Log", { { 6.f / (float)TEXTURE_ATLAS_SIZE, 0.f }, { 6.f / (float)TEXTURE_ATLAS_SIZE, 0.f }, { 7.f / (float)TEXTURE_ATLAS_SIZE, 0.f }, { 7.f / (float)TEXTURE_ATLAS_SIZE, 0.f }, { 7.f / (float)TEXTURE_ATLAS_SIZE, 0.f }, { 7.f / (float)TEXTURE_ATLAS_SIZE, 0.f } } },
	{ "Leaves", { { 0.f, 1.f / (float)TEXTURE_ATLAS_SIZE }, { 0.f, 1.f / (float)TEXTURE_ATLAS_SIZE }, { 0.f, 1.f / (float)TEXTURE_ATLAS_SIZE }, { 0.f, 1.f / (float)TEXTURE_ATLAS_SIZE }, { 0.f, 1.f / (float)TEXTURE_ATLAS_SIZE }, { 0.f, 1.f / (float)TEXTURE_ATLAS_SIZE } }, Block::HOLES },
	{ "Sand", { { 5.f / (float)TEXTURE_ATLAS_SIZE, 0.f }, { 5.f / (float)TEXTURE_ATLAS_SIZE, 0.f }, { 5.f / (float)TEXTURE_ATLAS_SIZE, 0.f }, { 5.f / (float)TEXTURE_ATLAS_SIZE, 0.f }, { 5.f / (float)TEXTURE_ATLAS_SIZE, 0.f }, { 5.f / (float)TEXTURE_ATLAS_SIZE, 0.f } } },
	{ "Planks", { { 4.f / (float)TEXTURE_ATLAS_SIZE, 1.f / (float)TEXTURE_ATLAS_SIZE }, { 4.f / (float)TEXTURE_ATLAS_SIZE, 1.f / (float)TEXTURE_ATLAS_SIZE }, { 4.f / (float)TEXTURE_ATLAS_SIZE, 1.f / (float)TEXTURE_ATLAS_SIZE }, { 4.f / (float)TEXTURE_ATLAS_SIZE, 1.f / (float)TEXTURE_ATLAS_SIZE }, { 4.f / (float)TEXTURE_ATLAS_SIZE, 1.f / (float)TEXTURE_ATLAS_SIZE }, { 4.f / (float)TEXTURE_ATLAS_SIZE, 1.f / (float)TEXTURE_ATLAS_SIZE } } },
	{ "Cobblestone", { { 5.f / (float)TEXTURE_ATLAS_SIZE, 1.f / (float)TEXTURE_ATLAS_SIZE }, { 5.f / (float)TEXTURE_ATLAS_SIZE, 1.f / (float)TEXTURE_ATLAS_SIZE }, { 5.f / (float)TEXTURE_ATLAS_SIZE, 1.f / (float)TEXTURE_ATLAS_SIZE }, { 5.f / (float)TEXTURE_ATLAS_SIZE, 1.f / (float)TEXTURE_ATLAS_SIZE }, { 5.f / (float)TEXTURE_ATLAS_SIZE, 1.f / (float)TEXTURE_ATLAS_SIZE }, { 5.f / (float)TEXTURE_ATLAS_SIZE, 1.f / (float)TEXTURE_ATLAS_SIZE } } },
	{ "Glass", { { 1.f / (float)TEXTURE_ATLAS_SIZE, 1.f / (float)TEXTURE_ATLAS_SIZE }, { 1.f / (float)TEXTURE_ATLAS_SIZE, 1.f / (float)TEXTURE_ATLAS_SIZE }, { 1.f / (float)TEXTURE_ATLAS_SIZE, 1.f / (float)TEXTURE_ATLAS_SIZE }, { 1.f / (float)TEXTURE_ATLAS_SIZE, 1.f / (float)TEXTURE_ATLAS_SIZE }, { 1.f / (float)TEXTURE_ATLAS_SIZE, 1.f / (float)TEXTURE_ATLAS_SIZE }, { 1.f / (float)TEXTURE_ATLAS_SIZE, 1.f / (float)TEXTURE_ATLAS_SIZE } }, Block::TRANSPARENT }
};

//Block mesh constants
const std::vector<glm::vec3> blockCorners = {
	{ 0.f, 0.f, 0.f },
	{ 1.f, 0.f, 0.f },
	{ 0.f, 1.f, 0.f },
	{ 0.f, 0.f, 1.f },
	{ 1.f, 1.f, 0.f },
	{ 0.f, 1.f, 1.f },
	{ 1.f, 0.f, 1.f },
	{ 1.f, 1.f, 1.f }
};

const std::vector<glm::vec3> blockNormals = {
	{ 0.f, 1.f, 0.f },
	{ 0.f, -1.f, 0.f },
	{ 1.f, 0.f, 0.f },
	{ -1.f, 0.f, 0.f },
	{ 0.f, 0.f, 1.f },
	{ 0.f, 0.f, -1.f }
};

const std::vector<glm::vec2> blockUvs = {
	{ 0.f, 1.f / (float)TEXTURE_ATLAS_SIZE },
	{ 0.f, 0.f },
	{ 1.f / (float)TEXTURE_ATLAS_SIZE, 0.f },
	{ 1.f / (float)TEXTURE_ATLAS_SIZE, 1.f / (float)TEXTURE_ATLAS_SIZE }
};

const std::vector<uint32_t> blockIndices = {
	2, 5, 7, 4, //Top
	1, 6, 3, 0, //Bottom
	1, 4, 7, 6, //Right
	3, 5, 2, 0, //Left
	6, 7, 5, 3, //Front
	0, 2, 4, 1 //Back
};

const std::vector<uint32_t> indices = {
	0, 1, 2, 2, 3, 0
};

ChunkMesh::ChunkMesh(Device& device, glm::ivec2 pos, ChunkManager& manager) : device(device), pos(pos), manager(manager) {
	meshData.resize(Swapchain::MAX_FRAMES_IN_FLIGHT);
}

ChunkMesh::~ChunkMesh() {

}

void ChunkMesh::Update(const UpdateEvent& event) {
	std::vector<Vertex> vertices;
	vertices.reserve(manager.NumBlocks(pos) * 24);
	std::vector<uint32_t> indices;
	indices.reserve(manager.NumBlocks(pos) * 36);

	std::vector<Vertex> transparentVertices;
	transparentVertices.reserve(1024);
	std::vector<uint32_t> transparentIndices;
	transparentIndices.reserve(1536);
	
	for (int x = 0; x < CHUNK_SIZE; x++) {
		for (int z = 0; z < CHUNK_SIZE; z++) {
			for (int y = 0; y < MAX_BLOCK_HEIGHT; y++) {
				glm::vec3 blockPos{ x, y, z };
				BlockID blockID = manager.BlockAt(glm::ivec3(x + this->pos.x * CHUNK_SIZE, y, z + this->pos.y * CHUNK_SIZE));
				if (blockID > 0) {
					const Block& block = ::blocks[blockID - 1];
					for (int s = 0; s < 6; s++) {
						BlockID sideBlock = manager.BlockAt(glm::ivec3(blockPos + blockNormals[s]) + glm::ivec3(this->pos.x * CHUNK_SIZE, 0, this->pos.y * CHUNK_SIZE));
						if (sideBlock == 0
							|| (sideBlock > 0 && (blocks[sideBlock - 1].flags & Block::HOLES))
							|| (sideBlock > 0 && (blocks[sideBlock - 1].flags & Block::TRANSPARENT) && sideBlock != blockID)
							) {
							if (block.flags & Block::TRANSPARENT) {
								for (int i = 0; i < 6; i++) {
									transparentIndices.emplace_back(::indices[i] + transparentVertices.size());
								}
							}
							else {
								for (int i = 0; i < 6; i++) {
									indices.emplace_back(::indices[i] + vertices.size());
								}
							}

							if (block.flags & Block::TRANSPARENT) {
								for (int j = 0; j < 4; j++) {
									Vertex vertex{};
									vertex.pos = blockCorners[blockIndices[s * 4 + j]] + blockPos;
									if ((block.flags & Block::LIQUID) && vertex.pos.y == 1.f + blockPos.y) {
										vertex.pos.y -= 0.0625f;
									}
									vertex.color = { 1.f, 1.f, 1.f };
									vertex.normal = blockNormals[s];
									vertex.uv = blockUvs[j] + block.textureOffsets[s];

									transparentVertices.emplace_back(vertex);
								}
							}
							else {
								for (int j = 0; j < 4; j++) {
									Vertex vertex{};
									vertex.pos = blockCorners[blockIndices[s * 4 + j]] + blockPos;
									if ((block.flags & Block::LIQUID) && vertex.pos.y == 1.f + blockPos.y) {
										vertex.pos.y -= 0.0625f;
									}
									vertex.color = { 1.f, 1.f, 1.f };
									vertex.normal = blockNormals[s];
									vertex.uv = blockUvs[j] + block.textureOffsets[s];

									vertices.emplace_back(vertex);
								}
							}
						}
					}
				}
			}
		}
	}

	//TODO: use a custom allocator for the buffers
	VkDeviceSize bufferSize = sizeof(Vertex) * (vertices.size() + transparentVertices.size()) + sizeof(uint32_t) * (indices.size() + transparentIndices.size());

	meshData[event.frameIndex] = std::make_unique<Buffer>(
		device,
		bufferSize,
		1,
		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
		Device::QueueFamilyIndices::Graphics
		);

	indexOffset = vertices.size() * sizeof(Vertex);
	transparentVertexOffset = indexOffset + indices.size() * sizeof(uint32_t);
	transparentIndexOffset = transparentVertexOffset + transparentVertices.size() * sizeof(Vertex);

	meshData[event.frameIndex]->Map();
	meshData[event.frameIndex]->WriteToBuffer((void*)vertices.data(), vertices.size() * sizeof(Vertex), 0);
	meshData[event.frameIndex]->WriteToBuffer((void*)indices.data(), indices.size() * sizeof(uint32_t), indexOffset);
	meshData[event.frameIndex]->WriteToBuffer((void*)transparentVertices.data(), transparentVertices.size() * sizeof(Vertex), transparentVertexOffset);
	meshData[event.frameIndex]->WriteToBuffer((void*)transparentIndices.data(), transparentIndices.size() * sizeof(uint32_t), transparentIndexOffset);
	meshData[event.frameIndex]->UnMap();
	
	mostRecentMesh = event.frameIndex;
	shouldUpdate = false;
	loaded = true;
}

void ChunkMesh::Draw(const RenderEvent& event) {
	VkBuffer vertexBuffer[] = { meshData[mostRecentMesh]->GetBuffer() };
	VkDeviceSize offset[] = { 0 };
	vkCmdBindVertexBuffers(event.commandBuffer, 0, 1, vertexBuffer, offset);
	vkCmdBindIndexBuffer(event.commandBuffer, meshData[mostRecentMesh]->GetBuffer(), indexOffset, VK_INDEX_TYPE_UINT32);
	vkCmdDrawIndexed(event.commandBuffer, (transparentVertexOffset - indexOffset) / sizeof(uint32_t), 1, 0, 0, 0);
	//TODO:
	//mesh[mostRecentMesh].reset();
}

void ChunkMesh::DrawTransparent(const RenderEvent& event) {
	if (transparentIndexOffset < meshData[mostRecentMesh]->GetBufferSize()) {
		VkBuffer vertexBuffer[] = { meshData[mostRecentMesh]->GetBuffer() };
		VkDeviceSize offset[] = { transparentVertexOffset };
		vkCmdBindVertexBuffers(event.commandBuffer, 0, 1, vertexBuffer, offset);
		vkCmdBindIndexBuffer(event.commandBuffer, meshData[mostRecentMesh]->GetBuffer(), transparentIndexOffset, VK_INDEX_TYPE_UINT32);
		vkCmdDrawIndexed(event.commandBuffer, (meshData[mostRecentMesh]->GetBufferSize() - transparentIndexOffset) / sizeof(uint32_t), 1, 0, 0, 0);
	}
	//TODO:
	//mesh[mostRecentMesh].reset();
}