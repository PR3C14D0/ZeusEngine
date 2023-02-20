#pragma once
#include <iostream>

class Time {
private:
	static Time* instance;
public:
	Time();
	static Time* GetInstance();

	float deltaTime;

	void SetDelta(float delta);
};