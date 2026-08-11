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

// Sum, over the pixels the triangle covers, of how much closer to the target
// the canvas gets by painting it: positive means an improvement. Equivalent to
// drawing onto a copy of `current` and comparing per-pixel distance to
// `target`, but without allocating or copying the canvas.
double triangle_score_delta(
	Triangle* tri,
	const Color* target,
	const Color* current,
	int canvas_width
);

#endif
