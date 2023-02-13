#pragma once
#include "Math/Vector3.h"

struct Transform {
	Vector3 location;
	Vector3 rotation;
	Vector3 scale;

	void translate(Vector3 offset);
	void translate(float x, float y, float z);

	void rotate(Vector3 rot);
	void rotate(float x, float y, float z);

	Transform();
};