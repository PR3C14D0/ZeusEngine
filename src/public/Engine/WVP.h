#pragma once
#include <directx/DirectXMath.h>

using namespace DirectX;

struct WVP {
	XMMATRIX World;
	XMMATRIX View;
	XMMATRIX Projection;
};