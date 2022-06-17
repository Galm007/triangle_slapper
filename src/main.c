#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include "fast_rand.h"
#include "genetic_algorithm.h"
#include "config.h"

#define THREAD_TRIS ((POPULATION - (POPULATION % THREADS)) / THREADS)

struct ScoringData
{
	unsigned thread_index;

	unsigned width, height;
	unsigned char* input_img;
	struct Color* current_img;

	unsigned tri_count;
	struct Triangle* triangles;
	double* scores;
};

void* calculate_scores(void* args)
{
	struct ScoringData* data = (struct ScoringData*)args;

	int start = data->tri_count * data->thread_index;

	struct Color* test_img = (struct Color*) calloc(
		data->width * data->height, sizeof(struct Color));

	for (int t = start; t < start + data->tri_count; ++t)
	{
		data->scores[t] = triangle_score(
			&data->triangles[t],
			data->input_img, data->current_img, test_img,
			data->width, data->height);
	}

	free(test_img);
	test_img = NULL;

	pthread_exit(NULL);
}

void write_img(char* name, struct Color* img, unsigned width, unsigned height)
{
	unsigned char* output_img = (unsigned char*) calloc(
		3 * width * height, 1);

	for (unsigned i = 0; i < width * height; ++i)
	{
		output_img[i * 3    ] = (unsigned char) img[i].r;
		output_img[i * 3 + 1] = (unsigned char) img[i].g;
		output_img[i * 3 + 2] = (unsigned char) img[i].b;
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
	fast_srand(time(0));

	// load input image
	int width, height, channels;
	unsigned char* input_img = stbi_load(
		argv[1], &width, &height, &channels, 3);
	if (!input_img)
	{
		fprintf(stderr, "Failed to load image!\n");
		exit(1);
	}
	
	// create empty image
	struct Color* result = (struct Color*) calloc(
		width * height, sizeof(struct Color));

	// genetic algorithm
	struct Triangle tris[POPULATION];
	double scores[POPULATION];

	for (unsigned k = 0; k < MAX_ITERATIONS; ++k)
	{
		for (int i = 0; i < POPULATION; ++i)
			triangle_init_random(tris + i, width, height);

		for (int g = 0; g < GENERATIONS; ++g)
		{
			pthread_t threads[THREADS];
			struct ScoringData thread_data[THREADS];

			// start threads
			for (int i = 0; i < THREADS; ++i)
			{
				struct ScoringData* data = &thread_data[i];
				data->thread_index = i;
				data->width = width;
				data->height = height;
				data->input_img = input_img;
				data->current_img = result;
				data->tri_count = THREAD_TRIS;
				data->triangles = tris;
				data->scores = scores;

				pthread_create(
					&threads[i],
					NULL,
					calculate_scores,
					data);
			}
			for (int i = 0; i < THREADS; ++i)
				pthread_join(threads[i], NULL);

			// sort triangles based on scores using bubble sort
			// https://www.geeksforgeeks.org/bubble-sort/
			for (int i = 0; i < POPULATION - 1; i++)
				for (int j = 0; j < POPULATION - i - 1; j++)
					if (scores[j] < scores[j + 1])
					{
						float ftmp = scores[j];
						scores[j] = scores[j + 1];
						scores[j + 1] = ftmp;
						
						struct Triangle ttmp = tris[j];
						tris[j] = tris[j + 1];
						tris[j + 1] = ttmp;
					}

			// create the next generation of triangles
			// based on the best triangles of this generation
			if (g < GENERATIONS - 1)
				for (int i = BEST_CUTOFF; i < POPULATION; ++i)
				{
					tris[i] = tris[i % BEST_CUTOFF];
					triangle_mutate(
						tris + i,
						width, height);
				}
		}

		// restart the iteration if there is no improvement
		if (scores[0] <= 0.0)
		{
			printf(
				"Iteration %i did not improve! "
				"restarting iteration...\n", k--);
			continue;
		}

		draw_triangle(result, width, tris);

		// output the current generated image so far
		char filename[16];
		sprintf(filename, "output_%i.png", k);
		write_img(filename, result, width, height);
		printf("output_%i.png -- score %f\n", k, scores[0]);
	}

	/*
	 * if progress images are created, then
	 * simply rename the most recent one
	 *
	 * otherwise, output the final image
	 */

	// deallocate stuff
	stbi_image_free(input_img);
	input_img = NULL;
	free(result);
	result = NULL;

	return 0;
}
