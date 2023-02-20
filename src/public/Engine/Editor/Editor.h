#pragma once
#include <iostream>
#include <imgui/imgui.h>
#include <imguizmo/ImGuizmo.h>
#include "Module/Time.h"
#include <string>

class Editor {
private:
	static Editor* instance;
	void RenderMenu();
	void PerformanceMenu();
	Time* time;
public:
	Editor();
	void Render();
	static Editor* GetInstance();
};