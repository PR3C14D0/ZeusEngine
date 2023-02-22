#include "Engine/ResourceManager.h"
#include "Engine/Core.h"

ResourceManager* ResourceManager::instance;

ResourceManager::ResourceManager() {
	this->core = Core::GetInstance();


}

ResourceManager* ResourceManager::GetInstance() {
	if (ResourceManager::instance == nullptr)
		ResourceManager::instance = new ResourceManager();
	return ResourceManager::instance;
}

bool ResourceManager::Exists(std::string name) {
	bool found = false;
	if (this->resources.count(name) > 0)
		found = true;
	return found;
}

void ResourceManager::GetResource(ComPtr<ID3D12Resource>& resource, std::string name) {
	if(this->Exists(name))
		resource = this->resources[name];
	return;
}

bool ResourceManager::AddResource(ComPtr<ID3D12Resource>& resource, std::string name) {
	if (this->Exists(name)) return false;

	this->resources[name] = resource;
	return true;
}

UINT ResourceManager::GetResourceIndex(std::string name) {
	UINT index = 0;
	if (this->Exists(name)) {
		for (std::pair<std::string, ComPtr<ID3D12Resource>> resPair : this->resources) {
			if (resPair.first == name) break;

			index++;
		}
	}
	return index;

}