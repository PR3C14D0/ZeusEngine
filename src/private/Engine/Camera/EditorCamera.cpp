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
		float x = input->deltaX;
		float y = input->deltaY;
		this->transform.rotate(y * 0.1f, x * 0.1f, 0);
		if (this->input->GetKeyDown('w')) {
			float speed = -0.05f;
			Vector3 translation = (this->transform.forward() * speed);
			this->transform.translate(translation);
		}
		if (this->input->GetKeyDown('s')) {
			float speed = 0.05f;
			Vector3 translation = (this->transform.forward() * speed);
			this->transform.translate(translation);
		}
		if (this->input->GetKeyDown('d')) {
			float speed = -0.05f;
			Vector3 translation = (this->transform.right() * speed);
			this->transform.translate(translation);
		}
		if (this->input->GetKeyDown('a')) {
			float speed = 0.05f;
			Vector3 translation = (this->transform.right() * speed);
			this->transform.translate(translation);
		}
	}
	else {
		this->input->ClampCursor(false);
	}
}