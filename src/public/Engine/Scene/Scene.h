#pragma once
#include <iostream>
#include <vector>
#include "Engine/GameObject/GameObject.h"
#include "Engine/Camera/Camera.h"
#include "Engine/Camera/EditorCamera.h"

class Scene {
private:
	std::string name;
	std::vector<GameObject*> sceneObjs;
public:
	Camera* actualCamera;
	Scene(std::string name);

	void SetCamera(Camera* camera);
	void Render();
};