#include "Engine/GameObject/GameObject.h"
#include "Engine/Core.h"

GameObject::GameObject(std::string name) {
	this->name = name;
	this->core = Core::GetInstance();
	this->vertexSize = sizeof(vertex);
	this->bLoaded = false;
	this->core->GetWindowSize(this->width, this->height);
	this->cbv_srvCPUHandle = this->core->GetCPUDescriptorHeapHandle(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	this->cbv_srvGPUHandle = this->core->GetGPUDescriptorHeapHandle(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	this->cbv_srvIncrementSize = this->core->GetDescriptorHeapHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	this->samplerCPUHandle = this->core->GetCPUDescriptorHeapHandle(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
	this->samplerGPUHandle = this->core->GetGPUDescriptorHeapHandle(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
	this->samplerIncrementSize = this->core->GetDescriptorHeapHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
	this->time = Time::GetInstance();
	this->resMgr = ResourceManager::GetInstance();
}

/*
	Loads a model to our object.
		Note: this must be called before initializing.
*/
void GameObject::LoadModel(std::string fileName) {
	if (this->bLoaded) return;
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(fileName.c_str(), NULL);
	if (!scene) {
		MessageBox(this->core->hwnd, "File not found", "Error", MB_OK | MB_ICONERROR);
		return;
	}

	for (int index = 0; index < scene->mNumMeshes; index++) {
		aiMesh* mesh = scene->mMeshes[index];

		std::vector<vertex> vertices;
		for (int i = 0; i < mesh->mNumVertices; i++) {
			
			POSITION pos = { 0.f, 0.f, 0.f };
			UV uv = { 0.f, 0.f };
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

			vertices.push_back(vertex{ { pos[0], pos[1], pos[2] }, { uv[0], uv[1] }, { normal[0], normal[1], normal[2] } });
		}
		this->vertices[index] = vertices;

		aiString texPath;
		if (scene->HasTextures()) {
			if (!scene->HasMaterials()) return;

			aiMaterial* mat = scene->mMaterials[mesh->mMaterialIndex];
			if (mat->GetTextureCount(aiTextureType_DIFFUSE) > 0 && mat->GetTexture(aiTextureType_DIFFUSE, 0, &texPath) == AI_SUCCESS) {
				if (!this->resMgr->Exists(texPath.C_Str())) {
					ComPtr<ID3D12Resource> texture;

					const aiTexture* tex = scene->GetEmbeddedTexture(texPath.C_Str());
					ResourceUploadBatch batch(this->core->dev.Get());
					batch.Begin(D3D12_COMMAND_LIST_TYPE_DIRECT);
					ThrowIfFailed(CreateWICTextureFromMemory(this->core->dev.Get(), batch, (BYTE*)tex->pcData, tex->mWidth, texture.GetAddressOf()));
					batch.End(this->core->queue.Get());

					D3D12_SHADER_RESOURCE_VIEW_DESC texDesc = { };
					texDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
					texDesc.Texture2D.MipLevels = 1;
					texDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
					texDesc.Format = texture->GetDesc().Format;

					UINT textureIndex = this->core->CBV_SRV_AddDescriptorToCount();
					this->textureIndex.push_back(textureIndex);
					UINT texVecIndex = this->textureIndex.size() - 1;

					D3D12_CPU_DESCRIPTOR_HANDLE textureCPUHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(this->cbv_srvCPUHandle, this->textureIndex[texVecIndex], this->cbv_srvIncrementSize);
					D3D12_GPU_DESCRIPTOR_HANDLE textureGPUHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(this->cbv_srvGPUHandle, this->textureIndex[texVecIndex], this->cbv_srvIncrementSize);
					this->textureCPUHandle.push_back(textureCPUHandle);
					this->textureGPUHandle.push_back(textureGPUHandle);
					this->textures[index] = texture;	

					this->resMgr->AddResource(texture, texPath.C_Str());

					this->core->dev->CreateShaderResourceView(
						this->textures[index].Get(),
						&texDesc,
						this->textureCPUHandle[index]
					);
				}
				else {
					ComPtr<ID3D12Resource> res;
					this->resMgr->GetResource(res, texPath.C_Str());
					this->textures[index] = res;
					UINT resIndex = this->resMgr->GetResourceIndex(texPath.C_Str());
					this->textureCPUHandle.push_back(this->textureCPUHandle[resIndex]);
					this->textureGPUHandle.push_back(this->textureGPUHandle[resIndex]);
				}
			}
		}
	}
	this->texSamplerIndex = this->core->SAMPLER_AddDescriptorToCount();
	this->texSamplerCPUHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(this->samplerCPUHandle, this->texSamplerIndex, this->samplerIncrementSize);
	this->texSamplerGPUHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(this->samplerGPUHandle, this->texSamplerIndex, this->samplerIncrementSize);

	D3D12_SAMPLER_DESC samplerDesc = { };
	samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	samplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
	samplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.MinLOD = 0.f;
	samplerDesc.MaxLOD = D3D12_FLOAT32_MAX;
	
	this->core->dev->CreateSampler(&samplerDesc, this->samplerCPUHandle);

	this->bLoaded = true;
}

void GameObject::Init() {
	if (!this->bLoaded) return;
	for (int i = 0; i < this->vertices.size(); i++) {
		D3D12_RESOURCE_DESC vbDesc = CD3DX12_RESOURCE_DESC::Buffer(this->vertexSize * this->vertices[i].size());
		D3D12_HEAP_PROPERTIES vbProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);

		ComPtr<ID3D12Resource> vertexBuff;

		ThrowIfFailed(
			this->core->dev->CreateCommittedResource(
				&vbProps,
				D3D12_HEAP_FLAG_NONE,
				&vbDesc,
				D3D12_RESOURCE_STATE_GENERIC_READ,
				nullptr,
				IID_PPV_ARGS(vertexBuff.GetAddressOf())
			)
		);

		vertexBuff->SetName(L"Vertex buffer");

		PUINT ms;
		std::vector<vertex> vertices;
		vertices = this->vertices[i];
		vertexBuff->Map(NULL, nullptr, (void**)&ms);
		memcpy(ms, vertices.data(), this->vertices[i].size() * this->vertexSize);
		vertexBuff->Unmap(NULL, nullptr);
		ms = nullptr;

		D3D12_VERTEX_BUFFER_VIEW vbView;

		vbView.BufferLocation = vertexBuff->GetGPUVirtualAddress();
		vbView.SizeInBytes = this->vertices[i].size() * this->vertexSize;
		vbView.StrideInBytes = this->vertexSize;
		this->vertexBuffers[i] = vertexBuff;

		this->vbViews.push_back(vbView);
	}

	this->InitPipeline();
}

void GameObject::InitPipeline() {
	UINT wvpIndex = this->core->CBV_SRV_AddDescriptorToCount();

	this->wvpCPUHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(cbv_srvCPUHandle, wvpIndex, cbv_srvIncrementSize);
	this->wvpGPUHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(cbv_srvGPUHandle, wvpIndex, cbv_srvIncrementSize);

	this->alignedWVPSize = (sizeof(this->wvp) + 255) & ~255;
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

	CD3DX12_DESCRIPTOR_RANGE textureRange;
	CD3DX12_ROOT_PARAMETER textureParam;
	textureRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0);
	textureParam.InitAsDescriptorTable(1, &textureRange, D3D12_SHADER_VISIBILITY_PIXEL);

	CD3DX12_DESCRIPTOR_RANGE samplerRange;
	CD3DX12_ROOT_PARAMETER samplerParam;
	samplerRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, 1, 0, 0);
	samplerParam.InitAsDescriptorTable(1, &samplerRange, D3D12_SHADER_VISIBILITY_PIXEL);

	D3D12_ROOT_PARAMETER params[] = {
		wvpParam,
		textureParam,
		samplerParam
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

	this->shader = new Shader(L"GBufferPass.hlsl", "VertexMain", "PixelMain");
	ComPtr<ID3DBlob> VS, PS;
	this->shader->GetBlob(VS, PS);

	D3D12_GRAPHICS_PIPELINE_STATE_DESC plDesc = { };
	plDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	plDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	plDesc.InputLayout = layout;
	plDesc.VS = CD3DX12_SHADER_BYTECODE(VS.Get());
	plDesc.PS = CD3DX12_SHADER_BYTECODE(PS.Get());
	plDesc.RTVFormats[0] = DXGI_FORMAT_B8G8R8A8_UNORM;
	plDesc.RTVFormats[1] = DXGI_FORMAT_B8G8R8A8_UNORM;
	plDesc.RTVFormats[2] = DXGI_FORMAT_B8G8R8A8_UNORM;
	plDesc.NumRenderTargets = 3;
	plDesc.SampleDesc.Count = 8;
	plDesc.SampleMask = UINT32_MAX;
	plDesc.pRootSignature = this->rootSig.Get();
	plDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
	plDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	plDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	plDesc.DepthStencilState.DepthEnable = TRUE;
	plDesc.DepthStencilState.StencilEnable = FALSE;
	plDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

	ThrowIfFailed(this->core->dev->CreateGraphicsPipelineState(&plDesc, IID_PPV_ARGS(this->plState.GetAddressOf())));
	this->plState->SetName(L"Mesh Pipeline");
}

void GameObject::UpdateConstantBuffers() {
	/* Set our WVP structure */
	this->wvp = this->core->sceneMgr->GetActualScene()->actualCamera->GetTransform(); // Our camera transform. (We'll skip camera's World because it's going to only be an identity matrix)
	this->wvp.World = XMMatrixTranspose(XMMatrixIdentity());
	this->transform.rotate(0.f, 45.f * this->time->deltaTime, 0.f);
	this->wvp.World *= XMMatrixTranspose(XMMatrixTranslation(this->transform.location.x, this->transform.location.y, this->transform.location.z));
	this->wvp.World *= XMMatrixTranspose(XMMatrixRotationX(XMConvertToRadians(this->transform.rotation.x)));
	this->wvp.World *= XMMatrixTranspose(XMMatrixRotationY(XMConvertToRadians(this->transform.rotation.y)));
	this->wvp.World *= XMMatrixTranspose(XMMatrixRotationZ(XMConvertToRadians(this->transform.rotation.z)));

	/* Copy our WVP structure */
	PUINT ms = nullptr;
	this->wvpBuff->Map(NULL, nullptr, (void**)&ms);
	memcpy(ms, &this->wvp, this->alignedWVPSize);
	this->wvpBuff->Unmap(NULL, nullptr);
}

void GameObject::Render() {
	if (!this->bLoaded) return;
	//this->transform.rotate(0.f, 1.f, 0.f);
	this->UpdateConstantBuffers();
	this->core->list->SetPipelineState(this->plState.Get());
	this->core->list->SetGraphicsRootSignature(this->rootSig.Get());
	
	ID3D12DescriptorHeap* heaps[] = {
		this->core->cbv_srvHeap.Get(),
		this->core->samplerHeap.Get()
	};

	this->core->list->SetDescriptorHeaps(_countof(heaps), heaps);

	for (int i = 0; i < this->vertices.size(); i++) {
		this->core->list->IASetVertexBuffers(0, 1, &this->vbViews[i]);
		this->core->list->SetGraphicsRootDescriptorTable(0, this->wvpGPUHandle);
		this->core->list->SetGraphicsRootDescriptorTable(1, this->textureGPUHandle[i]);
		this->core->list->SetGraphicsRootDescriptorTable(2, this->samplerGPUHandle);
		this->core->list->DrawInstanced(this->vertices[i].size(), 1, 0, 0);

	}

	return;
}