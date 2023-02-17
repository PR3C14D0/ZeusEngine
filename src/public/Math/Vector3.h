#pragma once

struct Vector3 {
public:
	float x, y, z;

	Vector3() = default;
	Vector3(float x, float y, float z);
	Vector3(Vector3& vector);

	Vector3 operator+(Vector3& vector);
	Vector3 operator-(Vector3& vector);
	Vector3 operator*(float& f);
};