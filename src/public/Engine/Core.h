#pragma once
#include <iostream>
#include <vector>
#include <Windows.h>
#include <wrl.h>
#include <directx/d3d12.h>
#include <dxgi1_4.h>
#include "Engine/Util.h"

using namespace Microsoft::WRL;

class Core {
private:
	HWND hwnd;
	static Core* instance;
	
	bool bInitialized;

	void InitD3D();

	ComPtr<IDXGIFactory2> factory;
	ComPtr<IDXGIAdapter> adapter;

	ComPtr<ID3D12Device> dev;
	ComPtr<ID3D12CommandAllocator> allocator;
	ComPtr<ID3D12CommandQueue> queue;

	bool GetMostCapableAdapter(ComPtr<IDXGIAdapter>& adapter, ComPtr<IDXGIFactory2>& factory);
	D3D_FEATURE_LEVEL GetMaxFeatureLevel(ComPtr<IDXGIAdapter>& adapter);
public:
	Core();
	static Core* GetInstance();

	void Init();
};