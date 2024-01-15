#ifndef __GENETIC_ALGORITHM_H__
#define __GENETIC_ALGORITHM_H__

#include "triangle.h"

void triangle_init_random(
	Triangle* tri,
	unsigned img_width,
	unsigned img_height
);

void triangle_mutate(
	Triangle* tri,
	unsigned img_width,
	unsigned img_height
);

double triangle_score(
	Triangle* tri,
	Color* target_img,
	Color* current_img,
	unsigned img_width,
	unsigned img_height
);

#endif
