#include "Module/Time.h"

Time* Time::instance;

Time::Time() {
	this->deltaTime = 0.f;
}

void Time::SetDelta(float delta) {
	this->deltaTime = delta;
	return;
}

Time* Time::GetInstance() {
	if (Time::instance == nullptr)
		Time::instance = new Time();
	return Time::instance;
}