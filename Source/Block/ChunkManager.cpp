#include "ChunkManager.h"
#include "Util\Raytrace.h"

ChunkManager::ChunkManager(Device& device) : device(device) {
	
}

void ChunkManager::Update(const UpdateEvent& event) {
	for (int x = -RENDER_DISTANCE; x < RENDER_DISTANCE; ++x) {
		for (int z = -RENDER_DISTANCE; z < RENDER_DISTANCE; ++z) {
			glm::ivec2 chunkPos{ x, z };
			chunkPos += glm::vec2(event.mainCamera.GetPos().x, event.mainCamera.GetPos().z) / float(CHUNK_SIZE);
			if (!chunks.contains(chunkPos)) {
				chunks[chunkPos] = std::make_unique<ChunkMesh>(device, chunkPos, *this);
				chunksToUpdate.push_back(chunkPos);
			}
		}
	}

	for (const auto& kv : chunks) {
		if (std::find(chunksToUpdate.begin(), chunksToUpdate.end(), kv.first) == chunksToUpdate.end()  && kv.second->ShouldUpdate()) {
			chunksToUpdate.push_back(kv.first);
		}
	}

	std::sort(chunksToUpdate.begin(), chunksToUpdate.end(), [&event](const glm::ivec2& a, const glm::ivec2& b) {
		return glm::length(glm::vec2(a) - glm::vec2(event.mainCamera.GetPos().x, event.mainCamera.GetPos().z) / (float)CHUNK_SIZE)
			< glm::length(glm::vec2(b) - glm::vec2(event.mainCamera.GetPos().x, event.mainCamera.GetPos().z) / (float)CHUNK_SIZE);
		});

	//Update closest chunks
	if (!chunksToUpdate.empty()) {
		ChunkMesh& mesh = *chunks[*chunksToUpdate.begin()];
		chunksToUpdate.erase(chunksToUpdate.begin());

		for (int i = 0; i < 9; i++) {
			GenerateChunk(glm::ivec2(i % 3 - 1, i / 3 - 1) + mesh.pos);
		}

		mesh.Update(event);
	}

	glm::ivec2 chunkID;
	BlockToChunk(glm::ivec3(event.mainCamera.GetPos() - 8.f), chunkID);

	if (chunks.contains(glm::ivec2(0, 0)) && chunks[glm::ivec2(0, 0)]->Loaded())
		chunks[glm::ivec2(0, 0)]->Resort(event);
}

//TODO: broken
bool ChunkManager::VoxelRaytrace(glm::vec3 pos, const glm::vec3& dir, float tMax, BlockHitInfo& info) const {
	pos += glm::vec3{ CHUNK_SIZE / 2, 0, CHUNK_SIZE / 2 };
	glm::vec3 tmpMin = pos;
	glm::vec3 tmpMax = pos + dir * tMax;

	glm::ivec3 min = glm::ivec3(glm::min(tmpMin.x, tmpMax.x), glm::min(tmpMin.y, tmpMax.y), glm::min(tmpMin.z, tmpMax.z));
	min -= glm::ivec3(1);
	glm::ivec3 max = glm::ivec3(glm::max(tmpMin.x, tmpMax.x), glm::max(tmpMin.y, tmpMax.y), glm::max(tmpMin.z, tmpMax.z));
	max += glm::ivec3(1);

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

					RectHit hit;
					if (RectRaytrace(pos, dir, minBounds, maxBounds, hit)) {
						if (hit.t < minDist && hit.t > 0.f) {
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
	if (!world.contains(chunkID) || loadedChunks[chunkID] == false) {
		for (int x = 0; x < CHUNK_SIZE; x++) {
			for (int z = 0; z < CHUNK_SIZE; z++) {
				glm::ivec2 block{ x + chunkID.x * CHUNK_SIZE, z + chunkID.y * CHUNK_SIZE };
				int noise1 = this->detail.fractal(8, block.x, block.y);
				int noise2 = this->detail.fractal(8, block.x, block.y);
				int height = this->height.fractal(14, block.x + 80.f * noise1, block.y + 80.f * noise2) * 26.f + 70.f;
				
				int sandNoise = sand.noise(block.x, block.y) * 2.f;
				//height += this->detail.fractal(6, block.x, block.y) * 4.f;
				for (int y = 0; y < MAX_BLOCK_HEIGHT; y++) {
					if (y == height) {
						if (y > 66 + sandNoise) {
							world[chunkID][y * CHUNK_SIZE * CHUNK_SIZE + z * CHUNK_SIZE + x] = 1; //Grass
						}
						else {
							world[chunkID][y * CHUNK_SIZE * CHUNK_SIZE + z * CHUNK_SIZE + x] = 7; //Sand
						}
					}
					else if (y > height - 2 && y < height) {
						if (y > 66 + sandNoise) {
							world[chunkID][y * CHUNK_SIZE * CHUNK_SIZE + z * CHUNK_SIZE + x] = 2; //Dirt
						}
						else {
							world[chunkID][y * CHUNK_SIZE * CHUNK_SIZE + z * CHUNK_SIZE + x] = 7; //Sand
						}
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
					//TODO: better way of generating random numbers
					//TODO: way of not having to write into nearby chunks that may have already been meshed
					//because structures exceeding CHUNK_SIZE will still break
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
										if ((leafx == -2 || leafx == 2) && (leafz == -2 || leafz == 2) && rand() / (float)RAND_MAX > 0.7) continue;
										world[chunkID][leafBlockPos.y * CHUNK_SIZE * CHUNK_SIZE + leafBlockPos.z * CHUNK_SIZE + leafBlockPos.x] = 6; //Leaves
									}
									else {
										if ((leafx == -2 || leafx == 2) && (leafz == -2 || leafz == 2) && rand() / (float)RAND_MAX > 0.7) continue;
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
										if (y == 7 && (leafx == -1 || leafx == 1) && (leafz == -1 || leafz == 1) && rand() / (float)RAND_MAX > 0.75) continue;
										world[chunkID][leafBlockPos.y * CHUNK_SIZE * CHUNK_SIZE + leafBlockPos.z * CHUNK_SIZE + leafBlockPos.x] = 6; //Leaves
									}
									else {
										if (y == 7 && (leafx == -1 || leafx == 1) && (leafz == -1 || leafz == 1) && rand() / (float)RAND_MAX > 0.75) continue;
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

uint32_t ChunkManager::MaxBlockHeight(const glm::ivec2& chunk) const {
	if (!world.contains(chunk)) return 0;

	int max = 0;
	for (int y = 0; y < MAX_BLOCK_HEIGHT; y++) {
		for (int i = 0; i < CHUNK_SIZE * CHUNK_SIZE; i++) {
			if (world.find(chunk)->second[y * CHUNK_SIZE * CHUNK_SIZE + i] > 0) {
				max = y;
				break;
			}
		}
	}

	return max + 1;
}