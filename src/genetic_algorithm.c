#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "triangle.h"
#include "genetic_algorithm.h"

#include "config.h"

#define RANDF(n) ((float) (rand() % (n)))
#define ABS(n)   ((n) < 0.0f ? -(n) : (n))

void triangle_init_random(
	struct Triangle* tri,
	unsigned scr_width,
	unsigned scr_height
) {
	tri->x1 = RANDF(scr_width );
	tri->y1 = RANDF(scr_height);
	tri->x2 = RANDF(scr_width );
	tri->y2 = RANDF(scr_height);
	tri->x3 = RANDF(scr_width );
	tri->y3 = RANDF(scr_height);
#ifdef INTERPOLATED_TRIANGLES
	tri->color1.r = RANDF(256);
	tri->color1.g = RANDF(256);
	tri->color1.b = RANDF(256);
	tri->color2.r = RANDF(256);
	tri->color2.g = RANDF(256);
	tri->color2.b = RANDF(256);
	tri->color3.r = RANDF(256);
	tri->color3.g = RANDF(256);
	tri->color3.b = RANDF(256);
#else
	struct Color clr = {
		.r = RANDF(256),
		.g = RANDF(256),
		.b = RANDF(256)
	};

	tri->color1 = clr;
	tri->color2 = clr;
	tri->color3 = clr;
#endif
}

void mutate_position_x(float* f, unsigned scr_width)
{
	*f += RANDF(MUTATION_AMOUNT_POS * 2 + 1) - MUTATION_AMOUNT_POS;
	*f = *f < 0.0f ? 0.0f : *f > scr_width ? scr_width : *f;
}

void mutate_position_y(float* f, unsigned scr_height)
{
	*f += RANDF(MUTATION_AMOUNT_POS * 2 + 1) - MUTATION_AMOUNT_POS;
	*f = *f < 0.0f ? 0.0f : *f > scr_height ? scr_height : *f;
}

void mutate_color(float* f)
{
	*f += RANDF(MUTATION_AMOUNT_CLR * 2 + 1) - MUTATION_AMOUNT_CLR;
	*f = *f < 0.0f ? 0.0f : *f > 255.0f ? 255.0f : *f;
}

void triangle_mutate(
	struct Triangle* tri,
	unsigned scr_width,
	unsigned scr_height
) {
	mutate_position_x(&tri->x1, scr_width);
	mutate_position_y(&tri->y1, scr_height);
	mutate_position_x(&tri->x2, scr_width);
	mutate_position_y(&tri->y2, scr_height);
	mutate_position_x(&tri->x3, scr_width);
	mutate_position_y(&tri->y3, scr_height);
#ifdef INTERPOLATED_TRIANGLES
	mutate_color(&tri->color1.r);
	mutate_color(&tri->color1.g);
	mutate_color(&tri->color1.b);
	mutate_color(&tri->color2.r);
	mutate_color(&tri->color2.g);
	mutate_color(&tri->color2.b);
	mutate_color(&tri->color3.r);
	mutate_color(&tri->color3.g);
	mutate_color(&tri->color3.b);
#else
	struct Color clr = {
		.r = tri->color1.r,
		.g = tri->color1.g,
		.b = tri->color1.b
	};

	mutate_color(&clr.r);
	mutate_color(&clr.g);
	mutate_color(&clr.b);

	tri->color1 = clr;
	tri->color2 = clr;
	tri->color3 = clr;
#endif
}

static double image_score_at(
	unsigned char* target_img,
	struct Color* img,
	unsigned img_width,
	unsigned xmin,
	unsigned xmax,
	unsigned ymin,
	unsigned ymax
) {
	double score = 0.0;
	for (unsigned y = ymin; y < ymax; ++y)
		for (unsigned x = xmin; x < xmax; ++x)
		{
			const unsigned img_i = y * img_width + x;
			const unsigned target_i = img_i * 3;

			score += ABS(target_img[target_i] - img[img_i].r)
				+ ABS(target_img[target_i + 1] - img[img_i].g)
				+ ABS(target_img[target_i + 2] - img[img_i].b);
		}
	return 255.0f * (ymax - ymin) * (xmax - xmin) - score;
}

double triangle_score(
	struct Triangle* tri,
	unsigned char* target_img,
	struct Color* current_img,
	unsigned scr_width,
	unsigned scr_height)
{
	// draw triangle on a test image
	struct Color* test_img = malloc(
		scr_width * scr_height * sizeof(struct Color));
	memcpy(
		test_img,
		current_img,
		scr_width * scr_height * sizeof(struct Color)
	);
	draw_triangle(test_img, scr_width, tri);
	
	// calculate bounding box of triangle
	// https://stackoverflow.com/a/39974397
	
	int xmin, xmax, ymin, ymax;
	{
		int x1 = tri->x1, x2 = tri->x2, x3 = tri->x3;
		int y1 = tri->y1, y2 = tri->y2, y3 = tri->y3;

		xmax = x1 > x2 ? x1 > x3 ? x1 : x3 : x2 > x3 ? x2 : x3;
		ymax = y1 > y2 ? y1 > y3 ? y1 : y3 : y2 > y3 ? y2 : y3;
		xmin = x1 < x2 ? x1 < x3 ? x1 : x3 : x2 < x3 ? x2 : x3;
		ymin = y1 < y2 ? y1 < y3 ? y1 : y3 : y2 < y3 ? y2 : y3;
	}
	
	// calculate scores
	double test_score = image_score_at(
		target_img,
		test_img,
		scr_width,
		xmin,
		xmax,
		ymin,
		ymax
	);
	double current_score = image_score_at(
		target_img,
		current_img,
		scr_width,
		xmin,
		xmax,
		ymin,
		ymax
	);

	free(test_img);
	test_img = NULL;
	
	return test_score - current_score;
}
