#pragma once
#include <iostream>
#include <directx/d3d12.h>
#include <dxtk/WICTextureLoader.h>
#include <wrl.h>
#include <map>

using namespace Microsoft::WRL;

class Core; // Forward declaration of our core class.

/*
	This class will be a manager for our resources (for avoiding repeated resources etc).
*/
class ResourceManager {
private:
	static ResourceManager* instance;

	Core* core;
	std::map<std::string, ComPtr<ID3D12Resource>> resources;
public:
	ResourceManager();
	static ResourceManager* GetInstance();

	bool Exists(std::string name);

	void GetResource(ComPtr<ID3D12Resource>& resource, std::string name);
	bool AddResource(ComPtr<ID3D12Resource>& resource, std::string name);
	UINT GetResourceIndex(std::string name);
};