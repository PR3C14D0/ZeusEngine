#include "Engine/Core.h"

Core* Core::instance;

Core::Core() {
	this->hwnd = NULL;
	this->bInitialized = false;
}

/*
	Our D3D12 initializator
*/
void Core::InitD3D() {

}

/*
	Our core initialization method.
		Note: This method must be called once.
*/
void Core::Init() {
	if (bInitialized) return;

	this->InitD3D();

	this->bInitialized = true;
}

Core* Core::GetInstance() {
	if (!Core::instance)
		Core::instance = new Core();
	return Core::instance;
}