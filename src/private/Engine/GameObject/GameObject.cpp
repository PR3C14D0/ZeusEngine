#include "Engine/GameObject/GameObject.h"
#include "Engine/Core.h"

GameObject::GameObject(std::string name) {
	this->name = name;
	this->core = Core::GetInstance();
	this->vertexSize = sizeof(vertex);
	this->bLoaded = false;
	this->core->GetWindowSize(this->width, this->height);
}

/*
	Loads a model to our object.
		Note: this must be called before initializing.
*/
void GameObject::LoadModel(std::string fileName) {
	if (this->bLoaded) return;
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(fileName.c_str(), NULL);
	if (!scene)
	{
		MessageBox(this->core->hwnd, "File not found", "Error", MB_OK | MB_ICONERROR);
		return;
	}
	aiMesh* mesh = scene->mMeshes[0]; // TODO: Support many meshes.

	for (int i = 0; i < mesh->mNumVertices; i++) {
		POSITION pos = { 0.f, 0.f, 0.f };
		UV uv = { 0.f, 0.f } ;
		POSITION normal = { 0.f, 0.f, 0.f };
		aiVector3D aiPos = mesh->mVertices[i];
		
		pos[0] = aiPos.x;
		pos[1] = aiPos.y;
		pos[2] = aiPos.z;

		if (mesh->HasTextureCoords(0)) {
			aiVector3D aiUV = mesh->mTextureCoords[0][i];
			uv[0] = aiUV.x;
			uv[1] = aiUV.y;
		}

		if (mesh->HasNormals()) {
			aiVector3D aiNormal = mesh->mNormals[i];
			normal[0] = aiNormal.x;
			normal[1] = aiNormal.y;
			normal[2] = aiNormal.z;
		}

		this->vertices.push_back(vertex { { pos[0], pos[1], pos[2] }, { uv[0], uv[1] }, { normal[0], normal[1], normal[2] } });
	}

	this->bLoaded = true;
}

void GameObject::Init() {
	if (!this->bLoaded) return;
	D3D12_RESOURCE_DESC vbDesc = CD3DX12_RESOURCE_DESC::Buffer(this->vertexSize * this->vertices.size());
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

	this->vertexBuff->SetName(L"Vertex buffer");

	PUINT ms;
	this->vertexBuff->Map(NULL, nullptr, (void**)&ms);
	memcpy(ms, this->vertices.data(), this->vertices.size() * this->vertexSize);
	this->vertexBuff->Unmap(NULL, nullptr);
	ms = nullptr;

	this->vbView.BufferLocation = this->vertexBuff->GetGPUVirtualAddress();
	this->vbView.SizeInBytes = this->vertices.size() * this->vertexSize;
	this->vbView.StrideInBytes = this->vertexSize;

	this->InitPipeline();
}

void GameObject::InitPipeline() {
	this->wvp.World = XMMatrixTranspose(XMMatrixIdentity());
	this->wvp.View = XMMatrixTranspose(XMMatrixIdentity() * XMMatrixTranslation(0.f, 0.f, 2.f));
	this->wvp.Projection = XMMatrixTranspose(XMMatrixPerspectiveFovLH(XMConvertToRadians(90.f), ((float)this->width / (float)this->height), .01f, 300.f)); // TODO Create camera class

	UINT wvpIndex = this->core->CBV_SRV_AddDescriptorToCount();
	D3D12_CPU_DESCRIPTOR_HANDLE cbv_srvCPUDescriptorHeap = this->core->GetCPUDescriptorHeapHandle(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	D3D12_GPU_DESCRIPTOR_HANDLE cbv_srvGPUDescriptorHeap = this->core->GetGPUDescriptorHeapHandle(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	UINT cbv_srvIncrementSize = this->core->GetDescriptorHeapHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	this->wvpCPUHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(cbv_srvCPUDescriptorHeap, wvpIndex, cbv_srvIncrementSize);
	this->wvpGPUHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(cbv_srvGPUDescriptorHeap, wvpIndex, cbv_srvIncrementSize);

	UINT alignedWVPSize = (sizeof(this->wvp) + 255) & ~255;
	D3D12_RESOURCE_DESC wvpDesc = CD3DX12_RESOURCE_DESC::Buffer(alignedWVPSize);
	D3D12_HEAP_PROPERTIES wvpProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	ThrowIfFailed(this->core->dev->CreateCommittedResource(
		&wvpProps,
		D3D12_HEAP_FLAG_NONE,
		&wvpDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(this->wvpBuff.GetAddressOf())
	));

	PUINT ms;
	this->wvpBuff->Map(NULL, nullptr, (void**)&ms);
	memcpy(ms, &this->wvp, alignedWVPSize);
	this->wvpBuff->Unmap(NULL, nullptr);
	ms = nullptr;

	this->wvpBuff->SetName(L"WVP Constant Buffer");

	D3D12_CONSTANT_BUFFER_VIEW_DESC wvpViewDesc = { };
	wvpViewDesc.BufferLocation = this->wvpBuff->GetGPUVirtualAddress();
	wvpViewDesc.SizeInBytes = alignedWVPSize;

	this->core->dev->CreateConstantBufferView(&wvpViewDesc, wvpCPUHandle);

	CD3DX12_DESCRIPTOR_RANGE wvpRange;
	CD3DX12_ROOT_PARAMETER wvpParam;
	wvpRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0, 0);
	wvpParam.InitAsDescriptorTable(1, &wvpRange, D3D12_SHADER_VISIBILITY_VERTEX);

	D3D12_ROOT_SIGNATURE_DESC rootSigDesc = { };
	rootSigDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	rootSigDesc.NumParameters = 1;
	rootSigDesc.pParameters = &wvpParam;
	rootSigDesc.pStaticSamplers = nullptr;
	rootSigDesc.NumStaticSamplers = 0;

	ComPtr<ID3DBlob> rootSigBlob, rootSigErr;
	ThrowIfFailed(D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1, rootSigBlob.GetAddressOf(), rootSigErr.GetAddressOf()));

	if (rootSigErr) {
		MessageBox(this->core->hwnd, (char*)rootSigErr->GetBufferPointer(), "Error", MB_OK | MB_ICONERROR);
		return;
	}

	this->core->dev->CreateRootSignature(0, rootSigBlob->GetBufferPointer(), rootSigBlob->GetBufferSize(), IID_PPV_ARGS(this->rootSig.GetAddressOf()));

	D3D12_INPUT_ELEMENT_DESC elements[] = {
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, NULL},
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, NULL},
		{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, NULL},
	};

	D3D12_INPUT_LAYOUT_DESC layout = { };
	layout.NumElements = _countof(elements);
	layout.pInputElementDescs = elements;

	this->shader = new Shader(L"shader.fx", "VertexMain", "PixelMain");
	ComPtr<ID3DBlob> VS, PS;
	this->shader->GetBlob(VS, PS);

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
	plDesc.pRootSignature = this->rootSig.Get();
	plDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
	plDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	plDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	plDesc.DepthStencilState.DepthEnable = TRUE;
	plDesc.DepthStencilState.StencilEnable = FALSE;

	ThrowIfFailed(this->core->dev->CreateGraphicsPipelineState(&plDesc, IID_PPV_ARGS(this->plState.GetAddressOf())));
}

void GameObject::Render() {
	this->core->list->SetPipelineState(this->plState.Get());
	this->core->list->IASetVertexBuffers(0, 1, &this->vbView);
	this->core->list->SetGraphicsRootSignature(this->rootSig.Get());
	this->core->list->SetDescriptorHeaps(1, this->core->cbv_srvHeap.GetAddressOf());
	this->core->list->SetGraphicsRootDescriptorTable(0, this->wvpGPUHandle);
	this->core->list->DrawInstanced(this->vertices.size(), 1, 0, 0);
	return;
}