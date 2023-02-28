#pragma once

#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"
#define GLFW_EXPOSE_NATIVE_WIN32
#include "GLFW/glfw3native.h"

#include "Common.h"
#include "InputSystem.h"

class Window {
public:
	Window(const std::string& name, int width, int height, class App& app);
	~Window();

	bool ShouldClose() const;

	const std::string& Name() const { return name; }
	int Width() const { return width; }
	int Height() const { return height; }

	VkSurfaceKHR GetSurface(VkInstance instance) const;
	VkExtent2D GetFramebufferSize() const;

	bool Resized() const { return resizedFlag; }
	void ResetFlags() { resizedFlag = false; }
	static void ResizedCallback(GLFWwindow* window, int width, int height);
	static void RefreshCallback(GLFWwindow* window);
	static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
	static void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
	static void MouseMoveCallback(GLFWwindow* window, double xpos, double ypos);
	static void MouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset);

	void PreUpdate(struct UpdateEvent& event);
	void PostUpdate(struct UpdateEvent& event);

	const InputSystem& GetInputSystem() const { return *inputSystem; }

private:
	friend class InputSystem;

	GLFWwindow* window;
	std::string name;
	int width, height;
	bool resizedFlag = false;
	class App& app;
	std::unique_ptr<InputSystem> inputSystem;

	static int numWindows;  // Tracks the number of active windows, used to check whether a window should call glfwTerminate()
};