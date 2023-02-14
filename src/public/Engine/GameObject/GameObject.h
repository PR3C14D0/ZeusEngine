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

	ComPtr<ID3D12PipelineState> plState;

	ComPtr<ID3D12Resource> wvpBuff;
	D3D12_GPU_DESCRIPTOR_HANDLE wvpGPUHandle;
	D3D12_CPU_DESCRIPTOR_HANDLE wvpCPUHandle;
	UINT alignedWVPSize;

	ComPtr<ID3D12RootSignature> rootSig;


	bool bLoaded;

	void InitPipeline();
	void UpdateConstantBuffers();

	int width, height;

	Shader* shader;

	WVP wvp;

public:
	Transform transform;
	GameObject(std::string name);

	void LoadModel(std::string fileName);

	void Init();
	void Render();
};