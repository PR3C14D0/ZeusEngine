#include "Engine/Core.h"

Core* Core::instance;

Core::Core() {
	this->hwnd = NULL;
	this->bInitialized = false;
	this->nNumBackBuffers = 2;
}

/*
	Our D3D12 initializator
*/
void Core::InitD3D() {
	this->sceneMgr = SceneManager::GetInstance();
	UINT dxgiFactoryCreateFlags = 0;

#ifndef NDEBUG
	dxgiFactoryCreateFlags |= DXGI_CREATE_FACTORY_DEBUG;

	{
		ComPtr<ID3D12Debug> debug;
		ThrowIfFailed(D3D12GetDebugInterface(IID_PPV_ARGS(debug.GetAddressOf())));
	}
#endif // NDEBUG

	ThrowIfFailed(CreateDXGIFactory2(dxgiFactoryCreateFlags, IID_PPV_ARGS(this->factory.GetAddressOf())));

	this->GetMostCapableAdapter(this->adapter, this->factory);

	D3D_FEATURE_LEVEL featureLevel = this->GetMaxFeatureLevel(this->adapter);

	ThrowIfFailed(D3D12CreateDevice(this->adapter.Get(), featureLevel, IID_PPV_ARGS(this->dev.GetAddressOf())));

	ThrowIfFailed(this->dev->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(this->allocator.GetAddressOf())));

	D3D12_COMMAND_QUEUE_DESC queueDesc = { };
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	queueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

	ThrowIfFailed(this->dev->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(this->queue.GetAddressOf())));

	{
		ComPtr<IDXGISwapChain1> sc;

		DXGI_SWAP_CHAIN_DESC1 scDesc = { };
		scDesc.BufferCount = this->nNumBackBuffers;
		scDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		scDesc.SampleDesc.Count = 1;
		scDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
		scDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

		this->factory->CreateSwapChainForHwnd(this->queue.Get(), this->hwnd, &scDesc, nullptr, nullptr, sc.GetAddressOf());

		sc.As(&this->sc);
	}

	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = { };
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	rtvHeapDesc.NumDescriptors = this->nNumBackBuffers;
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;

	ThrowIfFailed(this->dev->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(this->rtvHeap.GetAddressOf())));

	this->nRTVHeapIncrementSize = this->dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(this->rtvHeap->GetCPUDescriptorHandleForHeapStart());
	
	for (int i = 0; i < this->nNumBackBuffers; i++) {
		ComPtr<ID3D12Resource> backBuffer;
		ThrowIfFailed(this->sc->GetBuffer(i, IID_PPV_ARGS(backBuffer.GetAddressOf())));
		this->backBuffers.push_back(backBuffer);

		this->dev->CreateRenderTargetView(this->backBuffers[i].Get(), nullptr, rtvHandle);
		rtvHandle.Offset(1, this->nRTVHeapIncrementSize);
	}

	ThrowIfFailed(this->dev->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, this->allocator.Get(), nullptr, IID_PPV_ARGS(this->list.GetAddressOf())));
	
	ThrowIfFailed(this->list->Close());

	ZeroMemory(&this->viewport, sizeof(D3D12_VIEWPORT));
	this->viewport.Height = this->height;
	this->viewport.Width = this->width;
	this->viewport.TopLeftX = 0;
	this->viewport.TopLeftY = 0;

	this->scissorRect = CD3DX12_RECT(0, 0, (LONG)this->width, (LONG)this->height);

	this->nCurrentFence = 1;
	ThrowIfFailed(this->dev->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(this->fence.GetAddressOf())));
	this->fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	this->WaitFrame();
}

/*
	We'll wait for last frame to end drawing and presenting before draw next frame.
*/
void Core::WaitFrame() {
	UINT nFence = this->nCurrentFence;
	this->queue->Signal(this->fence.Get(), nFence);
	this->nCurrentFence++;

	if (this->fence->GetCompletedValue() < nFence) {
		this->fence->SetEventOnCompletion(nFence, this->fenceEvent);
		WaitForSingleObject(this->fenceEvent, INFINITE);
	}

	this->nCurrentBackBuffer = this->sc->GetCurrentBackBufferIndex();
	return;
}

/*
	Our render loop
*/
void Core::MainLoop() {
	this->sceneMgr->Render();
	ID3D12CommandList* cmdLists[] = { this->list.Get() };
	this->queue->ExecuteCommandLists(1, cmdLists);
	this->sc->Present(1, 0);
	this->WaitFrame();
}

/*
	This method sets hwnd parameter to our Core hwnd.
		Note: there must be an HWND.
*/
void Core::GetHWND(HWND& hwnd) {
	hwnd = this->hwnd;
}

/*
	This method sets our Core window
*/
void Core::SetHWND(HWND& hwnd) {
	this->hwnd = hwnd;
}

/*
	This method will get the most capable device with the minimum feature level
		Note: If there is no capable device, will show an error message.
*/
bool Core::GetMostCapableAdapter(ComPtr<IDXGIAdapter>& adapter, ComPtr<IDXGIFactory2>& factory) {
	if (adapter) return false;
	D3D_FEATURE_LEVEL minimumFeatureLevel = D3D_FEATURE_LEVEL_11_0;

	std::vector<ComPtr<IDXGIAdapter>> adapters;
	
	{
		ComPtr<IDXGIAdapter> tempAdapter;
		for (int i = 0; factory->EnumAdapters(i, tempAdapter.GetAddressOf()) != DXGI_ERROR_NOT_FOUND; i++)
			adapters.push_back(tempAdapter);
	}

	for (ComPtr<IDXGIAdapter> tempAdapter : adapters) {
		if (SUCCEEDED(D3D12CreateDevice(tempAdapter.Get(), minimumFeatureLevel, __uuidof(ID3D12Device), nullptr))) {
			adapter = tempAdapter;
			break;
		}
	}

	if (!adapter)
		MessageBox(this->hwnd, "Your adapter must be at leas D3D_FEATURE_LEVEL_11_0", "Error", MB_ICONERROR | MB_OK);

	return true;
}

/*
	This method gets the maximum feature level for our adapter.
*/
D3D_FEATURE_LEVEL Core::GetMaxFeatureLevel(ComPtr<IDXGIAdapter>& adapter) {
	D3D_FEATURE_LEVEL minimumFeatureLevel = D3D_FEATURE_LEVEL_11_0;
	D3D_FEATURE_LEVEL featureLevels[] = {
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_12_0,
		D3D_FEATURE_LEVEL_12_1,
		D3D_FEATURE_LEVEL_12_2,
	};
	UINT nNumFeatureLevels = _countof(featureLevels);

	D3D12_FEATURE_DATA_FEATURE_LEVELS featureLevelsData = { };
	featureLevelsData.NumFeatureLevels = nNumFeatureLevels;
	featureLevelsData.pFeatureLevelsRequested = featureLevels;

	{
		ComPtr<ID3D12Device> dev;
		ThrowIfFailed(D3D12CreateDevice(adapter.Get(), minimumFeatureLevel, IID_PPV_ARGS(dev.GetAddressOf())));
		dev->CheckFeatureSupport(D3D12_FEATURE_FEATURE_LEVELS, &featureLevelsData, sizeof(featureLevelsData));
	}

	return featureLevelsData.MaxSupportedFeatureLevel;
}

/*
	Our core initialization method.
		Note: This method must be called once.
*/
void Core::Init() {
	if (bInitialized) return;
	if (!this->hwnd) return;

	RECT rect;
	GetWindowRect(this->hwnd, &rect);
	this->width = rect.right - rect.left;
	this->height = rect.bottom - rect.top;

	this->InitD3D();

	this->bInitialized = true;
}

Core* Core::GetInstance() {
	if (Core::instance == nullptr)
		Core::instance = new Core();
	return Core::instance;
}