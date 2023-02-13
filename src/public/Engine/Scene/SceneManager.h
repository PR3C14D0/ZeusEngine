#pragma once
#include <iostream>
#include <vector>
#include "Engine/Scene/Scene.h"

class SceneManager {
private:
	static SceneManager* instance;
	std::vector<Scene*> scenes;
	Scene* actualScene;
public:
	SceneManager();
	static SceneManager* GetInstance();
	void Render();
};