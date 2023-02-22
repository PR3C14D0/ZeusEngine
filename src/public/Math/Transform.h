#pragma once
#include "Math/Vector3.h"
#include <directx/DirectXMath.h>

using namespace DirectX;

struct Transform {
	Vector3 location;
	Vector3 rotation;
	Vector3 scale;

	void translate(Vector3 offset);
	void translate(float x, float y, float z);

	void rotate(Vector3 rot);
	void rotate(float x, float y, float z);
	
	Vector3 forward();
	Vector3 right();

	Transform();

	Vector3 rotatePoint(Vector3 v);
};