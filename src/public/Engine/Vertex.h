#pragma once

typedef float POSITION[3];
typedef float RGBA[4];
typedef float UV[2];

struct vertex {
	POSITION pos;
	UV uv;
	POSITION normal;
};

struct ScreenQuadVertex {
	POSITION pos;
};