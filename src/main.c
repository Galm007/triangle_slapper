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
 * - Adjust triangle opacity
 * - Use many other shapes other than triangles
 * - Add an option to use OpenCL for gpu accelerated triangle scoring
 */

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
	int width, height;
	Color* target_img = img_load(conf.input_img_path, &width, &height);
	if (!target_img)
		return 1;
	
	// create empty image
	int resume_from;
	Color* current_img;
	{
		int highest_iter = get_highest_iteration(conf.output_dir);
		resume_from = conf.resume_from == -1
			? highest_iter
			: conf.resume_from;

		if (resume_from > highest_iter)
		{
			fprintf(
				stderr,
				"Error: resuming from iteration greater than"
				" previous highest iteration (%d)\n"
				"Consider using --resume-from=-1 to start"
				" from the highest available iteration\n",
				highest_iter
			);
			return 1;
		}
		if (conf.max_iterations < resume_from)
		{
			fprintf(
				stderr,
				"Error: resuming from iteration"
				" greater than --max-iter\n"
			);
			return 1;
		}

		if (resume_from == 0)
		{
			current_img = calloc(width * height, sizeof(Color));
		}
		else
		{
			char filename[64];
			sprintf(
				filename,
				"%s/output_%i.png",
				conf.output_dir,
				resume_from
			);
			current_img = img_load(filename, NULL, NULL);
		}
	}

	// prepare thread data
	int population = conf.threads * conf.thread_tris;
	Triangle tris[population];
	double scores[population];

	ScoringData data;
	data.current_img = current_img;
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
	for (int k = resume_from + 1; k < conf.max_iterations; k++)
	{
	ITERATION_START:
		for (int i = 0; i < population; i++)
			triangle_init_random(&tris[i], width, height, &conf);

		for (int g = 0; ; g++)
		{
			threadsync_dispatch(&ts);
			threadsync_wait(&ts);

			sort_triangles(scores, tris, population);
			if (scores[0] <= 0.0f)
				goto ITERATION_START;

			if (g == conf.generations - 1)
				break;

			// create the next generation of triangles
			// based on the best triangles of this generation
			for (int i = conf.best_cutoff; i < population; i++)
			{
				tris[i] = tris[i % conf.best_cutoff];
				triangle_mutate(&tris[i], width, height, &conf);
			}
		}

		// draw best triangle of the iteration to result
		draw_triangle(current_img, width, tris);
		printf("Iteration %i -- score %f\n", k, scores[0]);

		// output the current generated image so far
		char filename[64];
		sprintf(filename, "%s/output_%i.png", conf.output_dir, k);
		write_img(filename, current_img, width, height);
	}

	// cleanup
	if (!threadsync_destroy(&ts))
		fprintf(stderr, "Failed to destroy ThreadSync\n");
	img_free(current_img);
	img_free(target_img);
	config_destroy(&conf);

	return 0;
}
