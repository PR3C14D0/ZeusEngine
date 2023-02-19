#include "Engine/Camera/EditorCamera.h"

EditorCamera::EditorCamera(std::string name) : Camera::Camera(name) {
	
}

void EditorCamera::Init() {
	Camera::Init();
}

void EditorCamera::Render() {
	Camera::Render();
	if (this->input->GetButtonDown(RIGHT)) {
		this->input->ClampCursor(true);
		if (this->input->GetKeyDown('w')) {
			this->transform.translate(0.f, 0.f, .005f);
		}
	}
	else {
		this->input->ClampCursor(false);
	}
}