#include "Engine/GameObject/GameObject.h"
#include "Engine/Core.h"

GameObject::GameObject(std::string name) {
	this->name = name;
	this->core = Core::GetInstance();
	this->core->GetDevice(this->dev, this->list);
	this->vertexSize = sizeof(vertex);
}

/*
	Loads a model to our object.
		Note: this must be called before initializing.
*/
void GameObject::LoadModel(std::string fileName) {
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(fileName.c_str(), NULL);
	aiMesh* mesh = scene->mMeshes[0]; // TODO: Support many meshes.

	for (int i = 0; i < mesh->mNumVertices; i++) {
		POSITION pos = { 0.f, 0.f, 0.f };
		UV uv = { 0.f, 0.f } ;
		POSITION normal = { 0.f, 0.f, 0.f };
		aiVector3D aiPos = mesh->mVertices[i];
		
		pos[0] = aiPos.x;
		pos[1] = aiPos.y;
		pos[2] = aiPos.z;

		if (mesh->HasTextureCoords(0)) {
			aiVector3D aiUV = mesh->mTextureCoords[0][i];
			uv[0] = aiUV.x;
			uv[1] = aiUV.y;
		}

		if (mesh->HasNormals()) {
			aiVector3D aiNormal = mesh->mNormals[i];
			normal[0] = aiNormal.x;
			normal[1] = aiNormal.y;
			normal[2] = aiNormal.z;
		}

		this->vertices.push_back(vertex { { pos[0], pos[1], pos[2] }, { uv[0], uv[1] }, { normal[0], normal[1], normal[2] } });
	}
}

void GameObject::Init() {
	D3D12_RESOURCE_DESC vbDesc = CD3DX12_RESOURCE_DESC::Buffer(this->vertexSize * this->vertices.size());
	D3D12_HEAP_PROPERTIES vbProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);

	ThrowIfFailed(
		this->dev->CreateCommittedResource(
			&vbProps,
			D3D12_HEAP_FLAG_NONE,
			&vbDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(this->vertexBuff.GetAddressOf())
		)
	);

	PUINT ms;
	this->vertexBuff->Map(NULL, nullptr, (void**)&ms);
	memcpy(ms, this->vertices.data(), this->vertices.size() * this->vertexSize);
	this->vertexBuff->Unmap(NULL, nullptr);
	ms = nullptr;

	this->vbView.BufferLocation = this->vertexBuff->GetGPUVirtualAddress();
	this->vbView.SizeInBytes = this->vertices.size() * this->vertexSize;
	this->vbView.StrideInBytes = this->vertexSize;
}

void GameObject::Render() {
	
	this->list->DrawInstanced(this->vertices.size(), 1, 0, 0);
}