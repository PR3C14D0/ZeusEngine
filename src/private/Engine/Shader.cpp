#include "Engine/Shader.h"
#include "Engine/Core.h"

Shader::Shader(WCHAR* fileName, const char* vertexShaderMethod, const char* pixelShaderMethod) {
	this->core = Core::GetInstance();
	this->core->GetHWND(this->hwnd);

	ComPtr<ID3DBlob> VSErr, PSErr;
	ThrowIfFailed(D3DCompileFromFile(fileName, nullptr, nullptr, vertexShaderMethod, "vs_5_1", NULL, NULL, this->VS.GetAddressOf(), VSErr.GetAddressOf()));
	ThrowIfFailed(D3DCompileFromFile(fileName, nullptr, nullptr, pixelShaderMethod, "ps_5_1", NULL, NULL, this->VS.GetAddressOf(), PSErr.GetAddressOf()));

	if (VSErr) 
		MessageBox(this->hwnd, (char*)VSErr->GetBufferPointer(), "Error", MB_OK | MB_ICONERROR);

	if (PSErr)
		MessageBox(this->hwnd, (char*)PSErr->GetBufferPointer(), "Error", MB_OK | MB_ICONERROR);

	return;
}

void Shader::GetBlob(ComPtr<ID3DBlob>& VS, ComPtr<ID3DBlob>& PS) {
	VS = this->VS;
	PS = this->PS;
}