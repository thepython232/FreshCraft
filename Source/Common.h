#pragma once

#define VK_USE_PLATFORM_WIN32_KHR
#include "vulkan/vulkan.h"
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_RADIANS
#include "glm/glm.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/hash.hpp"
#include "glm/gtx/string_cast.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/matrix_inverse.hpp"

#include <iostream>
#include <vector>
#include <array>
#include <list>
#include <concepts>
#include <limits>
#include <cmath>
#include <cstdlib>
#include <cstdint>
#include <chrono>
#include <unordered_map>
#include <set>
#include <map>
#include <queue>
#include <algorithm>
#include <memory>
#include <bitset>
#include <sstream>
#include <string>
#include <fstream>
#include <exception>
#include <stdexcept>
#include <functional>
#undef max
#undef min
#undef near
#undef far

#ifndef NDEBUG
#define Assert(_c) if (!(_c)) std::cerr << "Assert Failed: " << __FILE__ << ": (" << __LINE__ << ") in " << __FUNCTION__ << ": " << #_c << "!" << std::endl
#else
#define Assert(_c) ((void)0)
#endif

template<glm::length_t L, typename T, glm::qualifier Q>
inline std::ostream& operator<<(std::ostream& os, const glm::vec<L, T, Q>& vec) {
	os << glm::to_string(vec);
	return os;
}

template<glm::length_t C, glm::length_t R, typename T, glm::qualifier Q>
inline std::ostream& operator<<(std::ostream& os, const glm::mat<C, R, T, Q>& mat) {
	os << glm::to_string(mat);
	return os;
}

inline bool IsDepthFormat(VkFormat format) {
	return format == VK_FORMAT_D16_UNORM
		|| format == VK_FORMAT_D16_UNORM_S8_UINT
		|| format == VK_FORMAT_D24_UNORM_S8_UINT
		|| format == VK_FORMAT_D32_SFLOAT
		|| format == VK_FORMAT_D32_SFLOAT_S8_UINT;
}

inline bool IsStencilFormat(VkFormat format) {
	return format == VK_FORMAT_S8_UINT
		|| format == VK_FORMAT_D16_UNORM_S8_UINT
		|| format == VK_FORMAT_D24_UNORM_S8_UINT
		|| format == VK_FORMAT_D32_SFLOAT_S8_UINT;
}

//TODO: actually check all color formats
inline bool IsColorFormat(VkFormat format) {
	return !(IsDepthFormat(format) || IsStencilFormat(format));
}

//Hash function for std::pair
namespace std {
	template<typename T, typename U>
	struct hash<std::pair<T, U>> {
		size_t operator()(std::pair<T, U> const& p) const {
			return hash<T>{}(p.first) ^ hash<U>{}(p.second);
		}
	};
}