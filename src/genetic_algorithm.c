#include <stdlib.h>
#include <math.h>
#include "genetic_algorithm.h"

#define RANDF(n) ((float) (rand() % (n)))
#define RANDF_RANGE(min, max) (RANDF((max - min) * 2) - (max - min))

void calculate_scores(void* args, int thread_i)
{
	ScoringData* data = args;

	int start_index = data->conf->thread_tris * thread_i;
	for (int i = 0; i < data->conf->thread_tris; i++)
	{
		data->scores[start_index + i] = triangle_score(
			&data->triangles[start_index + i],
			data->target_img,
			data->current_img,
			data->width,
			data->height
		);
	}
}

void sort_triangles(double* scores, Triangle* tris, int population)
{
	// sort triangles based on scores using bubble sort
	// https://www.geeksforgeeks.org/bubble-sort/
	for (int i = 0; i < population - 1; i++)
		for (int j = 0; j < population - i - 1; j++)
			if (scores[j] < scores[j + 1])
			{
				float ftmp = scores[j];
				scores[j] = scores[j + 1];
				scores[j + 1] = ftmp;
				
				Triangle ttmp = tris[j];
				tris[j] = tris[j + 1];
				tris[j + 1] = ttmp;
			}
}

void triangle_init_random(Triangle* tri, int img_width, int img_height, Config* conf)
{
	tri->x1 = RANDF(img_width );
	tri->y1 = RANDF(img_height);
	tri->x2 = RANDF(img_width );
	tri->y2 = RANDF(img_height);
	tri->x3 = RANDF(img_width );
	tri->y3 = RANDF(img_height);
	
	if (conf->no_interpolate)
	{
		tri->color1.r = RANDF(256);
		tri->color1.g = RANDF(256);
		tri->color1.b = RANDF(256);
		tri->color2.r = RANDF(256);
		tri->color2.g = RANDF(256);
		tri->color2.b = RANDF(256);
		tri->color3.r = RANDF(256);
		tri->color3.g = RANDF(256);
		tri->color3.b = RANDF(256);
	}
	else
	{
		Color clr = {
			.r = RANDF(256),
			.g = RANDF(256),
			.b = RANDF(256)
		};

		tri->color1 = clr;
		tri->color2 = clr;
		tri->color3 = clr;
	}
}

static void mutate(float* f, int max_mutation, int max_val)
{
	*f += RANDF_RANGE(-max_mutation, max_mutation);
	*f = fminf(fmaxf(0.0f, *f), max_val);
}

void triangle_mutate(Triangle* tri, int img_width, int img_height, Config* conf)
{
	mutate(&tri->x1, conf->max_pos_mut, img_width);
	mutate(&tri->y1, conf->max_pos_mut, img_height);
	mutate(&tri->x2, conf->max_pos_mut, img_width);
	mutate(&tri->y2, conf->max_pos_mut, img_height);
	mutate(&tri->x3, conf->max_pos_mut, img_width);
	mutate(&tri->y3, conf->max_pos_mut, img_height);

	if (conf->no_interpolate)
	{
		mutate(&tri->color1.r, conf->max_pos_mut, 255);
		mutate(&tri->color1.g, conf->max_pos_mut, 255);
		mutate(&tri->color1.b, conf->max_pos_mut, 255);
		mutate(&tri->color2.r, conf->max_pos_mut, 255);
		mutate(&tri->color2.g, conf->max_pos_mut, 255);
		mutate(&tri->color2.b, conf->max_pos_mut, 255);
		mutate(&tri->color3.r, conf->max_pos_mut, 255);
		mutate(&tri->color3.g, conf->max_pos_mut, 255);
		mutate(&tri->color3.b, conf->max_pos_mut, 255);
	}
	else
	{
		Color clr = {
			.r = tri->color1.r,
			.g = tri->color1.g,
			.b = tri->color1.b
		};

		mutate(&clr.r, conf->max_clr_mut, 255);
		mutate(&clr.g, conf->max_clr_mut, 255);
		mutate(&clr.b, conf->max_clr_mut, 255);

		tri->color1 = clr;
		tri->color2 = clr;
		tri->color3 = clr;
	}
}

double triangle_score(
	Triangle* tri,
	Color* target_img,
	Color* current_img,
	int img_width,
	int img_height)
{
	// how much painting this triangle improves the canvas, summed over the
	// pixels it actually covers -- no full-image copy needed
	double delta = triangle_score_delta(
		tri, target_img, current_img, img_width);

	// triangle area, to bias selection toward larger triangles
	// https://stackoverflow.com/a/39974397
	int x1 = tri->x1, x2 = tri->x2, x3 = tri->x3;
	int y1 = tri->y1, y2 = tri->y2, y3 = tri->y3;
	int area = 0.5f * abs(x1 * (y2 - y3) + x2 * (y3 - y1) + x3 * (y1 - y2));

	return delta * area * area;
}
