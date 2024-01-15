// triangle rasterizer:
// https://joshbeam.com/articles/triangle_rasterization/

#include "triangle.h"

typedef struct {
	Color color1, color2;
	int x1, y1, x2, y2;
} Edge;

static void edge_init(
	Edge* edge,
	Color color1, int x1, int y1,
	Color color2, int x2, int y2
) {
	if (y1 < y2)
	{
		edge->color1 = color1;
		edge->x1 = x1;
		edge->y1 = y1;
		edge->color2 = color2;
		edge->x2 = x2;
		edge->y2 = y2;
	}
	else
	{
		edge->color1 = color2;
		edge->x1 = x2;
		edge->y1 = y2;
		edge->color2 = color1;
		edge->x2 = x1;
		edge->y2 = y1;
	}
}

struct Span
{
	Color color1, color2;
	int x1, x2;
};

static void span_init(
	struct Span* span,
	Color color1, int x1,
	Color color2, int x2
) {
	if (x1 < x2)
	{
		span->color1 = color1;
		span->x1 = x1;
		span->color2 = color2;
		span->x2 = x2;
	}
	else
	{
		span->color1 = color2;
		span->x1 = x2;
		span->color2 = color1;
		span->x2 = x1;
	}
}

static void draw_span(
	Color* canvas,
	unsigned canvas_width,
	const struct Span* span,
	int y
) {
	int xdiff = span->x2 - span->x1;
	if (xdiff == 0)
		return;
	
	Color colordiff = color_sub(span->color2, span->color1);
	float factor = 0.0f;
	float factor_step = 1.0f / (float)xdiff;
	
	for (int x = span->x1; x < span->x2; ++x)
	{
		Color color = color_add(
			span->color1, color_scale(colordiff, factor));
		
		canvas[y * canvas_width + x] = color;
		factor += factor_step;
	}
}

static void draw_spans_between_edges(
	Color* canvas,
	unsigned canvas_width,
	const Edge* e1,
	const Edge* e2
) {
	float e1_ydiff = (float)(e1->y2 - e1->y1);
	float e2_ydiff = (float)(e2->y2 - e2->y1);
	
	if (e1_ydiff == 0.0f || e2_ydiff == 0.0f)
		return;
	
	float e1_xdiff = (float)(e1->x2 - e1->x1);
	float e2_xdiff = (float)(e2->x2 - e2->x1);
	Color e1_colordiff = color_sub(e1->color2, e1->color1);
	Color e2_colordiff = color_sub(e2->color2, e2->color1);
	
	float factor1 = (float)(e2->y1 - e1->y1) / e1_ydiff;
	float factor_step1 = 1.0f / e1_ydiff;
	float factor2 = 0.0f;
	float factor_step2 = 1.0f / e2_ydiff;
	
	for (int y = e2->y1; y < e2->y2; ++y)
	{
		struct Span span;
		span_init(
			&span,
			color_add(
				e1->color1,
				color_scale(e1_colordiff, factor1)
			),
			e1->x1 + (int)(e1_xdiff * factor1),
			color_add(
				e2->color1,
				color_scale(e2_colordiff, factor2)
			),
			e2->x1 + (int)(e2_xdiff * factor2)
		);
		draw_span(canvas, canvas_width, &span, y);
		
		factor1 += factor_step1;
		factor2 += factor_step2;
	}
}

void draw_triangle(
	Color* canvas,
	int canvas_width,
	Triangle* tri
) {
	// calculate edges
	Edge edges[3];
	edge_init(
		edges,
		tri->color1, (int)tri->x1, (int)tri->y1,
		tri->color2, (int)tri->x2, (int)tri->y2
	);
	edge_init(
		edges + 1,
		tri->color2, (int)tri->x2, (int)tri->y2,
		tri->color3, (int)tri->x3, (int)tri->y3
	);
	edge_init(
		edges + 2,
		tri->color3, (int)tri->x3, (int)tri->y3,
		tri->color1, (int)tri->x1, (int)tri->y1
	);
	
	// find edge with the greatest length in the y axis
	int max_length = 0, long_edge = 0;
	for (int i = 0; i < 3; ++i)
	{
		int length = edges[i].y2 - edges[i].y1;
		if (length > max_length)
		{
			max_length = length;
			long_edge = i;
		}
	}
	
	// fill the triangle
	int short_edge1 = (long_edge + 1) % 3;
	int short_edge2 = (long_edge + 2) % 3;
	draw_spans_between_edges(
		canvas,
		canvas_width,
		edges + long_edge,
		edges + short_edge1
	);
	draw_spans_between_edges(
		canvas,
		canvas_width,
		edges + long_edge,
		edges + short_edge2
	);
}
