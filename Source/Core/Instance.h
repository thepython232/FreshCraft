#pragma once

#include "Common.h"

#ifndef NDEBUG
const bool enableValidationLayers = true;
#else
const bool enableValidationLayers = false;
#endif

const std::vector<const char*> validationLayers = {
	"VK_LAYER_KHRONOS_validation"
};

class Instance {
public:
	Instance();
	~Instance();

	VkInstance GetInstance() const { return instance; }

private:
	bool CheckValidationLayers();
	void CreateInstance();
	void SetupDebugMessenger();
	VkDebugUtilsMessengerCreateInfoEXT PopulateDebugMessengerInfo();

	VkInstance instance;
	VkDebugUtilsMessengerEXT messenger;

	static VKAPI_ATTR VkBool32 VKAPI_CALL MessageCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageTypes,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData
	);
};