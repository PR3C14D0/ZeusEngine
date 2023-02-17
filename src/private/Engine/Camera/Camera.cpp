#include "Engine/Camera/Camera.h"
#include "Engine/Core.h"

Camera::Camera(std::string name) : GameObject::GameObject(name) {
	this->core = Core::GetInstance();
	this->input = Input::GetInstance();
}

void Camera::Init() {
	this->core->GetWindowSize(this->width, this->height);
	this->transformMatrix.World = XMMatrixTranspose(XMMatrixIdentity());
	this->transformMatrix.Projection = XMMatrixTranspose(XMMatrixPerspectiveFovLH(XMConvertToRadians(90.f), (float)this->width / (float)this->height, 0.01f, 300.f));
}

void Camera::Render() {
	this->transformMatrix.View = XMMatrixTranspose(XMMatrixIdentity());
	this->transformMatrix.View *= XMMatrixTranspose(XMMatrixRotationX(XMConvertToRadians(this->transform.rotation.x)));
	this->transformMatrix.View *= XMMatrixTranspose(XMMatrixRotationY(XMConvertToRadians(this->transform.rotation.y)));
	this->transformMatrix.View *= XMMatrixTranspose(XMMatrixRotationZ(XMConvertToRadians(this->transform.rotation.z)));
	this->transformMatrix.View *= XMMatrixTranspose(XMMatrixTranslation(this->transform.location.x, this->transform.location.y, this->transform.location.z));
}

WVP Camera::GetTransform() {
	return this->transformMatrix;
}