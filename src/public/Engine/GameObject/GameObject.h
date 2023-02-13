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

using namespace Microsoft::WRL;

/* Forward declarations */
class Core;
/* End:Forward declarations */

class GameObject {
private:
	std::string name;

	Core* core;

	ComPtr<ID3D12Device> dev;
	ComPtr<ID3D12GraphicsCommandList> list;

	std::vector<vertex> vertices;
	UINT vertexSize;

	ComPtr<ID3D12Resource> vertexBuff;
	D3D12_VERTEX_BUFFER_VIEW vbView;

	void InitPipeline();

public:
	Transform transform;
	GameObject(std::string name);

	void LoadModel(std::string fileName);

	void Init();
	void Render();
};