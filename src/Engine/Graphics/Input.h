#ifndef ENGINE_GRAPHICS_INPUT_H
#define ENGINE_GRAPHICS_INPUT_H

#include "GraphicsConfig.h"

inline bool* _focus_pointer = nullptr;
inline int* _key_status = nullptr;
inline bool _mouse_left_pressed = false;
inline bool _mouse_right_pressed = false;
inline bool _mouse_captured = false;
inline bool _aspect_ratio_updated = true;
inline int _cwidth = DEFAULT_WIDTH;
inline int _cheight = DEFAULT_HEIGHT;
inline void* _ptr_active_camera = nullptr;

inline void _framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
	_aspect_ratio_updated = true;
	_cwidth = width;
	_cheight = height;
}

inline void _window_focus_callback(GLFWwindow* window, int focused)
{
	if (_focus_pointer)
		*_focus_pointer = focused;
}

inline void _keyboard_key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (_key_status && key >= 0 && key < 512) {
		_key_status[key] = action;
	}
}

inline void _mouse_key_callback(GLFWwindow* window, int button, int action, int modes) {
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
		_mouse_left_pressed = true;
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
		_mouse_left_pressed = false;
	if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
		_mouse_right_pressed = true;
	if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE)
		_mouse_right_pressed = false;
}

inline bool isKeyPressedOrHeld(int key) {
	if (_key_status && key >= 0 && key < 512) {
		return _key_status[key] == GLFW_PRESS || _key_status[key] == GLFW_REPEAT;
	}
	return false;
}

inline char getUsernameKeyAndClear(bool space = false) {
	if (!_key_status)
		return 0;
	for (int i = GLFW_KEY_A; i <= GLFW_KEY_Z; i++) {
		if (_key_status[i] == GLFW_PRESS) {
			_key_status[i] = 0;
			if (_key_status[GLFW_KEY_LEFT_SHIFT] || _key_status[GLFW_KEY_RIGHT_SHIFT])
				return (char)i;
			else
				return (char)i + 32;
		}
	}
	for (int i = GLFW_KEY_0; i <= GLFW_KEY_9; i++) {
		if (_key_status[i] == GLFW_PRESS) {
			_key_status[i] = 0;
			return (char)i;
		}
	}
	if (_key_status[GLFW_KEY_BACKSPACE]) {
		_key_status[GLFW_KEY_BACKSPACE] = 0;
		return 127;
	}
	if (_key_status[GLFW_KEY_SPACE] && space) {
		_key_status[GLFW_KEY_SPACE] = 0;
		return ' ';
	}
	return 0;
}

inline void cleanKeys() {
	if (!_key_status)
		return;
	for (int i = 0; i < 512; i++)
		_key_status[i] = 0;
}

inline bool isMouseLeftClickPressed() {
	return _mouse_left_pressed;
}

inline bool isMouseRightClickPressed() {
	return _mouse_right_pressed;
}

inline void clearMouseLeftClickPressed() {
	_mouse_left_pressed = false;
}

inline void clearMouseRightClickPressed() {
	_mouse_right_pressed = false;
}

void inGameCursorPositionCallback(GLFWwindow* window, double xpos, double ypos);

#endif
