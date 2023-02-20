#include "Math/Transform.h"

Transform::Transform() {
	this->location = Vector3(0.f, 0.f, 0.f);
	this->rotation = Vector3(0.f, 0.f, 0.f);
	this->scale = Vector3(0.f, 0.f, 0.f);
}

void Transform::translate(Vector3 offset) {
	this->location = this->location + offset;
	return;
}

void Transform::translate(float x, float y, float z) {
	Vector3 vec{ x, y, z };
	this->translate(vec);
	return;
}

void Transform::rotate(Vector3 rot) {
	this->rotation = this->rotation + rot;
	return;
}

void Transform::rotate(float x, float y, float z) {
	Vector3 vec{ x, y, z };
	this->rotate(vec);
	return;
}

Vector3 Transform::forward() {
	return this->rotatePoint({ 0.f, 0.f, 1.f });
}

Vector3 Transform::right() {
	return this->rotatePoint({ 1.f, 0.f, 0.f });
}

Vector3 Transform::rotatePoint(Vector3 v) {
	XMMATRIX rot = XMMatrixTranspose(XMMatrixIdentity());
	rot *= XMMatrixTranspose(XMMatrixRotationX(XMConvertToRadians(this->rotation.x)));
	rot *= XMMatrixTranspose(XMMatrixRotationY(XMConvertToRadians(this->rotation.y)));
	rot *= XMMatrixTranspose(XMMatrixRotationZ(XMConvertToRadians(this->rotation.z)));

	XMVECTOR vec = XMVector4Transform(XMVectorSet(v.x, v.y, v.z, 1.f), rot);
	XMFLOAT4 rotated;
	XMStoreFloat4(&rotated, vec);
	return Vector3(rotated.x, rotated.y, rotated.z);
}