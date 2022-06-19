#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "fast_rand.h"
#include "triangle.h"
#include "genetic_algorithm.h"

#include "config.h"

#define RANDF(n) ((float) (fast_rand() % (n)))
#define ABS(n)   ((n) < 0.0f ? -(n) : (n))

void triangle_init_random(
	struct Triangle* tri,
	unsigned scr_width,
	unsigned scr_height)
{
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
	unsigned scr_height)
{
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

double triangle_score(
	struct Triangle* tri,
	unsigned char* target_img,
	struct Color* current_img,
	struct Color* test_img,
	unsigned img_width,
	unsigned img_height)
{
	// draw triangle on a test image
	
	memcpy(
		test_img,
		current_img,
		img_width * img_height * sizeof(struct Color));
	
	draw_triangle(test_img, img_width, tri);
	
	// calculate bounding box of triangle
	// https://stackoverflow.com/a/39974397
	
	int sx1 = tri->x1;
	int sx2 = tri->x2;
	int sx3 = tri->x3;
	int sy1 = tri->y1;
	int sy2 = tri->y2;
	int sy3 = tri->y3;
	
	int xmax = sx1 > sx2 ? sx1 > sx3 ? sx1 : sx3 : sx2 > sx3 ? sx2 : sx3;
	int ymax = sy1 > sy2 ? sy1 > sy3 ? sy1 : sy3 : sy2 > sy3 ? sy2 : sy3;
	int xmin = sx1 < sx2 ? sx1 < sx3 ? sx1 : sx3 : sx2 < sx3 ? sx2 : sx3;
	int ymin = sy1 < sy2 ? sy1 < sy3 ? sy1 : sy3 : sy2 < sy3 ? sy2 : sy3;
	
	// calculate scores
	
	double test_score = 0.0, crnt_score = 0.0;
	
	for (unsigned y = ymin; y < ymax; ++y)
		for (unsigned x = xmin; x < xmax; ++x)
		{
			double test_diff, crnt_diff;
			
			const unsigned i = y * img_width + x;
			const unsigned j = i * 3;
			
			test_diff = ABS(target_img[j    ] - test_img[i].r)
			          + ABS(target_img[j + 1] - test_img[i].g)
			          + ABS(target_img[j + 2] - test_img[i].b);
			
			crnt_diff = ABS(target_img[j    ] - current_img[i].r)
			          + ABS(target_img[j + 1] - current_img[i].g)
			          + ABS(target_img[j + 2] - current_img[i].b);
			
			test_score += 765.0 - test_diff;
			crnt_score += 765.0 - crnt_diff;
		}
	
	return test_score - crnt_score;
}
