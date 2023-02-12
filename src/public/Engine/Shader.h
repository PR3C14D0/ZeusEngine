#pragma once
#include <iostream>
#include <directx/d3d12.h>
#include <d3dcompiler.h>
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
	Shader(const wchar_t* fileName, const char* vertexShaderMethod, const char* pixelShaderMethod);

	void GetBlob(ComPtr<ID3DBlob>& VS, ComPtr<ID3DBlob>& PS);
};