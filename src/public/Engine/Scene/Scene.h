#pragma once
#include <iostream>
#include <vector>
#include "Engine/GameObject/GameObject.h"

class Scene {
private:
	std::string name;
	std::vector<GameObject*> sceneObjs;
public:
	Scene(std::string name);

	void Render();
};