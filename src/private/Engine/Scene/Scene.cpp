#include "Engine/Scene/Scene.h"

Scene::Scene(std::string name) {
	this->name = name;
	GameObject* sampleObj = new GameObject("SampleObj");
	this->sceneObjs.push_back(sampleObj);
}

void Scene::Render() {
	for (GameObject* obj : this->sceneObjs) {
		obj->Render();
	}
}