#include "Math/Vector3.h"

Vector3::Vector3(float x, float y, float z) {
	this->x = x;
	this->y = y;
	this->z = z;
}

Vector3::Vector3(Vector3& vector) {
	this->x = vector.x;
	this->y = vector.y;
	this->z = vector.z;
}

Vector3 Vector3::operator+(Vector3& vector) {
	float x = vector.x;
	float y = vector.y;
	float z = vector.z;

	return Vector3{ this->x + x, this->y + y, this->z + z };
}

Vector3 Vector3::operator*(float& f) {
	return Vector3{ this->x * f, this->y * f, this->z * f };
}

Vector3 Vector3::operator-(Vector3& vector) {
	float x = vector.x;
	float y = vector.y;
	float z = vector.z;
	
	return Vector3(this->x - x, this->y - y, this->z - z);
}