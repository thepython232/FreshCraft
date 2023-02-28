#pragma once

#include "Buffer.h"
#include "Vertex.h"

class Mesh {
public:
	Mesh(Device& device, const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);
	Mesh(Device& device, const std::vector<Vertex>& vertices);
	~Mesh();

	void Draw(VkCommandBuffer commandBuffer) const;

	static std::unique_ptr<Mesh> Load(Device& device, std::string filename);

private:
	std::unique_ptr<Buffer> vertexBuffer;
	std::unique_ptr<Buffer> indexBuffer;
	bool hasIndexBuffer;
	Device& device;
};