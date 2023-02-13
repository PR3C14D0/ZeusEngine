#include "Engine/Scene/Scene.h"

Scene::Scene(std::string name) {
	this->name = name;
	this->sceneObjs.push_back(new GameObject("SampleObj"));
}