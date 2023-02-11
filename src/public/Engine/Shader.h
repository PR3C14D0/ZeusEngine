#pragma once
#include <iostream>
#include <d3dcompiler.h>
#include <directx/d3d12.h>
#include "Util/Util.h"
#include <wrl.h>

using namespace Microsoft::WRL;

class Core;

class Shader {
private:
	Core* core;

	HWND hwnd;

	ComPtr<ID3DBlob> VS, PS;
public:
	Shader(WCHAR* fileName, const char* vertexShaderMethod, const char* pixelShaderMethod);

	void GetBlob(ComPtr<ID3DBlob>& VS, ComPtr<ID3DBlob>& PS);
};