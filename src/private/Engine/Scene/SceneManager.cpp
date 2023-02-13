#include "Engine/Scene/SceneManager.h"

SceneManager* SceneManager::instance;

SceneManager* SceneManager::GetInstance() {
	if (!SceneManager::instance)
		SceneManager::instance = new SceneManager();
	return SceneManager::instance;
}