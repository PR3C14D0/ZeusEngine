#pragma once
#include <Windows.h>

class Core {
private:
	HWND hwnd;
	static Core* instance;
	
	bool bInitialized;

	void InitD3D();
public:
	Core();
	static Core* GetInstance();

	void Init();
};