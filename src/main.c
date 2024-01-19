#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include "color.h"
#include "genetic_algorithm.h"
#include "thread_sync.h"
#include "config.h"

#define POPULATION (THREADS * THREAD_TRIS)

/* TODO:
 * - Implement a quadtree for the triangle vertices to set initial triangle colors
 * using downscaled versions of the target image
 * - Dynamically adjust mutation amounts
 * - Create a system for processing command line arguments
 * - Allow overriding config values with command line arguments
 * - Allow loading config files
 * - Add --continue option
 * - Add an option to use OpenCL for gpu accelerated triangle scoring
 */

typedef struct {
	Color *current_img, *target_img;
	int width, height;

	Triangle* triangles;
	double* scores;
} ScoringData;

void calculate_scores(void* args, int thread_i)
{
	ScoringData* data = args;

	int start_index = THREAD_TRIS * thread_i;
	for (int i = 0; i < THREAD_TRIS; i++)
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

void write_img(
	const char* name,
	Color* img,
	int width,
	int height
) {
	unsigned char* output_img = calloc(width * height, 3);

	for (int i = 0; i < width * height; i++)
	{
		output_img[i * 3    ] = (unsigned char)img[i].r;
		output_img[i * 3 + 1] = (unsigned char)img[i].g;
		output_img[i * 3 + 2] = (unsigned char)img[i].b;
	}

	stbi_write_png(name, width, height, 3, output_img, 3 * width);

	free(output_img);
	output_img = NULL;
}

int main(int argc, char* argv[])
{
	if (argc != 2)
	{
		fprintf(stderr, "Only one argument should be passed!\n");
		exit(1);
	}

	// seed for rand function
	srand(time(0));

	// load input image
	Color* target_img;
	int width, height;
	{
		int channels;
		unsigned char* input_img = stbi_load(
			argv[1], &width, &height, &channels, 3);
		if (!input_img)
		{
			fprintf(stderr, "Failed to load image!\n");
			exit(1);
		}

		target_img = malloc(width * height * sizeof(Color));
		for (int i = 0; i < width * height; i++)
		{
			target_img[i].r = input_img[i * 3];
			target_img[i].g = input_img[i * 3 + 1];
			target_img[i].b = input_img[i * 3 + 2];
		}

		stbi_image_free(input_img);
		input_img = NULL;
	}
	
	// create empty image
	Color* result = calloc(width * height, sizeof(Color));

	// prepare thread data
	Triangle tris[POPULATION];
	double scores[POPULATION];

	ScoringData data;
	data.current_img = result;
	data.target_img = target_img;
	data.width = width;
	data.height = height;
	data.triangles = tris;
	data.scores = scores;

	// initialize threads
	ThreadSync ts;
	threadsync_init(&ts, calculate_scores, &data, THREADS);

	// genetic algorithm
	for (int k = 0; k < MAX_ITERATIONS; k++)
	{
		for (int i = 0; i < POPULATION; i++)
			triangle_init_random(&tris[i], width, height);

		for (int g = 0; g < GENERATIONS; g++)
		{
			threadsync_dispatch(&ts);
			threadsync_wait(&ts);

			// sort triangles based on scores using bubble sort
			// https://www.geeksforgeeks.org/bubble-sort/
			for (int i = 0; i < POPULATION - 1; i++)
				for (int j = 0; j < POPULATION - i - 1; j++)
					if (scores[j] < scores[j + 1])
					{
						float ftmp = scores[j];
						scores[j] = scores[j + 1];
						scores[j + 1] = ftmp;
						
						Triangle ttmp = tris[j];
						tris[j] = tris[j + 1];
						tris[j + 1] = ttmp;
					}

			// create the next generation of triangles
			// based on the best triangles of this generation
			if (g < GENERATIONS - 1)
				for (int i = BEST_CUTOFF; i < POPULATION; i++)
				{
					tris[i] = tris[i % BEST_CUTOFF];
					triangle_mutate(
						tris + i,
						width,
						height
					);
				}
		}

		// restart the iteration if there is no improvement
		if (scores[0] <= 0.0)
		{
			printf(
				"Iteration %i did not improve! "
				"restarting iteration...\n",
				k--
			);
			continue;
		}

		// output the current generated image so far
		char filename[16];
		sprintf(filename, "output_%i.png", k);
		printf("output_%i.png -- score %f\n", k, scores[0]);

		draw_triangle(result, width, tris);
		write_img(filename, result, width, height);
	}

	// cleanup
	threadsync_destroy(&ts);
	free(result);
	free(target_img);

	return 0;
}
