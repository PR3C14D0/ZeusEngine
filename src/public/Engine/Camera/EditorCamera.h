#pragma once
#include "Engine/Camera/Camera.h"

class EditorCamera : public Camera {
public:
	EditorCamera(std::string name);
	void Init() override;
	void Render() override;
};