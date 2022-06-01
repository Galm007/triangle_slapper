#include "color.h"

struct Color color_add(struct Color a, struct Color b)
{
	struct Color out = {
		.r = a.r + b.r,
		.g = a.g + b.g,
		.b = a.b + b.b
	};
	return out;
}

struct Color color_sub(struct Color a, struct Color b)
{
	struct Color out = {
		.r = a.r - b.r,
		.g = a.g - b.g,
		.b = a.b - b.b
	};
	return out;
}

struct Color color_scale(struct Color a, float f)
{
	struct Color out = {
		.r = a.r * f,
		.g = a.g * f,
		.b = a.b * f
	};
	return out;
}

