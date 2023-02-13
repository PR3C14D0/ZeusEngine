#include "Engine/GameObject/GameObject.h"
#include "Engine/Core.h"

GameObject::GameObject(std::string name) {
	this->name = name;
	this->core = Core::GetInstance();
}

void GameObject::Render() {

}