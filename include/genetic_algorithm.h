#ifndef __GENETIC_ALGORITHM_H__
#define __GENETIC_ALGORITHM_H__

#include "triangle.h"

void triangle_init_random(
	struct Triangle* tri,
	unsigned scr_width,
	unsigned scr_height
);

void triangle_mutate(
	struct Triangle* tri,
	unsigned scr_width,
	unsigned scr_height
);

double triangle_score(
	struct Triangle* tri,
	unsigned char* target_img,
	struct Color* current_img,
	unsigned scr_width,
	unsigned scr_height
);

#endif
