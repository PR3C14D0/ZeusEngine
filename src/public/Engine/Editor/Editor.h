#pragma once
#include "imgui/imgui.h"

class Editor {
private:
	static Editor* instance;
	void RenderMenu();
public:
	Editor();
	void Render();
	static Editor* GetInstance();
};