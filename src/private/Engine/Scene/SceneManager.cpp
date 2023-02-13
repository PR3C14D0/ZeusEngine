#include "Engine/Scene/SceneManager.h"

SceneManager* SceneManager::instance;

SceneManager::SceneManager() {
	// This is hard coded. I'll delete it soon, is only for testing.
	Scene* sampleScene = new Scene("Sample scene");
	this->scenes.push_back(actualScene);
	this->actualScene = sampleScene;
}

SceneManager* SceneManager::GetInstance() {
	if (SceneManager::instance == nullptr)
		SceneManager::instance = new SceneManager();
	return SceneManager::instance;
}

void SceneManager::Render() {
	this->actualScene->Render();
}