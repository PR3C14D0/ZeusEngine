#pragma once
#include <iostream>
#include <vector>
#include <Windows.h>
#include <wrl.h>
#include <directx/d3d12.h>
#include <dxgi1_4.h>
#include "Util/Util.h"

using namespace Microsoft::WRL;

class Core {
private:
	HWND hwnd;
	static Core* instance;
	
	bool bInitialized;

	void InitD3D();

	ComPtr<IDXGIFactory2> factory;
	ComPtr<IDXGIAdapter> adapter;

	ComPtr<IDXGISwapChain3> sc;

	ComPtr<ID3D12Device> dev;
	ComPtr<ID3D12CommandAllocator> allocator;
	ComPtr<ID3D12CommandQueue> queue;

	UINT nNumBackBuffers;

	bool GetMostCapableAdapter(ComPtr<IDXGIAdapter>& adapter, ComPtr<IDXGIFactory2>& factory);
	D3D_FEATURE_LEVEL GetMaxFeatureLevel(ComPtr<IDXGIAdapter>& adapter);
public:
	Core();
	static Core* GetInstance();
	void SetHWND(HWND& hwnd);
	void GetHWND(HWND& hwnd);
	void Init();
};