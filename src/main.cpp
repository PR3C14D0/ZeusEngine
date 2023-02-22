#include <Windows.h>
#include <time.h>
#include "Engine/Core.h"
#include "Engine/Input.h"
#include "Module/Time.h"

LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

bool g_quit = false;
Core* g_core = Core::GetInstance();
Input* g_input = Input::GetInstance();
Time* g_time = Time::GetInstance();

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd) {
	const char CLASS_NAME[] = "ZeusEngine";

	WNDCLASS wc = { };
	wc.lpszClassName = CLASS_NAME;
	wc.hInstance = hInstance;
	wc.lpfnWndProc = WndProc;
	
	RegisterClass(&wc);

	HWND hwnd = CreateWindowEx(
		NULL,
		CLASS_NAME,
		"Zeus Engine",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT,
		CW_USEDEFAULT, CW_USEDEFAULT,

		NULL, NULL, hInstance, NULL
	);

	if (!hwnd) {
		MessageBox(NULL, "An error occurred creating the window", "Error", MB_ICONERROR | MB_OK);
		return 1;
	}

	ShowWindow(hwnd, nShowCmd);

	g_core->SetHWND(hwnd);
	g_core->Init();
	g_input->SetHWND(hwnd);

	clock_t startTime = clock();
	float deltaTime = 0.f;
	clock_t endTime;

	MSG msg = { };
	while (!g_quit) {
		g_input->RemoveReleased();
		while (PeekMessage(&msg, NULL, NULL, NULL, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		g_core->MainLoop();
		endTime = clock();
		deltaTime = endTime - startTime;
		startTime = endTime;
		g_time->SetDelta(deltaTime / 1000.f);
	}
	
	return 0;
}

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	if (ImGui_ImplWin32_WndProcHandler(hwnd, uMsg, wParam, lParam))
		return 0;

	g_input->Update(wParam, lParam);
	char pressedKey;

	switch (uMsg) {
	case WM_LBUTTONDOWN:
		g_input->SetButtonDown(LEFT);
		return 0;
	case WM_RBUTTONDOWN:
		g_input->SetButtonDown(RIGHT);
		return 0;
	case WM_LBUTTONUP:
		g_input->SetButtonDown(LEFT);
		return 0;
	case WM_RBUTTONUP:
		g_input->SetButtonUp(RIGHT);
		return 0;
	case WM_KEYUP:
		pressedKey = g_input->pressedKey;
		g_input->SetKeyUp(pressedKey);
		return 0;
	case WM_KEYDOWN:
		pressedKey = g_input->pressedKey;
		g_input->SetKeyDown(pressedKey);
		return 0;
	case WM_CLOSE:
		if (MessageBox(hwnd, "Are you sure you want to close Zeus Engine?", "Sure?", MB_OKCANCEL) == IDOK)
			DestroyWindow(hwnd);
		return 0;
	case WM_DESTROY:
		g_quit = true;
		PostQuitMessage(0);
		return 0;
	}

	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}