#pragma once
#include <iostream>
#include "Engine/GameObject/GameObject.h"
#include "Engine/WVP.h"
#include "Engine/Input.h"

class Core;

class Camera : public GameObject{
private:
	std::string name;
	WVP transformMatrix;

	Core* core;

	int width, height;
protected:
	Input* input;
public:
	Camera(std::string name);
	virtual void Init() override; // We dont want to use our GameObject::Init method
	virtual void Render() override; // We dont want to use our GameObject::Render method
	WVP GetTransform();
};