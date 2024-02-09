#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "config.h"

/*
 * Command Line Arguments:
 * --threads=8		Threads
 * --thread-tris=50	Triangles per thread
 * --max-iter=1000	Max iterations
 * --generations=100	Generations per iteration
 * --cutoff=20		Best cutoff per generation
 * --max-pos-mut=30	Maximum position mutation between generations
 * --max-clr-mut=20	Maximum color mutation between generations
 * --resume-from=0      Resume previous run from nth iteration
 * --no-interpolate	Don't use interpolated triangle colors
 * --output-dir=.	Output directory
 * input.png		Input image
 */

#define BUFFER_SIZE 128

static bool copy_arg_value(char* arg, const char* arg_name, char* dest)
{
	int name_span = strcspn(arg, "=");

	if (name_span != strlen(arg_name)
		|| strncmp(arg, arg_name, name_span) != 0
		|| name_span + 2 > strlen(arg))
		return false;

	strncpy(dest, &arg[name_span + 1], BUFFER_SIZE);
	return true;
}

static bool parse_arg(
	char* arg,
	const char* arg_name,
	void* dest,
	bool floating_point
) {
	char value_buf[BUFFER_SIZE];
	char* end;

	if (!copy_arg_value(arg, arg_name, value_buf))
		return false;

	if (floating_point)
		*(float*)dest = strtod(value_buf, &end);
	else
		*(int*)dest = strtol(value_buf, &end, 10);

	if (end == value_buf)
	{
		fprintf(
			stderr,
			"Error: input value for %s is not a number\n",
			arg_name
		);
		return false;
	}
	return true;
}

static bool parse_flag(char* arg, const char* arg_name, bool* dest_value)
{
	if (strcmp(arg, arg_name) == 0)
	{
		*dest_value = true;
		return true;
	}
	return false;
}

bool config_init_w_args(Config* conf, int argc, char** argv)
{
	// parse input file (required)
	if (argc < 2)
	{
		fprintf(stderr, "Error: No argument provided\n");
		return false;
	}
	conf->input_img_path = argv[argc - 1];

	// default values
	conf->threads = 8;
	conf->thread_tris = 50;
	conf->max_iterations = 1000;
	conf->generations = 100;
	conf->best_cutoff = 20;
	conf->max_pos_mut = 30;
	conf->max_clr_mut = 20;
	conf->resume_from = 0;
	conf->no_interpolate = true;
	conf->output_dir = malloc(BUFFER_SIZE);
	conf->output_dir[0] = '.';
	conf->output_dir[1] = '\0';

	// parse arguments to override default values
	for (int i = 1; i < argc - 1; i++)
	{
		if (parse_arg(argv[i], "--threads", &conf->threads, false)
			|| parse_arg(argv[i], "--thread-tris", &conf->thread_tris, false)
			|| parse_arg(argv[i], "--max-iter", &conf->max_iterations, false)
			|| parse_arg(argv[i], "--generations", &conf->generations, false)
			|| parse_arg(argv[i], "--cutoff", &conf->best_cutoff, false)
			|| parse_arg(argv[i], "--max-pos-mut", &conf->max_pos_mut, true)
			|| parse_arg(argv[i], "--max-clr-mut", &conf->max_clr_mut, true)
			|| parse_arg(argv[i], "--resume-from", &conf->resume_from, false)
			|| parse_flag(argv[i], "--no-interpolate", &conf->no_interpolate)
			|| copy_arg_value(argv[i], "--output-dir", conf->output_dir)
		) {
			continue;
		}
		else
		{
			fprintf(
				stderr,
				"Error: failed to parse argument %s\n",
				argv[i]
			);
			return false;
		}
	}

	// get rid of the extra '/' at the end of the output directory
	int output_len = strlen(conf->output_dir);
	if (conf->output_dir[output_len - 1] == '/')
		conf->output_dir[output_len - 1] = '\0';

	return true;
}

void config_destroy(Config* conf)
{
	free(conf->output_dir);
	conf->output_dir = NULL;
}

void config_print(Config* conf)
{
	printf(
		"---------------\n"
		"threads:                        %d\n"
		"triangles per thread:           %d\n"
		"max iterations:                 %d\n"
		"generations per iteration:      %d\n"
		"best cutoff per generation:     %d\n"
		"maximum position mutation:      %f\n"
		"maximum color mutation:         %f\n"
		"resume from:                    %d\n"
		"interpolated triangles:         %s\n"
		"input image path:               %s\n"
		"output directory:               %s\n"
		"---------------\n",
		conf->threads,
		conf->thread_tris,
		conf->max_iterations,
		conf->generations,
		conf->best_cutoff,
		conf->max_pos_mut,
		conf->max_clr_mut,
		conf->resume_from,
		conf->no_interpolate ? "false" : "true",
		conf->input_img_path,
		conf->output_dir
	);
}
