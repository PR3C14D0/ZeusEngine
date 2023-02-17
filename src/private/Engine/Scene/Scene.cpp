#include "Engine/Scene/Scene.h"

Scene::Scene(std::string name) {
	this->name = name;
	GameObject* sampleObj = new GameObject("SampleObj");
	this->sceneObjs.push_back(sampleObj);
	sampleObj->LoadModel("f16.fbx");
	sampleObj->Init();
	Camera* cam = new EditorCamera("Camera");
	this->SetCamera(cam);
	this->actualCamera->Init();
	this->actualCamera->transform.translate(0.f, 0.f, 2.f);
	//this->actualCamera->transform.rotate(-90.f, 0.f, 0.f);
}

void Scene::SetCamera(Camera* camera) {
	this->sceneObjs.push_back(camera);
	this->actualCamera = camera;
	return;
}

void Scene::Render() {
	for (GameObject* obj : this->sceneObjs) {
		obj->Render();
	}
}