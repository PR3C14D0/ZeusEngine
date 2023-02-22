#pragma once
#include <iostream>
#include <vector>
#include <directx/d3d12.h>
#include <directx/d3dx12.h>
#include <wrl.h>
#include "Engine/Vertex.h"
#include "Util/Util.h"
#include "Engine/Shader.h"
#include "Engine/WVP.h"

using namespace Microsoft::WRL;

class Core;

class ScreenQuad {
private:
	Core* core;

	ComPtr<ID3D12Resource> vertexBuff;
	D3D12_VERTEX_BUFFER_VIEW vbView;

	ComPtr<ID3D12Resource> IBO;
	D3D12_INDEX_BUFFER_VIEW IBOView;

	ComPtr<ID3D12RootSignature> rootSig;

	ComPtr<ID3D12PipelineState> plState;


	D3D12_CPU_DESCRIPTOR_HANDLE cbv_srvCPUHandle;
	D3D12_GPU_DESCRIPTOR_HANDLE cbv_srvGPUHandle;
	UINT cbv_srvIncrementSize;

	UINT albedoIndex;
	UINT normalIndex;
	UINT depthIndex;
	UINT posIndex;
	D3D12_CPU_DESCRIPTOR_HANDLE albedoCPUHandle;
	D3D12_CPU_DESCRIPTOR_HANDLE normalCPUHandle;
	D3D12_GPU_DESCRIPTOR_HANDLE albedoGPUHandle;
	D3D12_GPU_DESCRIPTOR_HANDLE normalGPUHandle;
	D3D12_CPU_DESCRIPTOR_HANDLE depthCPUHandle;
	D3D12_GPU_DESCRIPTOR_HANDLE depthGPUHandle;
	D3D12_CPU_DESCRIPTOR_HANDLE posCPUHandle;
	D3D12_GPU_DESCRIPTOR_HANDLE posGPUHandle;

	D3D12_CPU_DESCRIPTOR_HANDLE samplerCPUHandle;
	D3D12_GPU_DESCRIPTOR_HANDLE samplerGPUHandle;
	D3D12_GPU_DESCRIPTOR_HANDLE samplerStateGPUHandle;
	UINT samplerIndex;
	UINT samplerIncrementSize;
	D3D12_CPU_DESCRIPTOR_HANDLE samplerHandle;

	int width, height;

	Shader* shader;

	void InitPipeline();
public:
	ScreenQuad();
	void Render();
};