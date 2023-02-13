#pragma once

class SceneManager {
private:
	static SceneManager* instance;
public:
	static SceneManager* GetInstance();
};