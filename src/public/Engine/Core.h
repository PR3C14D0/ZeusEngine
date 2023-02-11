#pragma once
#include <iostream>
#include <vector>
#include <Windows.h>
#include <wrl.h>
#include <directx/d3d12.h>
#include <directx/d3dx12.h>
#include <dxgi1_4.h>
#include "Util/Util.h"
#include "Engine/Shader.h"
#include "Engine/Vertex.h"

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
	ComPtr<ID3D12GraphicsCommandList> list;
	
	ComPtr<ID3D12PipelineState> plState;

	ComPtr<ID3D12Resource> vertexBuffer;
	D3D12_VERTEX_BUFFER_VIEW vertexBuffView;

	UINT nNumBackBuffers;
	ComPtr<ID3D12DescriptorHeap> rtvHeap;
	std::vector<ComPtr<ID3D12Resource>> backBuffers;
	UINT nRTVHeapIncrementSize;

	ComPtr<ID3D12RootSignature> rootSig;

	D3D12_VIEWPORT viewport;
	D3D12_RECT scissorRect;
	int width, height;

	// Temporal
	void InitPipeline();
	void UploadBuffer();
	void PopulateCommandList();
	// End:Temporal

	bool GetMostCapableAdapter(ComPtr<IDXGIAdapter>& adapter, ComPtr<IDXGIFactory2>& factory);
	D3D_FEATURE_LEVEL GetMaxFeatureLevel(ComPtr<IDXGIAdapter>& adapter);
public:
	Core();
	static Core* GetInstance();
	void SetHWND(HWND& hwnd);
	void GetHWND(HWND& hwnd);
	void Init();
};