#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "triangle.h"
#include "genetic_algorithm.h"

#include "config.h"

#define RANDF(n) ((float) (rand() % (n)))

void triangle_init_random(
	Triangle* tri,
	unsigned img_width,
	unsigned img_height
) {
	tri->x1 = RANDF(img_width );
	tri->y1 = RANDF(img_height);
	tri->x2 = RANDF(img_width );
	tri->y2 = RANDF(img_height);
	tri->x3 = RANDF(img_width );
	tri->y3 = RANDF(img_height);
	
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
	Color clr = {
		.r = RANDF(256),
		.g = RANDF(256),
		.b = RANDF(256)
	};

	tri->color1 = clr;
	tri->color2 = clr;
	tri->color3 = clr;
#endif
}

void mutate_position_x(float* f, unsigned img_width)
{
	*f += RANDF(MUTATION_AMOUNT_POS * 2 + 1) - MUTATION_AMOUNT_POS;
	*f = *f < 0.0f ? 0.0f : *f > img_width ? img_width : *f;
}

void mutate_position_y(float* f, unsigned img_height)
{
	*f += RANDF(MUTATION_AMOUNT_POS * 2 + 1) - MUTATION_AMOUNT_POS;
	*f = *f < 0.0f ? 0.0f : *f > img_height ? img_height : *f;
}

void mutate_color(float* f)
{
	*f += RANDF(MUTATION_AMOUNT_CLR * 2 + 1) - MUTATION_AMOUNT_CLR;
	*f = *f < 0.0f ? 0.0f : *f > 255.0f ? 255.0f : *f;
}

void triangle_mutate(
	Triangle* tri,
	unsigned img_width,
	unsigned img_height
) {
	mutate_position_x(&tri->x1, img_width);
	mutate_position_y(&tri->y1, img_height);
	mutate_position_x(&tri->x2, img_width);
	mutate_position_y(&tri->y2, img_height);
	mutate_position_x(&tri->x3, img_width);
	mutate_position_y(&tri->y3, img_height);
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
	Color clr = {
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
	Color* target_img,
	Color* img,
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
			const unsigned i = y * img_width + x;

			score += fabsf(target_img[i].r - img[i].r)
				+ fabsf(target_img[i].g - img[i].g)
				+ fabsf(target_img[i].b - img[i].b);
		}
	return 255.0f * (ymax - ymin) * (xmax - xmin) - score;
}

double triangle_score(
	Triangle* tri,
	Color* target_img,
	Color* current_img,
	unsigned img_width,
	unsigned img_height)
{
	// draw triangle on a test image
	Color* test_img = malloc(
		img_width * img_height * sizeof(Color));
	memcpy(
		test_img,
		current_img,
		img_width * img_height * sizeof(Color)
	);
	draw_triangle(test_img, img_width, tri);
	
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
		img_width,
		xmin,
		xmax,
		ymin,
		ymax
	);
	double current_score = image_score_at(
		target_img,
		current_img,
		img_width,
		xmin,
		xmax,
		ymin,
		ymax
	);

	free(test_img);
	test_img = NULL;
	
	return test_score - current_score;
}
