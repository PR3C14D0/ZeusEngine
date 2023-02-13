#pragma once
#include <iostream>
#include "Math/Transform.h"

/* Forward declarations */
class Core;
/* End:Forward declarations */

class GameObject {
private:
	std::string name;

	Core* core;
public:
	Transform transform;
	GameObject(std::string name);

	void Render();
};