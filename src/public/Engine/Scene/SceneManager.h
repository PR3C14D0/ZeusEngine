#pragma once
#include <iostream>
#include <vector>

class SceneManager {
private:
	static SceneManager* instance;
public:
	static SceneManager* GetInstance();
	void Render();
};