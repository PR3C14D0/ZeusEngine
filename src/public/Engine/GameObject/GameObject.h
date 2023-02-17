#pragma once
#include <iostream>
#include "Math/Transform.h"
#include <directx/d3d12.h>
#include <directx/d3dx12.h>
#include <wrl.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "Engine/Vertex.h"
#include "Util/Util.h"
#include "Engine/WVP.h"
#include "Engine/Shader.h"
#include <wincodec.h>
#include <dxtk/WICTextureLoader.h>
#include <dxtk/ResourceUploadBatch.h>

using namespace Microsoft::WRL;

/* Forward declarations */
class Core;
/* End:Forward declarations */

class GameObject {
private:
	std::string name;

	Core* core;

	std::vector<vertex> vertices;
	UINT vertexSize;

	ComPtr<ID3D12Resource> vertexBuff;
	D3D12_VERTEX_BUFFER_VIEW vbView;

	ComPtr<ID3D12Resource> texture;
	D3D12_GPU_DESCRIPTOR_HANDLE textureGPUHandle;
	D3D12_CPU_DESCRIPTOR_HANDLE textureCPUHandle;
	UINT textureIndex;

	ComPtr<ID3D12PipelineState> plState;

	D3D12_CPU_DESCRIPTOR_HANDLE cbv_srvCPUHandle;
	D3D12_GPU_DESCRIPTOR_HANDLE cbv_srvGPUHandle;

	D3D12_CPU_DESCRIPTOR_HANDLE samplerCPUHandle;
	D3D12_GPU_DESCRIPTOR_HANDLE samplerGPUHandle;
	UINT samplerIncrementSize;

	D3D12_CPU_DESCRIPTOR_HANDLE texSamplerCPUHandle;
	D3D12_GPU_DESCRIPTOR_HANDLE texSamplerGPUHandle;
	UINT texSamplerIndex;

	ComPtr<ID3D12Resource> wvpBuff;
	D3D12_GPU_DESCRIPTOR_HANDLE wvpGPUHandle;
	D3D12_CPU_DESCRIPTOR_HANDLE wvpCPUHandle;
	UINT alignedWVPSize;
	UINT cbv_srvIncrementSize;

	ComPtr<ID3D12RootSignature> rootSig;

	bool bLoaded;

	void InitPipeline();
	void UpdateConstantBuffers();

	int width, height;

	Shader* shader;

	WVP wvp;

public:
	Transform transform;
	explicit GameObject(std::string name);

	void LoadModel(std::string fileName);

	virtual void Init();
	virtual void Render();
};