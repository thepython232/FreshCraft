#include "InputSystem.h"
#include "Core\Window.h"
#include "Core\Events.h"

InputSystem::InputSystem(glm::vec2 mousePos) : mousePos(mousePos) {
	keys.reset();
	oldKeys.reset();
	buttons.reset();
	oldButtons.reset();
	oldScroll = glm::vec2(0.f);
	scroll = glm::vec2(0.f);
}

InputSystem::~InputSystem() {

}

void InputSystem::PreUpdate(UpdateEvent& event) {
	
}

void InputSystem::PostUpdate(UpdateEvent& event) {
	oldKeys = keys;
	oldButtons = buttons;
	oldMousePos = mousePos;
	oldScroll = scroll;
}

InputSystem::ButtonState InputSystem::GetKeyState(int key) const {
	Assert(key < MAX_KEYS);

	if (keys[key]) {
		if (oldKeys[key]) {
			return ButtonState::Held;
		}
		else {
			return ButtonState::Pressed;
		}
	}
	else {
		if (oldKeys[key]) {
			return ButtonState::Released;
		}
		else {
			return ButtonState::None;
		}
	}
}

bool InputSystem::GetKey(int key) const {
	Assert(key < MAX_KEYS);
	return keys[key];
}

InputSystem::ButtonState InputSystem::GetButtonState(int button) const {
	Assert(button < MAX_BUTTONS);

	if (buttons[button]) {
		if (oldButtons[button]) {
			return ButtonState::Held;
		}
		else {
			return ButtonState::Pressed;
		}
	}
	else {
		if (oldButtons[button]) {
			return ButtonState::Released;
		}
		else {
			return ButtonState::None;
		}
	}
}

bool InputSystem::GetButton(int button) const {
	Assert(button < MAX_BUTTONS);

	return buttons[button];
}

glm::vec2 InputSystem::GetMousePos() const {
	return mousePos;
}
glm::vec2 InputSystem::GetMouseDelta() const {
	return mousePos - oldMousePos;
}

glm::vec2 InputSystem::GetScroll() const {
	return scroll;
}

glm::vec2 InputSystem::GetScrollDelta() const {
	return scroll - oldScroll;
}

void InputSystem::HandleKey(int key, int scancode, int action, int mods) {
	if (action == GLFW_PRESS) {
		keys[key] = true;
	}
	else {
		keys[key] = false;
	}
}

void InputSystem::HandleMouseButton(int button, int action, int mods) {
	if (action == GLFW_PRESS) {
		buttons[button] = true;
	}
	else {
		buttons[button] = false;
	}
}

void InputSystem::HandleMouseMove(float xpos, float ypos) {
	mousePos = glm::vec2{ xpos, ypos };
}

void InputSystem::HandleMouseScroll(float xoffset, float yoffset) {
	glm::vec2 delta{ xoffset, yoffset };
	scroll += delta;
}