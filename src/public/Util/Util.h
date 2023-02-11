#pragma once
#include <iostream>
#include <Windows.h>

void ThrowIfFailed(HRESULT hr) {
	if (FAILED(hr))
		throw std::exception();
	return;
}