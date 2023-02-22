#pragma once
#include <iostream>
#include <imgui/imgui.h>
#include <imguizmo/ImGuizmo.h>
#include <directx/DirectXMath.h>
#include "Module/Time.h"
#include <string>
#include "Engine/Scene/SceneManager.h"

class Core;
enum VSYNC;

class Editor {
private:
	static Editor* instance;
	void RenderMenu();
	void PerformanceMenu();
	void SettingsMenu();
	VSYNC vsyncState;
	Time* time;
	Core* core;
	SceneManager* sceneMgr;
public:
	Editor();
	void Render();
	static Editor* GetInstance();
};