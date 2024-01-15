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
#include "config.h"

#define THREAD_TRIS ((POPULATION - (POPULATION % THREADS)) / THREADS)

// TODO:
// Use thread pools
// Add --continue argument

typedef struct {
	Color *current_img, *target_img;
	int width, height;

	Triangle* triangles;
	double* scores;
	int start_index;
} ScoringData;

void* calculate_scores(void* args)
{
	ScoringData* data = args;

	for (int i = 0; i < THREAD_TRIS; i++)
	{
		data->scores[data->start_index + i] = triangle_score(
			&data->triangles[data->start_index + i],
			data->target_img,
			data->current_img,
			data->width,
			data->height
		);
	}

	pthread_exit(NULL);
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

	// genetic algorithm
	Triangle tris[POPULATION];
	double scores[POPULATION];

	for (int k = 0; k < MAX_ITERATIONS; k++)
	{
		for (int i = 0; i < POPULATION; i++)
			triangle_init_random(&tris[i], width, height);

		for (int g = 0; g < GENERATIONS; g++)
		{
			pthread_t threads[THREADS];
			ScoringData thread_data[THREADS];

			// start threads
			for (int i = 0; i < THREADS; i++)
			{
				ScoringData* data = &thread_data[i];
				data->current_img = result;
				data->target_img = target_img;
				data->width = width;
				data->height = height;
				data->triangles = tris;
				data->scores = scores;
				data->start_index = THREAD_TRIS * i;

				if (pthread_create(
					&threads[i],
					NULL,
					calculate_scores,
					data
				))
					printf(
						"Failed to create thread %d!\n",
						i
					);
			}
			for (int i = 0; i < THREADS; i++)
				if (pthread_join(threads[i], NULL))
					printf(
						"Failed to create join %d!\n",
						i
					);


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
	free(result);
	free(target_img);

	return 0;
}
