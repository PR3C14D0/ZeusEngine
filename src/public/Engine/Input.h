#pragma once
#include <iostream>
#include <map>
#include <vector>
#include <Windows.h>

enum InputState {
	PRESSED = 0,
	RELEASED = 1
};

enum MouseButton {
	LEFT = 0,
	RIGHT = 1
};

class Input {
private:
	Input();
	static Input* instance;
	HWND hwnd;

	std::map<char, InputState> keys;
	std::map<MouseButton, InputState> mouseBtns;
	
public:
	static Input* GetInstance();
	void SetHWND(HWND& hwnd);
	void Update(WPARAM wParam, LPARAM lParam);
	char pressedKey;

	bool GetKey(char key, InputState state);
	bool GetKeyUp(char key);
	bool GetKeyDown(char key);

	bool GetButton(MouseButton btn, InputState state);
	bool GetButtonUp(MouseButton btn);
	bool GetButtonDown(MouseButton btn);

	void SetButton(MouseButton btn, InputState state);
	void SetButtonDown(MouseButton btn);
	void SetButtonUp(MouseButton btn);

	void SetKey(char key, InputState state);
	void SetKeyDown(char key);
	void SetKeyUp(char key);

	void RemoveReleased();
};