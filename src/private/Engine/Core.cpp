#include "Engine/Core.h"

Core* Core::instance;

Core::Core() {
	this->hwnd = NULL;
	this->bInitialized = false;
	this->nNumBackBuffers = 2;
	this->cbv_srvUsedDescriptors = 0;
}

/*
	Our D3D12 initializator
*/
void Core::InitD3D() {
	UINT dxgiFactoryCreateFlags = 0;

#ifndef NDEBUG
	dxgiFactoryCreateFlags |= DXGI_CREATE_FACTORY_DEBUG;

	{
		ComPtr<ID3D12Debug1> debug;
		ThrowIfFailed(D3D12GetDebugInterface(IID_PPV_ARGS(debug.GetAddressOf())));
		debug->EnableDebugLayer();
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
		scDesc.Width = this->width;
		scDesc.Height = this->height;

		this->factory->CreateSwapChainForHwnd(this->queue.Get(), this->hwnd, &scDesc, nullptr, nullptr, sc.GetAddressOf());

		sc.As(&this->sc);
	}

	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = { };
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	rtvHeapDesc.NumDescriptors = this->nNumBackBuffers + 2; // +2 Because of Albedo and Normals
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

	gbufferIndices[0] = this->nNumBackBuffers;
	gbufferIndices[1] = this->nNumBackBuffers + 1;

	D3D12_RESOURCE_DESC gBuffDesc = { };
	gBuffDesc.DepthOrArraySize = 1;
	gBuffDesc.MipLevels = 1;
	gBuffDesc.Height = this->height;
	gBuffDesc.Width = this->width;
	gBuffDesc.SampleDesc.Count = 1;
	gBuffDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
	gBuffDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	gBuffDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;

	D3D12_HEAP_PROPERTIES gBuffProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);

	D3D12_RENDER_TARGET_VIEW_DESC grtvDesc = { };
	grtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	grtvDesc.Format = gBuffDesc.Format;
	
	D3D12_CLEAR_VALUE gbuffClear = { };
	gbuffClear.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	gbuffClear.Color[3] = 1.f;

	for (int n : this->gbufferIndices) {
		ComPtr<ID3D12Resource> gbuffer;
		this->dev->CreateCommittedResource(
			&gBuffProps,
			D3D12_HEAP_FLAG_NONE,
			&gBuffDesc,
			D3D12_RESOURCE_STATE_RENDER_TARGET,
			&gbuffClear,
			IID_PPV_ARGS(gbuffer.GetAddressOf())
		);

		this->gbuffers.push_back(gbuffer);
		
		this->dev->CreateRenderTargetView(this->gbuffers[n - this->nNumBackBuffers].Get(), &grtvDesc, rtvHandle);
		rtvHandle.Offset(1, this->nRTVHeapIncrementSize);
	}

	ThrowIfFailed(this->dev->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, this->allocator.Get(), nullptr, IID_PPV_ARGS(this->list.GetAddressOf())));
	
	ThrowIfFailed(this->list->Close());

	ZeroMemory(&this->viewport, sizeof(D3D12_VIEWPORT));
	this->viewport.Height = this->height;
	this->viewport.Width = this->width;
	this->viewport.TopLeftX = 0;
	this->viewport.TopLeftY = 0;
	this->viewport.MinDepth = 0.f;
	this->viewport.MaxDepth = 1.f;

	this->scissorRect = CD3DX12_RECT(0, 0, (LONG)this->width, (LONG)this->height);

	D3D12_DESCRIPTOR_HEAP_DESC cbvSrvHeapDesc = { };
	cbvSrvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	cbvSrvHeapDesc.NumDescriptors = D3D12_MAX_SHADER_VISIBLE_DESCRIPTOR_HEAP_SIZE_TIER_1;
	cbvSrvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;


	ThrowIfFailed(this->dev->CreateDescriptorHeap(&cbvSrvHeapDesc, IID_PPV_ARGS(this->cbv_srvHeap.GetAddressOf())));
	this->cbv_srvHeapIncrementSize = this->dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	
	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = { };
	dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	dsvHeapDesc.NumDescriptors = 1;
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	ThrowIfFailed(this->dev->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(this->dsvHeap.GetAddressOf())));

	D3D12_RESOURCE_DESC dsvTexDesc = { };
	dsvTexDesc.DepthOrArraySize = 1;
	dsvTexDesc.MipLevels = 1;
	dsvTexDesc.SampleDesc.Count = 1;
	dsvTexDesc.Height = this->height;
	dsvTexDesc.Width = this->width;
	dsvTexDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	dsvTexDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
	dsvTexDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	
	D3D12_HEAP_PROPERTIES dsvTexProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);

	D3D12_CLEAR_VALUE dsvClear = { };
	dsvClear.DepthStencil.Depth = 1.f;
	dsvClear.DepthStencil.Stencil = 1.f;
	dsvClear.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;


	ThrowIfFailed(
		this->dev->CreateCommittedResource(
			&dsvTexProps,
			D3D12_HEAP_FLAG_NONE,
			&dsvTexDesc,
			D3D12_RESOURCE_STATE_DEPTH_WRITE,
			&dsvClear,
			IID_PPV_ARGS(this->zBuffer.GetAddressOf())
		)
	);

	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = { };
	dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Format = dsvTexDesc.Format;

	this->dev->CreateDepthStencilView(this->zBuffer.Get(), &dsvDesc, this->dsvHeap->GetCPUDescriptorHandleForHeapStart());
	
	D3D12_DESCRIPTOR_HEAP_DESC samplerHeapDesc = { };
	samplerHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	samplerHeapDesc.NumDescriptors = D3D12_MAX_SHADER_VISIBLE_SAMPLER_HEAP_SIZE;
	samplerHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;

	ThrowIfFailed(this->dev->CreateDescriptorHeap(&samplerHeapDesc, IID_PPV_ARGS(this->samplerHeap.GetAddressOf())));

	this->InitScreenQuad();

	this->sceneMgr = SceneManager::GetInstance();

	this->nCurrentFence = 1;
	ThrowIfFailed(this->dev->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(this->fence.GetAddressOf())));
	this->fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	this->WaitFrame();
}

/*
	Creates our Screen quad
*/
void Core::InitScreenQuad() {
	ScreenQuadVertex sqVertices[] = {
		{{-1.f, 1.f, 0.f}, {0.f, 0.f}},
		{{1.f, 1.f, 0.f}, {1.f, 0.f}},
		{{-1.f, -1.f, 0.f}, {0.f, 1.f}},
		{{1.f, -1.f, 0.f}, {1.f, 1.f}}
	};

	UINT indices[] = {
		0, 1, 2,
		1, 3, 2
	};

	D3D12_RESOURCE_DESC sqDesc = CD3DX12_RESOURCE_DESC::Buffer(sizeof(sqVertices));
	D3D12_HEAP_PROPERTIES sqProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);

	ThrowIfFailed(
		this->dev->CreateCommittedResource(
			&sqProps,
			D3D12_HEAP_FLAG_NONE,
			&sqDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(this->sqBuff.GetAddressOf())
		)
	);

	PUINT ms;
	this->sqBuff->Map(NULL, nullptr, (void**)&ms);
	memcpy(ms, sqVertices, sizeof(sqVertices));
	this->sqBuff->Unmap(NULL, nullptr);

	this->sqBuffView.BufferLocation = this->sqBuff->GetGPUVirtualAddress();
	this->sqBuffView.SizeInBytes = sizeof(sqVertices);
	this->sqBuffView.StrideInBytes = sizeof(ScreenQuadVertex);

	D3D12_RESOURCE_DESC iboDesc = CD3DX12_RESOURCE_DESC::Buffer(sizeof(indices));
	D3D12_HEAP_PROPERTIES iboProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);

	ThrowIfFailed(
		this->dev->CreateCommittedResource(
			&iboProps,
			D3D12_HEAP_FLAG_NONE,
			&iboDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(this->sqIBO.GetAddressOf())
		)
	);

	this->sqIBO->Map(NULL, nullptr, (void**)&ms);
	memcpy(ms, indices, sizeof(indices));
	this->sqIBO->Unmap(NULL, nullptr);

	this->sqIBOView.BufferLocation = this->sqIBO->GetGPUVirtualAddress();
	this->sqIBOView.Format = DXGI_FORMAT_R32_UINT;
	this->sqIBOView.SizeInBytes = sizeof(indices);
	
	this->sqShader = new Shader(L"LightPass.hlsl", "VertexMain", "PixelMain");

	CD3DX12_DESCRIPTOR_RANGE samplerRange;
	samplerRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, 1, 0, 0);

	CD3DX12_DESCRIPTOR_RANGE textureRange;
	textureRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 2, 0, 0);

	CD3DX12_ROOT_PARAMETER samplerParam;
	samplerParam.InitAsDescriptorTable(1, &samplerRange, D3D12_SHADER_VISIBILITY_PIXEL);

	CD3DX12_ROOT_PARAMETER textureParam;
	textureParam.InitAsDescriptorTable(1, &textureRange, D3D12_SHADER_VISIBILITY_PIXEL);

	D3D12_ROOT_PARAMETER params[] = {
		textureParam,
		samplerParam,
	};

	D3D12_ROOT_SIGNATURE_DESC rootSigDesc = { };
	rootSigDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	rootSigDesc.NumParameters = _countof(params);
	rootSigDesc.pParameters = params;
	rootSigDesc.pStaticSamplers = nullptr;
	rootSigDesc.NumStaticSamplers = 0;

	ComPtr<ID3DBlob> rootSigBlob, rootSigErr;

	ThrowIfFailed(D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1, rootSigBlob.GetAddressOf(), rootSigErr.GetAddressOf()));

	if (rootSigErr) {
		MessageBox(this->hwnd, (char*)rootSigErr->GetBufferPointer(), "Error", MB_ICONERROR | MB_OK);
		return;
	}

	ThrowIfFailed(this->dev->CreateRootSignature(0, rootSigBlob->GetBufferPointer(), rootSigBlob->GetBufferSize(), IID_PPV_ARGS(this->sqRs.GetAddressOf())));


	D3D12_SAMPLER_DESC samplerDesc = { };
	samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	samplerDesc.MinLOD = 0.f;
	samplerDesc.MaxLOD = D3D12_FLOAT32_MAX;
	samplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.MaxAnisotropy = 1;
	samplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	
	this->dev->CreateSampler(&samplerDesc, this->samplerHeap->GetCPUDescriptorHandleForHeapStart());

	D3D12_INPUT_ELEMENT_DESC elements[] = {
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, NULL},
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, NULL},
	};

	D3D12_INPUT_LAYOUT_DESC layout = { };
	layout.NumElements = _countof(elements);
	layout.pInputElementDescs = elements;

	ComPtr<ID3DBlob> VS, PS;
	this->sqShader->GetBlob(VS, PS);

	D3D12_GRAPHICS_PIPELINE_STATE_DESC plDesc = { };
	plDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	plDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	plDesc.InputLayout = layout;
	plDesc.VS = CD3DX12_SHADER_BYTECODE(VS.Get());
	plDesc.PS = CD3DX12_SHADER_BYTECODE(PS.Get());
	plDesc.RTVFormats[0] = DXGI_FORMAT_B8G8R8A8_UNORM;
	plDesc.NumRenderTargets = 1;
	plDesc.SampleDesc.Count = 1;
	plDesc.SampleMask = UINT32_MAX;
	plDesc.pRootSignature = this->sqRs.Get();
	plDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
	plDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	plDesc.DepthStencilState.DepthEnable = FALSE;
	plDesc.DepthStencilState.StencilEnable = FALSE;

	ThrowIfFailed(this->dev->CreateGraphicsPipelineState(&plDesc, IID_PPV_ARGS(this->sqPl.GetAddressOf())));

	albedoIndex = this->CBV_SRV_AddDescriptorToCount();
	normalIndex = this->CBV_SRV_AddDescriptorToCount();

	CD3DX12_CPU_DESCRIPTOR_HANDLE albedoHandle(this->cbv_srvHeap->GetCPUDescriptorHandleForHeapStart(), albedoIndex, this->cbv_srvHeapIncrementSize);
	CD3DX12_CPU_DESCRIPTOR_HANDLE normalHandle(this->cbv_srvHeap->GetCPUDescriptorHandleForHeapStart(), normalIndex, this->cbv_srvHeapIncrementSize);

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = { };
	srvDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	
	this->dev->CreateShaderResourceView(this->gbuffers[0].Get(), &srvDesc, albedoHandle);
	this->dev->CreateShaderResourceView(this->gbuffers[1].Get(), &srvDesc, normalHandle);

	return;
}

D3D12_GPU_DESCRIPTOR_HANDLE Core::GetGPUDescriptorHeapHandle(D3D12_DESCRIPTOR_HEAP_TYPE type) {
	if (type == D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV)
		return this->cbv_srvHeap->GetGPUDescriptorHandleForHeapStart();
}

UINT Core::CBV_SRV_AddDescriptorToCount() {
	UINT returnedIndex = this->cbv_srvUsedDescriptors;
	this->cbv_srvUsedDescriptors++;
	return returnedIndex;
}

/*
	Returns a descriptor heap cpu handle.
*/
D3D12_CPU_DESCRIPTOR_HANDLE Core::GetCPUDescriptorHeapHandle(D3D12_DESCRIPTOR_HEAP_TYPE type) {
	D3D12_CPU_DESCRIPTOR_HANDLE handle{};
	if (type == D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV)
		handle = this->cbv_srvHeap->GetCPUDescriptorHandleForHeapStart();
	
	return handle;
}

/*
	Returns a descriptor heap cpu handle increment size.
*/
UINT Core::GetDescriptorHeapHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE type) {
	if (type == D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV)
		return this->cbv_srvHeapIncrementSize;
}

/*
	This method is for getting our window height and width
*/
void Core::GetWindowSize(int& width, int& height) {
	width = this->width;
	height = this->height;
	return;
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
	This method gets our device and command list.
*/
void Core::GetDevice(ComPtr<ID3D12Device>& dev, ComPtr<ID3D12GraphicsCommandList>& list) {
	if (!this->bInitialized) return;
	dev = this->dev;
	list = this->list;
	return;
}

void Core::RenderScreenQuad() {
	D3D12_RESOURCE_BARRIER resourceBarrier = CD3DX12_RESOURCE_BARRIER::Transition(this->gbuffers[0].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	this->list->ResourceBarrier(1, &resourceBarrier);
	this->list->IASetVertexBuffers(0, 1, &this->sqBuffView);
	this->list->IASetIndexBuffer(&this->sqIBOView);
	this->list->SetPipelineState(this->sqPl.Get());
	this->list->SetGraphicsRootSignature(this->sqRs.Get());
	this->list->RSSetViewports(1, &this->viewport);
	this->list->RSSetScissorRects(1, &this->scissorRect);
	ID3D12DescriptorHeap* heaps[] = {
		this->cbv_srvHeap.Get(),
		this->samplerHeap.Get()
	};

	CD3DX12_GPU_DESCRIPTOR_HANDLE albedoHandle(this->cbv_srvHeap->GetGPUDescriptorHandleForHeapStart(), albedoIndex, this->cbv_srvHeapIncrementSize);

	this->list->SetDescriptorHeaps(_countof(heaps), heaps);
	this->list->SetGraphicsRootDescriptorTable(0, albedoHandle);
	this->list->SetGraphicsRootDescriptorTable(1, this->samplerHeap->GetGPUDescriptorHandleForHeapStart());
	this->list->DrawIndexedInstanced(6, 1, 0, 0, 0);
	resourceBarrier = CD3DX12_RESOURCE_BARRIER::Transition(this->gbuffers[0].Get(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);
	this->list->ResourceBarrier(1, &resourceBarrier);
}

void Core::PopulateCommandList() {
	ThrowIfFailed(this->allocator->Reset());
	ThrowIfFailed(this->list->Reset(this->allocator.Get(), nullptr));

	D3D12_RESOURCE_BARRIER resBarrier = CD3DX12_RESOURCE_BARRIER::Transition(this->backBuffers[this->nCurrentBackBuffer].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
	this->list->ResourceBarrier(1, &resBarrier);
	
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(this->rtvHeap->GetCPUDescriptorHandleForHeapStart(), this->nCurrentBackBuffer, this->nRTVHeapIncrementSize);
	CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(this->dsvHeap->GetCPUDescriptorHandleForHeapStart());
		
	std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> gbufferHandles;
	CD3DX12_CPU_DESCRIPTOR_HANDLE albedoHandle(this->rtvHeap->GetCPUDescriptorHandleForHeapStart(), this->gbufferIndices[0], this->nRTVHeapIncrementSize);
	CD3DX12_CPU_DESCRIPTOR_HANDLE normalHandle(this->rtvHeap->GetCPUDescriptorHandleForHeapStart(), this->gbufferIndices[1], this->nRTVHeapIncrementSize);

	gbufferHandles.push_back(albedoHandle);
	gbufferHandles.push_back(normalHandle);

	this->list->OMSetRenderTargets(gbufferHandles.size(), gbufferHandles.data(), FALSE, &dsvHandle);
	this->list->ClearRenderTargetView(albedoHandle, RGBA{ 0.f, 0.f, 0.f, 1.f }, 0, nullptr);
	this->list->ClearRenderTargetView(normalHandle, RGBA{0.f, 0.f, 0.f, 1.f}, 0, nullptr);
	this->list->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.f, 0.f, 0, nullptr);
	
	this->list->RSSetScissorRects(1, &this->scissorRect);
	this->list->RSSetViewports(1, &this->viewport);
	this->list->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	this->sceneMgr->Render();

	this->list->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);
	this->list->ClearRenderTargetView(rtvHandle, RGBA{ 0.f, 0.f, 0.f, 1.f }, 0, nullptr);
	this->RenderScreenQuad();
	resBarrier = CD3DX12_RESOURCE_BARRIER::Transition(this->backBuffers[this->nCurrentBackBuffer].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
	this->list->ResourceBarrier(1, &resBarrier);
	ThrowIfFailed(this->list->Close());
}

/*
	Our render loop
*/
void Core::MainLoop() {
	this->PopulateCommandList();
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