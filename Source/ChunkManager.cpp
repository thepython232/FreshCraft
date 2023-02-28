#include "ChunkManager.h"
#include "Raytrace.h"
#include "glm/gtx/integer.hpp"

ChunkManager::ChunkManager(Device& device) : device(device) {
	
}

void ChunkManager::Update(const UpdateEvent& event) {
	for (int x = -RENDER_DISTANCE; x < RENDER_DISTANCE; ++x) {
		for (int z = -RENDER_DISTANCE; z < RENDER_DISTANCE; ++z) {
			glm::ivec2 chunkPos{ x, z };
			chunkPos += glm::vec2(event.mainCamera.GetPos().x, event.mainCamera.GetPos().z) / float(CHUNK_SIZE);
			if (!chunks.contains(chunkPos)) {
				chunks[chunkPos] = std::make_unique<ChunkMesh>(device, chunkPos, *this);
				chunksToUpdate.insert(chunkPos);
			}
		}
	}

	for (const auto& kv : chunks) {
		if (!chunksToUpdate.contains(kv.first) && kv.second->ShouldUpdate()) {
			chunksToUpdate.insert(kv.first);
		}
	}

	//Update closest chunks
	if (!chunksToUpdate.empty()) {
		ChunkMesh& mesh = *chunks[*chunksToUpdate.begin()];
		chunksToUpdate.erase(chunksToUpdate.begin());

		for (int i = 0; i < 9; i++) {
			GenerateChunk(glm::ivec2(i % 3 - 1, i / 3 - 1) + mesh.pos);
		}

		mesh.Update(event);
	}
}

//TODO: broken
bool ChunkManager::VoxelRaytrace(glm::vec3 pos, const glm::vec3& dir, float tMax, BlockHitInfo& info) const {
	pos += glm::vec3{ CHUNK_SIZE / 2, 0, CHUNK_SIZE / 2 };
	glm::vec3 tmpMin = pos;
	glm::vec3 tmpMax = pos + dir * tMax;

	glm::ivec3 min = glm::min(tmpMin, tmpMax);
	glm::ivec3 max = glm::max(tmpMin, tmpMax);

	BlockHitInfo minInfo;
	float minDist = tMax;

	for (int x = min.x; x <= max.x; x++) {
		for (int y = min.y; y <= max.y; y++) {
			for (int z = min.z; z <= max.z; z++) {
				glm::ivec3 blockPos{ x, y, z };
				glm::ivec2 chunkID;
				glm::ivec3 chunkBlockPos = BlockToChunk(blockPos, chunkID);

				if (world.contains(chunkID) && world.find(chunkID)->second[chunkBlockPos.y * CHUNK_SIZE * CHUNK_SIZE + chunkBlockPos.z * CHUNK_SIZE + chunkBlockPos.x] > 0) {
					glm::vec3 minBounds = glm::vec3{ blockPos };
					glm::vec3 maxBounds = glm::vec3{ blockPos } + glm::vec3{ 1.f };

					RectHit hit{};
					if (RectRaytrace(pos, dir, minBounds, maxBounds, hit)) {
						if (hit.t < minDist) {
							minInfo.blockPos = blockPos;
							minInfo.chunkID = chunkID;
							minInfo.blockID = world.find(chunkID)->second[chunkBlockPos.y * CHUNK_SIZE * CHUNK_SIZE + chunkBlockPos.z * CHUNK_SIZE + chunkBlockPos.x];
							minInfo.block = blocks[minInfo.blockID - 1];
							minInfo.normal = hit.normal;
							minInfo.dist = hit.t;
							minInfo.worldPos = hit.pos;

							minDist = hit.t;
						}
					}
				}
			}
		}
	}

	if (minDist < tMax) {
		info = minInfo;
		return true;
	}

	return false;
}

void ChunkManager::BreakBlock(const glm::ivec3& pos, const UpdateEvent& event) {
	glm::ivec2 chunkID;
	glm::ivec3 blockPos = BlockToChunk(pos, chunkID);
	world[chunkID][blockPos.y * CHUNK_SIZE * CHUNK_SIZE + blockPos.z * CHUNK_SIZE + blockPos.x] = 0;
	if (blockPos.x == 0) {
		chunks[chunkID + glm::ivec2(-1, 0)]->shouldUpdate = true;
		chunks[chunkID + glm::ivec2(-1, 0)]->Update(event);
	}
	else if (blockPos.x == CHUNK_SIZE - 1) {
		chunks[chunkID + glm::ivec2(1, 0)]->shouldUpdate = true;
		chunks[chunkID + glm::ivec2(1, 0)]->Update(event);
	}
	if (blockPos.z == 0) {
		chunks[chunkID + glm::ivec2(0, -1)]->shouldUpdate = true;
		chunks[chunkID + glm::ivec2(0, -1)]->Update(event);
	}
	else if (blockPos.z == CHUNK_SIZE - 1) {
		chunks[chunkID + glm::ivec2(0, 1)]->shouldUpdate = true;
		chunks[chunkID + glm::ivec2(0, 1)]->Update(event);
	}
	chunks[chunkID]->shouldUpdate = true;
}

void ChunkManager::PlaceBlock(const glm::ivec3& pos, BlockID block, const UpdateEvent& event) {
	glm::ivec2 chunkID;
	glm::ivec3 blockPos = BlockToChunk(pos, chunkID);

	world[chunkID][blockPos.y * CHUNK_SIZE * CHUNK_SIZE + blockPos.z * CHUNK_SIZE + blockPos.x] = block;
	if (blockPos.x == 0) {
		chunks[chunkID + glm::ivec2(-1, 0)]->shouldUpdate = true;
	}
	else if (blockPos.x == CHUNK_SIZE - 1) {
		chunks[chunkID + glm::ivec2(1, 0)]->shouldUpdate = true;
	}
	if (blockPos.z == 0) {
		chunks[chunkID + glm::ivec2(0, -1)]->shouldUpdate = true;
	}
	else if (blockPos.z == CHUNK_SIZE - 1) {
		chunks[chunkID + glm::ivec2(0, 1)]->shouldUpdate = true;
	}
	chunks[chunkID]->shouldUpdate = true;
}

void ChunkManager::GenerateChunk(const glm::ivec2& chunkID) {
	auto start = std::chrono::high_resolution_clock::now();

	if (!world.contains(chunkID) || loadedChunks[chunkID] == false) {
		for (int x = 0; x < CHUNK_SIZE; x++) {
			for (int z = 0; z < CHUNK_SIZE; z++) {
				glm::ivec2 block{ x + chunkID.x * CHUNK_SIZE, z + chunkID.y * CHUNK_SIZE };
				int height = this->height.fractal(14, block.x, block.y) * 26.f + 70.f;
				height += this->detail.fractal(6, block.x, block.y) * 4.f;
				for (int y = 0; y < MAX_BLOCK_HEIGHT; y++) {
					if (y == height) {
						world[chunkID][y * CHUNK_SIZE * CHUNK_SIZE + z * CHUNK_SIZE + x] = 1; //Grass
					}
					else if (y > height - 2 && y < height) {
						world[chunkID][y * CHUNK_SIZE * CHUNK_SIZE + z * CHUNK_SIZE + x] = 2; //Dirt
					}
					else if (y < height) {
						world[chunkID][y * CHUNK_SIZE * CHUNK_SIZE + z * CHUNK_SIZE + x] = 3; //Stone
					}
					else if (y < 64) {
						world[chunkID][y * CHUNK_SIZE * CHUNK_SIZE + z * CHUNK_SIZE + x] = 4; //Water
					}
					else {
						break;
					}
				}

				if (height > 66) {
					//TODO: generate structures
					//TODO: also generate structure in surrounding chunks to avoid seams across chunk borders
					//TODO: better way of generating random numbers
					srand(std::hash<glm::ivec2>{}(block));
					if (rand() / (float)RAND_MAX > 0.992) {
						for (int y = height; y < height + 5; ++y) {
							world[chunkID][y * CHUNK_SIZE * CHUNK_SIZE + z * CHUNK_SIZE + x] = 5; //Log
						}

						//Lower half of leaves
						for (int leafx = -2; leafx < 3; leafx++) {
							for (int leafz = -2; leafz < 3; leafz++) {
								for (int y = 4; y < 6; y++) {
									glm::ivec3 leafBlockPos{ leafx + x, height + y, leafz + z }; //Local to this chunk
									if (leafBlockPos.x >= 0 && leafBlockPos.x < CHUNK_SIZE && leafBlockPos.z >= 0 && leafBlockPos.z < CHUNK_SIZE) {
										if (leafx == 0 && leafz == 0 && y == 4) continue; //Leave one log piece
										world[chunkID][leafBlockPos.y * CHUNK_SIZE * CHUNK_SIZE + leafBlockPos.z * CHUNK_SIZE + leafBlockPos.x] = 6; //Leaves
									}
									else {
										//Figure out what chunk to write to
										glm::ivec2 chunkID;
										glm::ivec3 blockPos = BlockToChunk(glm::ivec3(block.x + leafx, height + y, block.y + leafz), chunkID);
										world[chunkID][blockPos.y * CHUNK_SIZE * CHUNK_SIZE + blockPos.z * CHUNK_SIZE + blockPos.x] = 6; //Leaves
									}
								}
							}
						}

						//Upper half of leaves
						for (int leafx = -1; leafx < 2; leafx++) {
							for (int leafz = -1; leafz < 2; leafz++) {
								//Lower half of leaves
								for (int y = 6; y < 8; y++) {
									glm::ivec3 leafBlockPos{ leafx + x, height + y, leafz + z }; //Local to this chunk
									if (leafBlockPos.x >= 0 && leafBlockPos.x < CHUNK_SIZE && leafBlockPos.z >= 0 && leafBlockPos.z < CHUNK_SIZE) {
										world[chunkID][leafBlockPos.y * CHUNK_SIZE * CHUNK_SIZE + leafBlockPos.z * CHUNK_SIZE + leafBlockPos.x] = 6; //Leaves
									}
									else {
										//Figure out what chunk to write to
										glm::ivec2 chunkID;
										glm::ivec3 blockPos = BlockToChunk(glm::ivec3(block.x + leafx, height + y, block.y + leafz), chunkID);
										world[chunkID][blockPos.y * CHUNK_SIZE * CHUNK_SIZE + blockPos.z * CHUNK_SIZE + blockPos.x] = 6; //Leaves
									}
								}
							}
						}
					}
				}
			}
		}
	}

	loadedChunks[chunkID] = true;
}