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
	this->translate(Vector3{ x, y, z });
	return;
}

void Transform::rotate(Vector3 rot) {
	this->rotation = this->rotation + rot;
	return;
}

void Transform::rotate(float x, float y, float z) {
	this->rotate(Vector3{ x, y, z });
	return;
}