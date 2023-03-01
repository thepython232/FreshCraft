#include "ChunkMesh.h"
#include "Block.h"
#include "ChunkManager.h"

constexpr int TEXTURE_ATLAS_SIZE = 8;

const std::vector<Block> blocks = {
	{ "Grass", { { 2.f / (float)TEXTURE_ATLAS_SIZE, 0.f }, { 1.f / (float)TEXTURE_ATLAS_SIZE, 0.f }, { 0.f, 0.f }, { 0.f, 0.f }, { 0.f, 0.f }, { 0.f, 0.f } } },
	{ "Dirt", { { 1.f / (float)TEXTURE_ATLAS_SIZE, 0.f }, { 1.f / (float)TEXTURE_ATLAS_SIZE, 0.f }, { 1.f / (float)TEXTURE_ATLAS_SIZE, 0.f }, { 1.f / (float)TEXTURE_ATLAS_SIZE, 0.f }, { 1.f / (float)TEXTURE_ATLAS_SIZE, 0.f }, { 1.f / (float)TEXTURE_ATLAS_SIZE, 0.f } } },
	{ "Stone", { { 4.f / (float)TEXTURE_ATLAS_SIZE, 0.f }, { 4.f / (float)TEXTURE_ATLAS_SIZE, 0.f }, { 4.f / (float)TEXTURE_ATLAS_SIZE, 0.f }, { 4.f / (float)TEXTURE_ATLAS_SIZE, 0.f }, { 4.f / (float)TEXTURE_ATLAS_SIZE, 0.f }, { 4.f / (float)TEXTURE_ATLAS_SIZE, 0.f } } },
	{ "Water", { { 3.f / (float)TEXTURE_ATLAS_SIZE, 0.f }, { 3.f / (float)TEXTURE_ATLAS_SIZE, 0.f }, { 3.f / (float)TEXTURE_ATLAS_SIZE, 0.f }, { 3.f / (float)TEXTURE_ATLAS_SIZE, 0.f }, { 3.f / (float)TEXTURE_ATLAS_SIZE, 0.f }, { 3.f / (float)TEXTURE_ATLAS_SIZE, 0.f } }, Block::TRANSPARENT },
	{ "Log", { { 6.f / (float)TEXTURE_ATLAS_SIZE, 0.f }, { 6.f / (float)TEXTURE_ATLAS_SIZE, 0.f }, { 7.f / (float)TEXTURE_ATLAS_SIZE, 0.f }, { 7.f / (float)TEXTURE_ATLAS_SIZE, 0.f }, { 7.f / (float)TEXTURE_ATLAS_SIZE, 0.f }, { 7.f / (float)TEXTURE_ATLAS_SIZE, 0.f } } },
	{ "Leaves", { { 0.f, 1.f / (float)TEXTURE_ATLAS_SIZE }, { 0.f, 1.f / (float)TEXTURE_ATLAS_SIZE }, { 0.f, 1.f / (float)TEXTURE_ATLAS_SIZE }, { 0.f, 1.f / (float)TEXTURE_ATLAS_SIZE }, { 0.f, 1.f / (float)TEXTURE_ATLAS_SIZE }, { 0.f, 1.f / (float)TEXTURE_ATLAS_SIZE } }, Block::HOLES },
	{ "Sand", { { 5.f / (float)TEXTURE_ATLAS_SIZE, 0.f }, { 5.f / (float)TEXTURE_ATLAS_SIZE, 0.f }, { 5.f / (float)TEXTURE_ATLAS_SIZE, 0.f }, { 5.f / (float)TEXTURE_ATLAS_SIZE, 0.f }, { 5.f / (float)TEXTURE_ATLAS_SIZE, 0.f }, { 5.f / (float)TEXTURE_ATLAS_SIZE, 0.f } } }
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
	mesh.resize(Swapchain::MAX_FRAMES_IN_FLIGHT);
	transparentMesh.resize(Swapchain::MAX_FRAMES_IN_FLIGHT);
}

ChunkMesh::~ChunkMesh() {

}

void ChunkMesh::Update(const UpdateEvent& event) {
	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;

	std::vector<Vertex> transparentVertices;
	std::vector<uint32_t> transparentIndices;

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
									transparentIndices.push_back(::indices[i] + transparentVertices.size());
								}
							}
							else {
								for (int i = 0; i < 6; i++) {
									indices.push_back(::indices[i] + vertices.size());
								}
							}

							if (block.flags & Block::TRANSPARENT) {
								for (int j = 0; j < 4; j++) {
									Vertex vertex{};
									vertex.pos = blockCorners[blockIndices[s * 4 + j]] + blockPos;
									if (vertex.pos.y == 1.f + blockPos.y) {
										vertex.pos.y -= 0.0625f;
									}
									vertex.color = { 1.f, 1.f, 1.f };
									vertex.normal = blockNormals[s];
									vertex.uv = blockUvs[j] + block.textureOffsets[s];

									transparentVertices.push_back(vertex);
								}
							}
							else {
								for (int j = 0; j < 4; j++) {
									Vertex vertex{};
									vertex.pos = blockCorners[blockIndices[s * 4 + j]] + blockPos;
									vertex.color = { 1.f, 1.f, 1.f };
									vertex.normal = blockNormals[s];
									vertex.uv = blockUvs[j] + block.textureOffsets[s];

									vertices.push_back(vertex);
								}
							}
						}
					}
				}
			}
		}
	}

	mesh[event.frameIndex] = std::make_unique<Mesh>(device, vertices, indices);
	if (transparentVertices.size() > 0) {
		transparentMesh[event.frameIndex] = std::make_unique<Mesh>(device, transparentVertices, transparentIndices);
	}
	mostRecentMesh = event.frameIndex;
	shouldUpdate = false;
	loaded = true;
}

void ChunkMesh::Draw(const RenderEvent& event) {
	mesh[mostRecentMesh]->Draw(event.commandBuffer);
	//TODO:
	//mesh[mostRecentMesh].reset();
}

void ChunkMesh::DrawTransparent(const RenderEvent& event) {
	if (transparentMesh[mostRecentMesh] != nullptr) {
		transparentMesh[mostRecentMesh]->Draw(event.commandBuffer);
	}
	//TODO:
	//mesh[mostRecentMesh].reset();
}