#pragma once

#include "Common.h"

class InputSystem {
public:
	static constexpr int MAX_KEYS = 349;
	static constexpr int MAX_BUTTONS = 8;

	enum ButtonState {
		Pressed = 1,
		Released = 2,
		Held = 3,
		None = 0
	};

	InputSystem(glm::vec2 mousePos);
	~InputSystem();

	void PreUpdate(struct UpdateEvent& event);
	void PostUpdate(struct UpdateEvent& event);

	ButtonState GetKeyState(int key) const;
	bool GetKey(int key) const;

	ButtonState GetButtonState(int button) const;
	bool GetButton(int key) const;

	glm::vec2 GetMousePos() const;
	glm::vec2 GetMouseDelta() const;

	glm::vec2 GetScroll() const;
	glm::vec2 GetScrollDelta() const;

	void HandleKey(int key, int scancode, int action, int mods);
	void HandleMouseButton(int button, int action, int mods);
	void HandleMouseMove(float xpos, float ypos);
	void HandleMouseScroll(float xoffset, float yoffset);

private:
	std::bitset<MAX_KEYS> keys, oldKeys;
	glm::vec2 mousePos, oldMousePos;
	std::bitset<MAX_BUTTONS> buttons, oldButtons;
	glm::vec2 scroll, oldScroll;
};