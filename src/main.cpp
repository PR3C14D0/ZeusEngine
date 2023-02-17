#include <Windows.h>
#include "Engine/Core.h"
#include "Engine/Input.h"

LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

bool g_quit = false;
Core* g_core = Core::GetInstance();
Input* g_input = Input::GetInstance();

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

	MSG msg = { };
	while (!g_quit) {
		while (PeekMessage(&msg, NULL, NULL, NULL, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		g_core->MainLoop();
		g_input->RemoveReleased();
	}
	
	return 0;
}


LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
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