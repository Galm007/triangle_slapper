// triangle rasterizer:
// https://joshbeam.com/articles/triangle_rasterization/

#ifndef __TRIANGLE_H__
#define __TRIANGLE_H__

#include "color.h"

struct Triangle
{
	float x1, y1, x2, y2, x3, y3;
	struct Color color1, color2, color3;
};

void draw_triangle(
	struct Color* canvas,
	unsigned canvas_width,
	struct Triangle* tri
);

#endif
