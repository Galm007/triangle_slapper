#ifndef __CONFIG_H__
#define __CONFIG_H__

#include <stdbool.h>

typedef struct {
	int threads;
	int thread_tris;
	int max_iterations;
	int generations;
	int best_cutoff;
	float max_pos_mut;
	float max_clr_mut;
	bool no_interpolate;
	char* input_img_path;
	char* output_dir;
} Config;

bool config_init_w_args(Config* conf, int argc, char** argv);
void config_destroy(Config* conf);
void config_print(Config* conf);

#endif
