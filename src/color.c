#include "color.h"

Color color_add(Color a, Color b)
{
	return (Color){
		.r = a.r + b.r,
		.g = a.g + b.g,
		.b = a.b + b.b
	};
}

Color color_sub(Color a, Color b)
{
	return (Color){
		.r = a.r - b.r,
		.g = a.g - b.g,
		.b = a.b - b.b
	};
}

Color color_scale(Color a, float f)
{
	return (Color){
		.r = a.r * f,
		.g = a.g * f,
		.b = a.b * f
	};
}

