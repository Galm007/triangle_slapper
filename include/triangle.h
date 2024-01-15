// triangle rasterizer:
// https://joshbeam.com/articles/triangle_rasterization/

#ifndef __TRIANGLE_H__
#define __TRIANGLE_H__

#include "color.h"

typedef struct {
	float x1, y1, x2, y2, x3, y3;
	Color color1, color2, color3;
} Triangle;

void draw_triangle(
	Color* canvas,
	int canvas_width,
	Triangle* tri
);

#endif
