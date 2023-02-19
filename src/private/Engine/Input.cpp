#include "Engine/Input.h"

Input* Input::instance;

Input::Input() { 
	this->hwnd = NULL;
	this->bCursorClamped = false;
	this->cursor = LoadCursor(NULL, IDC_ARROW);
	this->deltaX = 0.f;
	this->deltaY = 0.f;
}

void Input::SetHWND(HWND& hwnd) {
	this->hwnd = hwnd;
}

void Input::Update(WPARAM wParam, LPARAM lParam) {
	if (this->bCursorClamped) {
		SetCursorPos(this->centerX, this->centerY);
	}

	BYTE keyBoardState[256];
	bool bKeyboardState = GetKeyboardState(keyBoardState);
	if (!bKeyboardState)
		return;

	ToAscii(wParam, lParam, keyBoardState, (LPWORD)&this->pressedKey, NULL);
}

void Input::ClampCursor(bool bEnabled) {
	this->bCursorClamped = bEnabled;
	if (this->bCursorClamped) {
		RECT rect;
		GetClientRect(this->hwnd, &rect);

		POINT lt;
		lt.x = rect.left;
		lt.y = rect.top;

		POINT rb;
		rb.x = rect.right;
		rb.y = rect.bottom;

		MapWindowPoints(this->hwnd, nullptr, &lt, 1);
		MapWindowPoints(this->hwnd, nullptr, &rb, 1);

		rect.left = lt.x;
		rect.right = rb.x;
		rect.top = lt.y;
		rect.bottom = rb.y;

		int width = rect.right - rect.left;
		int height = rect.bottom - rect.top;

		this->centerX = width / 2;
		this->centerY = height / 2;

		ClipCursor(&rect);
		SetCursor(NULL);
		POINT cursorPos;
		GetCursorPos(&cursorPos);
		this->deltaX = this->centerX - cursorPos.x;
		this->deltaY = this->centerY - cursorPos.y;
	}
	else {
		ClipCursor(NULL);
		SetCursor(this->cursor);
	}
}

void Input::SetKey(char key, InputState state) {
	if (this->keys.count(key) > 0) {
		if (this->keys[key] == state)
			return;
	}

	this->keys[key] = state;
	return;
}

bool Input::GetKey(char key, InputState state) {
	bool found = false;
	if (this->keys.count(key) > 0) {
		if (this->keys[key] == state)
			found = true;
	}

	return found;
}
void Input::SetKeyDown(char key) {
	this->SetKey(key, PRESSED);
	return;
}

void Input::SetKeyUp(char key) {
	this->SetKey(key, RELEASED);
	return;
}

bool Input::GetKeyDown(char key) {
	return this->GetKey(key, PRESSED);
}

bool Input::GetButton(MouseButton btn, InputState state)
{
	bool found = false;
	if (this->mouseBtns.count(btn) > 0) {
		if (this->mouseBtns[btn] == state)
			found = true;
	}

	return found;
}

bool Input::GetButtonUp(MouseButton btn)
{
	return this->GetButton(btn, RELEASED);
}

bool Input::GetButtonDown(MouseButton btn)
{
	return this->GetButton(btn, PRESSED);
}

bool Input::GetKeyUp(char key) {
	return this->GetKey(key, RELEASED);
}

void Input::RemoveReleased() {
	if (this->keys.size() > 0) {
		std::vector<char> toRemove;
		for (std::pair<char, InputState> key : this->keys) {
			if (key.second == RELEASED)
				toRemove.push_back(key.first);
		}

		for (char key : toRemove)
			this->keys.erase(key);
	}

	if (this->mouseBtns.size() > 0) {
		std::vector<MouseButton> toRemove;
		for (std::pair<MouseButton, InputState> btn : this->mouseBtns) {
			if (btn.second == RELEASED)
				toRemove.push_back(btn.first);
		}

		for (MouseButton btn : toRemove)
			this->mouseBtns.erase(btn);
	}

	this->deltaX = 0.f;
	this->deltaY = 0.f;

	return;
}

void Input::SetButton(MouseButton btn, InputState state) {
	if (this->mouseBtns.count(btn) > 0) {
		if (this->mouseBtns[btn] == state)
			return;
	}

	this->mouseBtns[btn] = state;
	return;
}

void Input::SetButtonDown(MouseButton btn)
{
	this->SetButton(btn, PRESSED);
	return;
}

void Input::SetButtonUp(MouseButton btn)
{
	this->SetButton(btn, RELEASED);
	return;
}

Input* Input::GetInstance() {
	if (Input::instance == nullptr)
		Input::instance = new Input();
	return Input::instance;
}