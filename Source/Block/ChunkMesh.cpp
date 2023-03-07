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
	
	int height = manager.MaxBlockHeight(pos);

	for (int x = 0; x < CHUNK_SIZE; x++) {
		for (int z = 0; z < CHUNK_SIZE; z++) {
			for (int y = 0; y < height; y++) {
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
									//Fix transparent blocks on holed blocks
									if (sideBlock > 0 && (blocks[sideBlock - 1].flags & Block::HOLES))
										vertex.pos -= blockNormals[s] * 0.001f;
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
	if (mostRecentMesh == event.frameIndex)
		mostRecentMesh = (event.frameIndex + 1) % Swapchain::MAX_FRAMES_IN_FLIGHT;
	else
		mostRecentMesh = event.frameIndex;

	VkDeviceSize bufferSize = sizeof(Vertex) * (vertices.size() + transparentVertices.size()) + sizeof(uint32_t) * (indices.size() + transparentIndices.size());

	meshData[mostRecentMesh] = std::make_unique<Buffer>(
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

	meshData[mostRecentMesh]->Map();
	meshData[mostRecentMesh]->WriteToBuffer((void*)vertices.data(), vertices.size() * sizeof(Vertex), 0);
	meshData[mostRecentMesh]->WriteToBuffer((void*)indices.data(), indices.size() * sizeof(uint32_t), indexOffset);
	meshData[mostRecentMesh]->WriteToBuffer((void*)transparentVertices.data(), transparentVertices.size() * sizeof(Vertex), transparentVertexOffset);
	meshData[mostRecentMesh]->WriteToBuffer((void*)transparentIndices.data(), transparentIndices.size() * sizeof(uint32_t), transparentIndexOffset);
	meshData[mostRecentMesh]->Flush();
	meshData[mostRecentMesh]->UnMap();

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

glm::vec3 Centroid(glm::vec3 a, glm::vec3 b, glm::vec3 c, glm::vec3 d) {
	return (a + b + c + d) / 4.f;
}

void ChunkMesh::Resort(const UpdateEvent& event) {
	if (transparentVertexOffset != meshData[mostRecentMesh]->GetBufferSize() && Loaded()) {
		uint32_t indexCount = (meshData[mostRecentMesh]->GetBufferSize() - transparentIndexOffset) / sizeof(uint32_t);
		//Dump data out of buffer
		meshData[mostRecentMesh]->Map(meshData[mostRecentMesh]->GetBufferSize() - transparentVertexOffset, transparentVertexOffset);
		meshData[mostRecentMesh]->Invalidate(meshData[mostRecentMesh]->GetBufferSize() - transparentVertexOffset, transparentVertexOffset);
		const Vertex* vertices = reinterpret_cast<const Vertex*>(meshData[mostRecentMesh]->GetMappedMemory());
		uint32_t* indices = reinterpret_cast<uint32_t*>((char*)meshData[mostRecentMesh]->GetMappedMemory() + (transparentIndexOffset - transparentVertexOffset));

		//List of triangle indices
		std::vector<Triangle> tris;
		for (uint32_t i = 0; i < indexCount; i += 6) {
			tris.push_back({ { indices[i], indices[i + 1], indices[i + 2], indices[i + 3], indices[i + 4], indices[i + 5]}});
		}

		//Sort the tris
		std::sort(tris.begin(), tris.end(), [&event, vertices, this](const Triangle& a, const Triangle& b) {
			return glm::length(Centroid(
				vertices[a.indices[0]].pos + glm::vec3(this->pos.x * CHUNK_SIZE - CHUNK_SIZE / 2, 0.f, this->pos.y * CHUNK_SIZE - CHUNK_SIZE / 2) - event.mainCamera.GetPos(),
				vertices[a.indices[1]].pos + glm::vec3(this->pos.x * CHUNK_SIZE - CHUNK_SIZE / 2, 0.f, this->pos.y * CHUNK_SIZE - CHUNK_SIZE / 2) - event.mainCamera.GetPos(),
				vertices[a.indices[2]].pos + glm::vec3(this->pos.x * CHUNK_SIZE - CHUNK_SIZE / 2, 0.f, this->pos.y * CHUNK_SIZE - CHUNK_SIZE / 2) - event.mainCamera.GetPos(),
				vertices[a.indices[4]].pos + glm::vec3(this->pos.x * CHUNK_SIZE - CHUNK_SIZE / 2, 0.f, this->pos.y * CHUNK_SIZE - CHUNK_SIZE / 2) - event.mainCamera.GetPos()
			)) > glm::length(Centroid(
				vertices[b.indices[0]].pos + glm::vec3(this->pos.x * CHUNK_SIZE - CHUNK_SIZE / 2, 0.f, this->pos.y * CHUNK_SIZE - CHUNK_SIZE / 2) - event.mainCamera.GetPos(),
				vertices[b.indices[1]].pos + glm::vec3(this->pos.x * CHUNK_SIZE - CHUNK_SIZE / 2, 0.f, this->pos.y * CHUNK_SIZE - CHUNK_SIZE / 2) - event.mainCamera.GetPos(),
				vertices[b.indices[2]].pos + glm::vec3(this->pos.x * CHUNK_SIZE - CHUNK_SIZE / 2, 0.f, this->pos.y * CHUNK_SIZE - CHUNK_SIZE / 2) - event.mainCamera.GetPos(),
				vertices[b.indices[4]].pos + glm::vec3(this->pos.x * CHUNK_SIZE - CHUNK_SIZE / 2, 0.f, this->pos.y * CHUNK_SIZE - CHUNK_SIZE / 2) - event.mainCamera.GetPos()
			));
			});

		//Write the data back
		meshData[mostRecentMesh]->WriteToBuffer((void*)tris.data(), indexCount * sizeof(uint32_t), transparentIndexOffset - transparentVertexOffset);
		meshData[mostRecentMesh]->Flush(meshData[mostRecentMesh]->GetBufferSize() - transparentIndexOffset, transparentIndexOffset);
		meshData[mostRecentMesh]->UnMap();
	}

	shouldResort = false;
}