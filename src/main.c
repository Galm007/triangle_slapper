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

/* TODO:
 * - Dynamically adjust mutation amounts
 * - Allow loading config files
 * - Add --continue option
 * - Print errors to stderr instead of stdout
 * - Add an option to use OpenCL for gpu accelerated triangle scoring
 */

typedef struct {
	Color *current_img, *target_img;
	int width, height;

	Triangle* triangles;
	double* scores;

	Config* conf;
} ScoringData;

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
	Config conf;
	if (!config_init_w_args(&conf, argc, argv))
		return 1;
	config_print(&conf);

	// seed for rand function
	srand(time(0));

	// load input image
	Color* target_img;
	int width, height;
	{
		int channels;
		unsigned char* input_img = stbi_load(
			conf.input_img_path,
			&width,
			&height,
			&channels,
			3
		);
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
	int population = conf.threads * conf.thread_tris;
	Triangle tris[population];
	double scores[population];

	ScoringData data;
	data.current_img = result;
	data.target_img = target_img;
	data.width = width;
	data.height = height;
	data.triangles = tris;
	data.scores = scores;
	data.conf = &conf;

	// initialize threads
	ThreadSync ts;
	if (!threadsync_init(&ts, calculate_scores, &data, conf.threads))
	{
		fprintf(
			stderr,
			"Failed to initialize ThreadSync. Exiting...\n"
		);
		return 1;
	}

	// genetic algorithm
	for (int k = 0; k < conf.max_iterations; k++)
	{
		for (int i = 0; i < population; i++)
			triangle_init_random(&tris[i], width, height);

		for (int g = 0; g < conf.generations; g++)
		{
			threadsync_dispatch(&ts);
			threadsync_wait(&ts);

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

			// create the next generation of triangles
			// based on the best triangles of this generation
			if (g == conf.generations - 1)
				break;
			for (int i = conf.best_cutoff; i < population; i++)
			{
				tris[i] = tris[i % conf.best_cutoff];
				triangle_mutate(tris + i, width, height, &conf);
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
		sprintf(filename, "%s/output_%i.png", conf.output_dir, k);
		printf("Iteration %i -- score %f\n", k, scores[0]);

		draw_triangle(result, width, tris);
		write_img(filename, result, width, height);
	}

	// cleanup
	if (!threadsync_destroy(&ts))
	{
		fprintf(stderr, "Failed to destroy ThreadSync\n");
	}
	free(result);
	free(target_img);
	config_destroy(&conf);

	return 0;
}
