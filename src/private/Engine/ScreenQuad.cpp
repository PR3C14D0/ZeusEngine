#include "Engine/ScreenQuad.h"
#include "Engine/Core.h"

ScreenQuad::ScreenQuad() {
	this->core = Core::GetInstance();
	
	ScreenQuadVertex vertices[] = {
		{{-1.f, 1.f, 0.f}, {0.f, 0.f}},
		{{1.f, 1.f, 0.f}, {1.f, 0.f}},
		{{1.f, -1.f, 0.f}, {1.f, 1.f}},
		{{-1.f, -1.f, 0.f}, {0.f, 1.f}},
	};

	UINT verticesSize = sizeof(vertices);

	UINT indices[] = {
		0, 1, 3,
		3, 1, 2
	};

	this->core->GetWindowSize(this->width, this->height);

	UINT indicesSize = sizeof(indices);

	D3D12_RESOURCE_DESC vbDesc = CD3DX12_RESOURCE_DESC::Buffer(verticesSize);
	D3D12_HEAP_PROPERTIES vbProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);

	ThrowIfFailed(
		this->core->dev->CreateCommittedResource(
			&vbProps,
			D3D12_HEAP_FLAG_NONE,
			&vbDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(this->vertexBuff.GetAddressOf())
		)
	);

	PUINT mappedVBO;
	this->vertexBuff->Map(NULL, nullptr, (void**)&mappedVBO);
	memcpy(mappedVBO, vertices, verticesSize);
	this->vertexBuff->Unmap(NULL, nullptr);

	this->vertexBuff->SetName(L"Screen Quad VBO");

	this->vbView.BufferLocation = this->vertexBuff->GetGPUVirtualAddress();
	this->vbView.SizeInBytes = verticesSize;
	this->vbView.StrideInBytes = sizeof(ScreenQuadVertex);

	D3D12_RESOURCE_DESC IBODesc = CD3DX12_RESOURCE_DESC::Buffer(indicesSize);
	D3D12_HEAP_PROPERTIES IBOProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);

	ThrowIfFailed(
		this->core->dev->CreateCommittedResource(
			&IBOProps,
			D3D12_HEAP_FLAG_NONE,
			&IBODesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(this->IBO.GetAddressOf())
		)
	);

	PUINT mappedIBO;
	this->IBO->Map(NULL, nullptr, (void**)&mappedIBO);
	memcpy(mappedIBO, indices, indicesSize);
	this->IBO->Unmap(NULL, nullptr);

	this->IBOView.BufferLocation = this->IBO->GetGPUVirtualAddress();
	this->IBOView.Format = DXGI_FORMAT_R32_UINT;
	this->IBOView.SizeInBytes = indicesSize;

	this->shader = new Shader(L"LightPass.hlsl", "VertexMain", "PixelMain");

	this->InitPipeline();

	this->albedoIndex = this->core->CBV_SRV_AddDescriptorToCount();
	this->normalIndex = this->core->CBV_SRV_AddDescriptorToCount();
	this->depthIndex = this->core->CBV_SRV_AddDescriptorToCount();
	this->posIndex = this->core->CBV_SRV_AddDescriptorToCount();

	this->cbv_srvCPUHandle = this->core->GetCPUDescriptorHeapHandle(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	this->cbv_srvGPUHandle = this->core->GetGPUDescriptorHeapHandle(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	this->cbv_srvIncrementSize = this->core->GetDescriptorHeapHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	this->albedoCPUHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(this->cbv_srvCPUHandle, this->albedoIndex, this->cbv_srvIncrementSize);
	this->normalCPUHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(this->cbv_srvCPUHandle, this->normalIndex, this->cbv_srvIncrementSize);
	this->depthCPUHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(this->cbv_srvCPUHandle, this->depthIndex, this->cbv_srvIncrementSize);
	this->posCPUHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(this->cbv_srvCPUHandle, this->posIndex, this->cbv_srvIncrementSize);

	D3D12_RESOURCE_DESC gbuffersDesc = { };
	gbuffersDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	gbuffersDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	gbuffersDesc.Width = this->width;
	gbuffersDesc.Height = this->height;
	gbuffersDesc.SampleDesc.Count = 1;
	gbuffersDesc.DepthOrArraySize = 1;
	gbuffersDesc.MipLevels = 1;
	gbuffersDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

	D3D12_HEAP_PROPERTIES gbuffProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	
	for (int i = 0; i < _countof(this->gbuffers); i++) {
		ThrowIfFailed(this->core->dev->CreateCommittedResource(
			&gbuffProps,
			D3D12_HEAP_FLAG_NONE,
			&gbuffersDesc,
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
			nullptr,
			IID_PPV_ARGS(this->gbuffers[i].GetAddressOf())
		));
	}

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = { };
	srvDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Texture2D.MipLevels = 1;
	
	this->core->dev->CreateShaderResourceView(
		this->gbuffers[0].Get(),
		&srvDesc,
		this->albedoCPUHandle
	);

	this->core->dev->CreateShaderResourceView(
		this->gbuffers[1].Get(),
		&srvDesc,
		this->normalCPUHandle
	);

	this->core->dev->CreateShaderResourceView(
		this->gbuffers[2].Get(),
		&srvDesc,
		this->posCPUHandle
	);

	srvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;

	/*this->core->dev->CreateShaderResourceView(
		this->core->zBuffer.Get(),
		&srvDesc,
		this->depthCPUHandle
	);
	*/

	D3D12_SAMPLER_DESC samplerDesc = { };
	samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	samplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
	samplerDesc.MinLOD = 0.f;
	samplerDesc.MaxLOD = D3D12_FLOAT32_MAX;
	samplerDesc.MaxAnisotropy = 1;
	samplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;

	this->samplerIndex = this->core->SAMPLER_AddDescriptorToCount();
	this->samplerCPUHandle = this->core->GetCPUDescriptorHeapHandle(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
	this->samplerGPUHandle = this->core->GetGPUDescriptorHeapHandle(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
	this->samplerIncrementSize = this->core->GetDescriptorHeapHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);

	this->samplerHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(this->samplerCPUHandle, this->samplerIndex, this->samplerIncrementSize);

	this->core->dev->CreateSampler(&samplerDesc, samplerHandle);
}

void ScreenQuad::InitPipeline() {
	CD3DX12_DESCRIPTOR_RANGE samplerRange;
	CD3DX12_DESCRIPTOR_RANGE albedoRange;
	CD3DX12_DESCRIPTOR_RANGE normalRange;
	CD3DX12_DESCRIPTOR_RANGE depthRange;
	CD3DX12_DESCRIPTOR_RANGE posRange;

	samplerRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, 1, 0, 0);
	albedoRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0);
	normalRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1, 0);
	depthRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 2, 0);
	posRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 3, 0);

	CD3DX12_ROOT_PARAMETER samplerParam;
	CD3DX12_ROOT_PARAMETER albedoParam;
	CD3DX12_ROOT_PARAMETER normalParam;
	CD3DX12_ROOT_PARAMETER depthParam;
	CD3DX12_ROOT_PARAMETER posParam;

	samplerParam.InitAsDescriptorTable(1, &samplerRange, D3D12_SHADER_VISIBILITY_PIXEL);
	albedoParam.InitAsDescriptorTable(1, &albedoRange, D3D12_SHADER_VISIBILITY_PIXEL);
	normalParam.InitAsDescriptorTable(1, &normalRange, D3D12_SHADER_VISIBILITY_PIXEL);
	depthParam.InitAsDescriptorTable(1, &depthRange, D3D12_SHADER_VISIBILITY_PIXEL);
	posParam.InitAsDescriptorTable(1, &posRange, D3D12_SHADER_VISIBILITY_PIXEL);

	D3D12_ROOT_PARAMETER rootParameters[] = {
		samplerParam,
		albedoParam,
		normalParam,
		depthParam,
		posParam
	};

	D3D12_ROOT_SIGNATURE_DESC rootSigDesc = { };
	rootSigDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	rootSigDesc.NumParameters = _countof(rootParameters);
	rootSigDesc.pParameters = rootParameters;
	rootSigDesc.NumStaticSamplers = 0;
	rootSigDesc.pStaticSamplers = nullptr;

	ComPtr<ID3DBlob> rsBlob, rsErr;
	ThrowIfFailed(D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1, rsBlob.GetAddressOf(), rsErr.GetAddressOf()));

	if (rsErr) {
		MessageBox(this->core->hwnd, (char*)rsErr->GetBufferPointer(), "Error", MB_ICONERROR | MB_OK);
		return;
	}

	ThrowIfFailed(this->core->dev->CreateRootSignature(0, rsBlob->GetBufferPointer(), rsBlob->GetBufferSize(), IID_PPV_ARGS(this->rootSig.GetAddressOf())));

	D3D12_INPUT_ELEMENT_DESC elements[] = {
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, NULL},
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, NULL},
	};

	D3D12_INPUT_LAYOUT_DESC layout = { };
	layout.NumElements = _countof(elements);
	layout.pInputElementDescs = elements;

	ComPtr<ID3DBlob> VS, PS;
	this->shader->GetBlob(VS, PS);

	D3D12_GRAPHICS_PIPELINE_STATE_DESC plDesc = { };
	plDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	plDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
	plDesc.RTVFormats[0] = DXGI_FORMAT_B8G8R8A8_UNORM;
	plDesc.NumRenderTargets = 1;
	plDesc.VS = CD3DX12_SHADER_BYTECODE(VS.Get());
	plDesc.PS = CD3DX12_SHADER_BYTECODE(PS.Get());
	plDesc.DepthStencilState.DepthEnable = FALSE;
	plDesc.DepthStencilState.StencilEnable = FALSE;
	plDesc.InputLayout = layout;
	plDesc.pRootSignature = this->rootSig.Get();
	plDesc.SampleDesc.Count = 1;
	plDesc.SampleMask = UINT32_MAX;
	plDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	plDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

	ThrowIfFailed(this->core->dev->CreateGraphicsPipelineState(&plDesc, IID_PPV_ARGS(this->plState.GetAddressOf())));
	this->plState->SetName(L"ScreenQuad Pipeline");
}

void ScreenQuad::Render() {
	std::vector<D3D12_RESOURCE_BARRIER> barriers;
	for (ComPtr<ID3D12Resource> buffer : this->gbuffers) {
		D3D12_RESOURCE_BARRIER resourceBarrier = CD3DX12_RESOURCE_BARRIER::Transition(buffer.Get(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RESOLVE_DEST);
		barriers.push_back(resourceBarrier);
	}

	for (ComPtr<ID3D12Resource> buffer : this->core->gbuffers) {
		D3D12_RESOURCE_BARRIER resourceBarrier = CD3DX12_RESOURCE_BARRIER::Transition(buffer.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_RESOLVE_SOURCE);
		barriers.push_back(resourceBarrier);
	}

	D3D12_RESOURCE_BARRIER depthBarrier = CD3DX12_RESOURCE_BARRIER::Transition(this->core->zBuffer.Get(), D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	barriers.push_back(depthBarrier);

	this->core->list->ResourceBarrier(barriers.size(), barriers.data());

	for (int i = 0; i < _countof(this->gbuffers); i++) {
		this->core->list->ResolveSubresource(this->gbuffers[i].Get(), 0, this->core->gbuffers[i].Get(), 0, DXGI_FORMAT_B8G8R8A8_UNORM);
	}

	barriers.clear();

	for (ComPtr<ID3D12Resource> buffer : this->gbuffers) {
		D3D12_RESOURCE_BARRIER resourceBarrier = CD3DX12_RESOURCE_BARRIER::Transition(buffer.Get(), D3D12_RESOURCE_STATE_RESOLVE_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
		barriers.push_back(resourceBarrier);
	}

	for (ComPtr<ID3D12Resource> buffer : this->core->gbuffers) {
		D3D12_RESOURCE_BARRIER resourceBarrier = CD3DX12_RESOURCE_BARRIER::Transition(buffer.Get(), D3D12_RESOURCE_STATE_RESOLVE_SOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);
		barriers.push_back(resourceBarrier);
	}

	depthBarrier = CD3DX12_RESOURCE_BARRIER::Transition(this->core->zBuffer.Get(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_DEPTH_WRITE);
	barriers.push_back(depthBarrier);

	this->core->list->ResourceBarrier(barriers.size(), barriers.data());

	this->core->list->IASetVertexBuffers(0, 1, &this->vbView);
	this->core->list->SetGraphicsRootSignature(this->rootSig.Get());
	this->core->list->SetPipelineState(this->plState.Get());
	this->core->list->IASetIndexBuffer(&this->IBOView);
	this->core->list->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	this->albedoGPUHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(this->cbv_srvGPUHandle, this->albedoIndex, this->cbv_srvIncrementSize);
	this->normalGPUHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE (this->cbv_srvGPUHandle, this->normalIndex, this->cbv_srvIncrementSize);
	this->depthGPUHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(this->cbv_srvGPUHandle, this->depthIndex, this->cbv_srvIncrementSize);
	this->posGPUHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE (this->cbv_srvGPUHandle, this->posIndex, this->cbv_srvIncrementSize);
	this->samplerStateGPUHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(this->samplerGPUHandle, this->samplerIndex, this->samplerIncrementSize);

	ID3D12DescriptorHeap* heaps[] = {
		this->core->cbv_srvHeap.Get(),
		this->core->samplerHeap.Get()
	};

	this->core->list->SetDescriptorHeaps(_countof(heaps), heaps);
	this->core->list->SetGraphicsRootDescriptorTable(0, this->samplerGPUHandle);
	this->core->list->SetGraphicsRootDescriptorTable(1, this->albedoGPUHandle);
	this->core->list->SetGraphicsRootDescriptorTable(2, this->normalGPUHandle);
	this->core->list->SetGraphicsRootDescriptorTable(3, this->depthGPUHandle);
	this->core->list->SetGraphicsRootDescriptorTable(4, this->posGPUHandle);
	
	this->core->list->DrawIndexedInstanced(6, 1, 0, 0, 0);

	return;
}