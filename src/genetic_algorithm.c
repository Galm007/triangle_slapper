#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "config.h"
#include "genetic_algorithm.h"

#define RANDF(n) ((float) (rand() % (n)))
#define RANDF_RANGE(min, max) (RANDF((max - min) * 2) - (max - min))

void triangle_init_random(Triangle* tri, int img_width, int img_height)
{
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

static void mutate(float* f, int max_mutation, int max_val)
{
	*f += RANDF_RANGE(-max_mutation, max_mutation);
	*f = fminf(fmaxf(0.0f, *f), max_val);
}

void triangle_mutate(Triangle* tri, int img_width, int img_height)
{
	mutate(&tri->x1, MUTATION_AMOUNT_POS, img_width);
	mutate(&tri->y1, MUTATION_AMOUNT_POS, img_height);
	mutate(&tri->x2, MUTATION_AMOUNT_POS, img_width);
	mutate(&tri->y2, MUTATION_AMOUNT_POS, img_height);
	mutate(&tri->x3, MUTATION_AMOUNT_POS, img_width);
	mutate(&tri->y3, MUTATION_AMOUNT_POS, img_height);

#ifdef INTERPOLATED_TRIANGLES
	mutate(&tri->color1.r, MUTATION_AMOUNT_CLR, 255);
	mutate(&tri->color1.g, MUTATION_AMOUNT_CLR, 255);
	mutate(&tri->color1.b, MUTATION_AMOUNT_CLR, 255);
	mutate(&tri->color2.r, MUTATION_AMOUNT_CLR, 255);
	mutate(&tri->color2.g, MUTATION_AMOUNT_CLR, 255);
	mutate(&tri->color2.b, MUTATION_AMOUNT_CLR, 255);
	mutate(&tri->color3.r, MUTATION_AMOUNT_CLR, 255);
	mutate(&tri->color3.g, MUTATION_AMOUNT_CLR, 255);
	mutate(&tri->color3.b, MUTATION_AMOUNT_CLR, 255);
#else
	Color clr = {
		.r = tri->color1.r,
		.g = tri->color1.g,
		.b = tri->color1.b
	};

	mutate(&clr.r, MUTATION_AMOUNT_CLR, 255);
	mutate(&clr.g, MUTATION_AMOUNT_CLR, 255);
	mutate(&clr.b, MUTATION_AMOUNT_CLR, 255);

	tri->color1 = clr;
	tri->color2 = clr;
	tri->color3 = clr;
#endif
}

static double image_score_at(
	Color* target_img,
	Color* img,
	int img_width,
	int xmin,
	int xmax,
	int ymin,
	int ymax
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
	int img_width,
	int img_height)
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
