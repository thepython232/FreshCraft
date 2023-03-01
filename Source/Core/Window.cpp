#include "Window.h"
#include "App.h"

int Window::numWindows = 0;

Window::Window(const std::string& name, int width, int height, App& app)
	: name(name), width(width), height(height), app(app) {
	numWindows++;
	if (numWindows == 1)
		glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

	window = glfwCreateWindow(width, height, name.c_str(), NULL, NULL);
	glfwSetWindowUserPointer(window, (void*)this);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetFramebufferSizeCallback(window, &Window::ResizedCallback);
	glfwSetWindowRefreshCallback(window, &Window::RefreshCallback);
	glfwSetKeyCallback(window, &Window::KeyCallback);
	glfwSetMouseButtonCallback(window, &Window::MouseButtonCallback);
	glfwSetCursorPosCallback(window, &Window::MouseMoveCallback);
	glfwSetScrollCallback(window, &Window::MouseScrollCallback);

	if (!window) {
		const char* description;
		glfwGetError(&description);
		std::cerr << "Failed to create window " << name << "! " << description << std::endl;
	}

	double xpos, ypos;
	glfwGetCursorPos(window, &xpos, &ypos);
	inputSystem = std::make_unique<InputSystem>(glm::vec2{ static_cast<float>(xpos), static_cast<float>(ypos) });
}

Window::~Window() {
	glfwDestroyWindow(window);
	numWindows--;
	if (numWindows == 0) {
		glfwTerminate();
	}
}

bool Window::ShouldClose() const {
	return glfwWindowShouldClose(window) || glfwGetKey(window, GLFW_KEY_ESCAPE);
}

void Window::PreUpdate(UpdateEvent& event) {
	inputSystem->PreUpdate(event);
}

void Window::PostUpdate(UpdateEvent& event) {
	inputSystem->PostUpdate(event);
}

VkSurfaceKHR Window::GetSurface(VkInstance instance) const {
	VkSurfaceKHR surface;
	VkWin32SurfaceCreateInfoKHR createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	createInfo.hwnd = glfwGetWin32Window(window);
	createInfo.hinstance = GetModuleHandle(nullptr);
	if (vkCreateWin32SurfaceKHR(instance, &createInfo, nullptr, &surface) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create window surface!");
	}

	return surface;
}

VkExtent2D Window::GetFramebufferSize() const {
	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	return VkExtent2D{ static_cast<uint32_t>(width), static_cast<uint32_t>(height) };
}

void Window::ResizedCallback(GLFWwindow* window, int width, int height) {
	Window* ptr = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
	ptr->width = width;
	ptr->height = height;
	ptr->resizedFlag = true;
}

void Window::RefreshCallback(GLFWwindow* window) {
	Window* ptr = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
	if (ptr->app.IsInitialized())
		ptr->app.ReDraw();
}

void Window::KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	Window* ptr = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
	if (ptr->app.IsInitialized() && action != GLFW_REPEAT && key != GLFW_KEY_UNKNOWN) {
		ptr->inputSystem->HandleKey(key, scancode, action, mods);
	}
}

void Window::MouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
	Window* ptr = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
	if (ptr->app.IsInitialized() && action != GLFW_REPEAT) {
		ptr->inputSystem->HandleMouseButton(button, action, mods);
	}
}

void Window::MouseMoveCallback(GLFWwindow* window, double xpos, double ypos) {
	Window* ptr = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
	if (ptr->app.IsInitialized()) {
		ptr->inputSystem->HandleMouseMove(static_cast<float>(xpos), static_cast<float>(ypos));
	}
}

void Window::MouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
	Window* ptr = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
	if (ptr->app.IsInitialized()) {
		ptr->inputSystem->HandleMouseScroll(static_cast<float>(xoffset), static_cast<float>(yoffset));
	}
}