#include "Mesh.h"
#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

Mesh::Mesh(Device& device, const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices) : device(device), hasIndexBuffer(true) {
	vertexBuffer = std::make_unique<Buffer>(
		device,
		sizeof(Vertex),
		vertices.size(),
		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		Device::QueueFamilyIndices::Graphics
		);

	Buffer vertexStagingBuffer(
		device,
		sizeof(Vertex),
		vertices.size(),
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		Device::QueueFamilyIndices::Graphics
	);

	vertexStagingBuffer.Map();
	vertexStagingBuffer.WriteToBuffer((void*)vertices.data());
	vertexStagingBuffer.UnMap();

	auto commandBuffer = device.BeginSingleTimeCommands();

	VkBufferCopy copy{};
	copy.size = vertexStagingBuffer.GetBufferSize();
	vkCmdCopyBuffer(commandBuffer, vertexStagingBuffer.GetBuffer(), vertexBuffer->GetBuffer(), 1, &copy);
	//device.CopyBuffer(vertexStagingBuffer.GetBuffer(), vertexBuffer->GetBuffer(), vertexStagingBuffer.GetBufferSize());

	indexBuffer = std::make_unique<Buffer>(
		device,
		sizeof(uint32_t),
		indices.size(),
		VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		Device::QueueFamilyIndices::Graphics
		);

	Buffer indexStagingBuffer(
		device,
		sizeof(uint32_t),
		indices.size(),
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		Device::QueueFamilyIndices::Graphics
	);

	indexStagingBuffer.Map();
	indexStagingBuffer.WriteToBuffer((void*)indices.data());
	indexStagingBuffer.UnMap();

	//device.CopyBuffer(indexStagingBuffer.GetBuffer(), indexBuffer->GetBuffer(), indexStagingBuffer.GetBufferSize());

	copy.size = indexStagingBuffer.GetBufferSize();
	vkCmdCopyBuffer(commandBuffer, indexStagingBuffer.GetBuffer(), indexBuffer->GetBuffer(), 1, &copy);

	device.EndSingleTimeCommands(commandBuffer);
}

Mesh::Mesh(Device& device, const std::vector<Vertex>& vertices) : device(device), hasIndexBuffer(false) {
	vertexBuffer = std::make_unique<Buffer>(
		device,
		sizeof(Vertex),
		vertices.size(),
		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		Device::QueueFamilyIndices::Graphics
		);

	Buffer vertexStagingBuffer(
		device,
		sizeof(Vertex),
		vertices.size(),
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		Device::QueueFamilyIndices::Graphics
	);

	vertexStagingBuffer.Map();
	vertexStagingBuffer.WriteToBuffer((void*)vertices.data());
	vertexStagingBuffer.UnMap();

	device.CopyBuffer(vertexStagingBuffer.GetBuffer(), vertexBuffer->GetBuffer(), vertexStagingBuffer.GetBufferSize());
}

Mesh::~Mesh() {

}

void Mesh::Draw(VkCommandBuffer commandBuffer) const {
	VkBuffer buffers[] = { vertexBuffer->GetBuffer() };
	VkDeviceSize offsets[] = { 0 };
	vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);
	if (hasIndexBuffer) {
		vkCmdBindIndexBuffer(commandBuffer, indexBuffer->GetBuffer(), 0, VK_INDEX_TYPE_UINT32);
		vkCmdDrawIndexed(commandBuffer, indexBuffer->GetInstanceCount(), 1, 0, 0, 0);
	}
	else {
		vkCmdDraw(commandBuffer, vertexBuffer->GetInstanceCount(), 1, 0, 0);
	}
}

std::unique_ptr<Mesh> Mesh::Load(Device& device, std::string filename) {
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string warn, err;

	if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filename.c_str())) {
		throw std::runtime_error("Failed to load model " + filename + ". " + warn + '\n' + err);
	}

	std::unordered_map<Vertex, uint32_t> uniqueVertices;
	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;

	for (const auto& shape : shapes) {
		for (const auto& index : shape.mesh.indices) {
			Vertex vertex{};
			vertex.pos = {
				attrib.vertices[index.vertex_index * 3 + 0],
				attrib.vertices[index.vertex_index * 3 + 1],
				attrib.vertices[index.vertex_index * 3 + 2]
			};

			vertex.normal = {
				attrib.normals[index.normal_index * 3 + 0],
				attrib.normals[index.normal_index * 3 + 1],
				attrib.normals[index.normal_index * 3 + 2]
			};

			vertex.color = {
				attrib.colors[index.vertex_index * 3 + 0],
				attrib.colors[index.vertex_index * 3 + 1],
				attrib.colors[index.vertex_index * 3 + 2]
			};

			vertex.uv = {
				attrib.texcoords[index.texcoord_index * 2 + 0],
				attrib.texcoords[index.texcoord_index * 2 + 1]
			};


			if (!uniqueVertices.contains(vertex)) {
				uniqueVertices[vertex] = vertices.size();
				vertices.push_back(vertex);
			}

			indices.push_back(uniqueVertices[vertex]);
		}
	}

	return std::make_unique<Mesh>(device, vertices, indices);
}