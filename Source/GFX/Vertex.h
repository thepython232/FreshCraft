#pragma once

#include "Common.h"

struct Vertex {
	glm::vec3 pos;
	glm::vec3 normal;
	glm::vec3 color;
	glm::vec2 uv;

	bool operator==(const Vertex& other) const {
		return pos == other.pos && normal == other.normal && color == other.color && uv == other.uv;
	}

	static VkVertexInputBindingDescription GetBinding() {
		VkVertexInputBindingDescription binding{};
		binding.binding = 0;
		binding.stride = sizeof(Vertex);
		binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		return binding;
	}

	static std::vector<VkVertexInputAttributeDescription> GetAttributes() {
		std::vector<VkVertexInputAttributeDescription> attributes(4);
		attributes[0].binding = 0;
		attributes[0].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributes[0].location = 0;
		attributes[0].offset = offsetof(Vertex, pos);

		attributes[1].binding = 0;
		attributes[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributes[1].location = 1;
		attributes[1].offset = offsetof(Vertex, normal);

		attributes[2].binding = 0;
		attributes[2].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributes[2].location = 2;
		attributes[2].offset = offsetof(Vertex, color);

		attributes[3].binding = 0;
		attributes[3].format = VK_FORMAT_R32G32_SFLOAT;
		attributes[3].location = 3;
		attributes[3].offset = offsetof(Vertex, uv);

		return attributes;
	}
};

template<typename T, typename... Rest>
void HashCombine(std::size_t& seed, const T& val, Rest... rest) {
	seed ^= std::hash<T>{}(val) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
	(HashCombine(seed, rest), ...);
}

namespace std {
	template<>
	struct hash<Vertex> {
		size_t operator()(Vertex const& vertex) const {
			size_t seed = 0;
			HashCombine(seed, vertex.pos, vertex.normal, vertex.color, vertex.uv);
			return seed;
		}
	};
}