#ifndef __GENETIC_ALGORITHM_H__
#define __GENETIC_ALGORITHM_H__

#include "triangle.h"

void triangle_init_random(
	Triangle* tri,
	int img_width,
	int img_height
);

void triangle_mutate(
	Triangle* tri,
	int img_width,
	int img_height
);

double triangle_score(
	Triangle* tri,
	Color* target_img,
	Color* current_img,
	int img_width,
	int img_height
);

#endif
