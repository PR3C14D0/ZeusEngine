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

	this->InitPipeline();
	ThrowIfFailed(this->dev->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, this->allocator.Get(), this->plState.Get(), IID_PPV_ARGS(this->list.GetAddressOf())));
	
	ThrowIfFailed(this->list->Close());

	this->UploadBuffer();
}

/*
*	TODO: Delete this.
*/
void Core::UploadBuffer() {
	vertex vertices[] = {
		{.5f, 0.f, 0.f, {1.f, 0.f, 0.f, 1.f}},
		{-.5f, .5f, 0.f, {0.f, 1.f, 0.f, 1.f}},
		{-.5f, -.5f, 0.f, {0.f, 0.f, 1.f, 1.f}},
	};

	UINT verticesSize = sizeof(vertices);

	D3D12_RESOURCE_DESC buffDesc = CD3DX12_RESOURCE_DESC::Buffer(verticesSize);
	D3D12_HEAP_PROPERTIES buffProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);

	ThrowIfFailed(this->dev->CreateCommittedResource(
		&buffProps,
		D3D12_HEAP_FLAG_NONE,
		&buffDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(this->vertexBuffer.GetAddressOf())
	));

	PUINT mappedBuffer;
	this->vertexBuffer->Map(NULL, nullptr, (void**)&mappedBuffer);
	memcpy(mappedBuffer, vertices, verticesSize);
	this->vertexBuffer->Unmap(0, nullptr);

	return;
}

/*
	This method populates our command list.
	I'll modify this method drastically soon.
*/
void Core::PopulateCommandList() {
	ThrowIfFailed(this->allocator->Reset());
	ThrowIfFailed(this->list->Reset(this->allocator.Get(), this->plState.Get()));

	
}

/*
	This method initializes our pipeline state
		TODO: Create many pipelines for each GameObject (I'll create GameObject class soon)
*/
void Core::InitPipeline() {
	D3D12_ROOT_SIGNATURE_DESC rootSigDesc = { };
	rootSigDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	rootSigDesc.NumParameters = 0;
	rootSigDesc.pParameters = nullptr;
	rootSigDesc.NumStaticSamplers = 0;
	rootSigDesc.pStaticSamplers = nullptr;

	ComPtr<ID3DBlob> rootSigBlob, rootSigErr;
	ThrowIfFailed(D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1_0, rootSigBlob.GetAddressOf(), rootSigErr.GetAddressOf()));

	if (rootSigErr) {
		MessageBox(this->hwnd, (char*)rootSigErr->GetBufferPointer(), "Error", MB_OK | MB_ICONERROR);
		return;
	}

	ThrowIfFailed(this->dev->CreateRootSignature(0, rootSigBlob->GetBufferPointer(), rootSigBlob->GetBufferSize(), IID_PPV_ARGS(this->rootSig.GetAddressOf())));

	Shader* shader = new Shader(L"shader.fx", "VertexMain", "PixelMain");
	ComPtr<ID3DBlob> VS, PS;
	shader->GetBlob(VS, PS);

	D3D12_INPUT_ELEMENT_DESC elements[] = {
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, NULL},
		{"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, NULL},
	};

	UINT nNumElements = _countof(elements);

	D3D12_INPUT_LAYOUT_DESC layout = { };
	layout.NumElements = nNumElements;
	layout.pInputElementDescs = elements;

	D3D12_GRAPHICS_PIPELINE_STATE_DESC plDesc = { };
	plDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	plDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	plDesc.RTVFormats[0] = DXGI_FORMAT_B8G8R8A8_UNORM;
	plDesc.NumRenderTargets = 1;
	plDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	plDesc.pRootSignature = this->rootSig.Get();
	plDesc.VS = CD3DX12_SHADER_BYTECODE(VS.Get());
	plDesc.PS = CD3DX12_SHADER_BYTECODE(PS.Get());
	plDesc.DepthStencilState.DepthEnable = FALSE;
	plDesc.DepthStencilState.StencilEnable = FALSE;
	plDesc.InputLayout = layout;
	plDesc.SampleDesc.Count = 1;
	plDesc.SampleMask = UINT32_MAX;
	
	ThrowIfFailed(this->dev->CreateGraphicsPipelineState(&plDesc, IID_PPV_ARGS(this->plState.GetAddressOf())));
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
	if (!Core::instance)
		Core::instance = new Core();
	return Core::instance;
}