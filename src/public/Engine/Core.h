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
#include "Engine/Scene/SceneManager.h"
#include "Engine/Vertex.h"
#include "Engine/ScreenQuad.h"
#include <imgui/imgui.h>
#include <imgui/backends/imgui_impl_dx12.h>
#include <imgui/backends/imgui_impl_win32.h>
#include <imguizmo/ImGuizmo.h>
#include "Engine/Editor/Editor.h"

using namespace Microsoft::WRL;

enum VSYNC {
	DISABLED = 0,
	ENABLED = 1,
	MEDIUM = 2
};

class Core {
	friend class Editor;
	friend class GameObject;
	friend class ScreenQuad;
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

	UINT nNumBackBuffers;
	ComPtr<ID3D12DescriptorHeap> rtvHeap;
	std::vector<ComPtr<ID3D12Resource>> backBuffers;
	std::vector<ComPtr<ID3D12Resource>> gbuffers;
	ComPtr<ID3D12Resource> screenQuadBuff;
	UINT gbufferIndices[3];
	UINT sqBuffIndex;
	UINT nRTVHeapIncrementSize;
	UINT nCurrentBackBuffer;

	D3D12_VIEWPORT viewport;
	D3D12_RECT scissorRect;
	int width, height;

	ComPtr<ID3D12DescriptorHeap> cbv_srvHeap;
	UINT cbv_srvHeapIncrementSize;
	UINT cbv_srvUsedDescriptors;

	ComPtr<ID3D12Fence> fence;
	HANDLE fenceEvent;
	UINT nCurrentFence;

	SceneManager* sceneMgr;
	ScreenQuad* screenQuad;
	
	bool GetMostCapableAdapter(ComPtr<IDXGIAdapter>& adapter, ComPtr<IDXGIFactory2>& factory);
	D3D_FEATURE_LEVEL GetMaxFeatureLevel(ComPtr<IDXGIAdapter>& adapter);

	ComPtr<ID3D12Resource> zBuffer;
	ComPtr<ID3D12DescriptorHeap> dsvHeap;

	ComPtr<ID3D12DescriptorHeap> samplerHeap;
	UINT nSamplerIncrementSize;
	UINT nSamplerUsedDescriptors;

	Editor* editor;

	void WaitFrame();
	void PopulateCommandList();

	ImGuiIO* imIO;

	UINT sampleCount; // Change sample count

	VSYNC vSyncState;
public:
	Core();
	static Core* GetInstance();
	void SetHWND(HWND& hwnd);
	void GetHWND(HWND& hwnd);
	void Init();


	UINT CBV_SRV_AddDescriptorToCount();
	UINT SAMPLER_AddDescriptorToCount();

	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHeapHandle(D3D12_DESCRIPTOR_HEAP_TYPE type);
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHeapHandle(D3D12_DESCRIPTOR_HEAP_TYPE type);
	UINT GetDescriptorHeapHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE type);

	void GetDevice(ComPtr<ID3D12Device>& dev, ComPtr<ID3D12GraphicsCommandList>& list);

	void SetVSYNC(VSYNC state);

	void GetWindowSize(int& width, int& height);
	
	void MainLoop();
};