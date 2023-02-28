#include "Instance.h"
#include "GLFW/glfw3.h"

Instance::Instance() {
	CreateInstance();
	SetupDebugMessenger();
}

Instance::~Instance() {
	auto destroyFunc = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
	if (destroyFunc)
		destroyFunc(instance, messenger, nullptr);
	vkDestroyInstance(instance, nullptr);
}

bool Instance::CheckValidationLayers() {
	uint32_t count;
	vkEnumerateInstanceLayerProperties(&count, nullptr);
	std::vector<VkLayerProperties> layers(count);
	vkEnumerateInstanceLayerProperties(&count, layers.data());

	for (const char* layer : validationLayers) {
		bool layerFound = false;
		for (const auto& property : layers) {
			if (!strcmp(layer, property.layerName)) {
				layerFound = true;
				break;
			}
		}

		if (!layerFound)
			return false;
	}

	return true;
}

void Instance::CreateInstance() {
	VkApplicationInfo appInfo{};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.apiVersion = VK_MAKE_API_VERSION(0, 1, 0, 0);
	appInfo.applicationVersion = VK_MAKE_API_VERSION(0, 1, 0, 0);
	appInfo.engineVersion = VK_MAKE_API_VERSION(0, 1, 0, 0);
	appInfo.pApplicationName = "FreshCraft";
	appInfo.pEngineName = "No Engine!";

	VkInstanceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;
	
	//Check layer support
	if (enableValidationLayers && CheckValidationLayers()) {
		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();
	}
	else {
		createInfo.enabledLayerCount = 0;
	}

	//Check extension support/Poll extensions
	uint32_t count;
	const char** glfwExtensions;
	glfwExtensions = glfwGetRequiredInstanceExtensions(&count);
	std::vector<const char*> extensions(glfwExtensions, glfwExtensions + count);
	extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

	createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
	createInfo.ppEnabledExtensionNames = extensions.data();

	auto debugInfo = PopulateDebugMessengerInfo();
	createInfo.pNext = (void*)&debugInfo;

	if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create instance!");
	}
}

void Instance::SetupDebugMessenger() {
	//Load the function
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
	VkDebugUtilsMessengerCreateInfoEXT createInfo = PopulateDebugMessengerInfo();
	if (!func)
		throw std::runtime_error("Failed to load vkCreateDebugUtilsMessengerEXT!");
	if (func(instance, &createInfo, nullptr, &messenger) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create debug messenger!");
	}
}

VkDebugUtilsMessengerCreateInfoEXT Instance::PopulateDebugMessengerInfo() {
	VkDebugUtilsMessengerCreateInfoEXT createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT
		| VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
	createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
		| VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT
		| VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
	createInfo.pfnUserCallback = &Instance::MessageCallback;
	createInfo.pUserData = (void*)this;

	return createInfo;
}

VKAPI_ATTR VkBool32 VKAPI_CALL Instance::MessageCallback(
	VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageTypes,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void* pUserData
) {
	std::cerr << "Validation Message: " << pCallbackData->pMessage << std::endl;
	return VK_FALSE;
}