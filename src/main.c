#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include "fast_rand.h"
#include "genetic_algorithm.h"

int main(int argc, char* argv[])
{
	// seed for rand function
	fast_srand(time(0));
	
	// load input image
	int width, height, channels;
	unsigned char* input_img = stbi_load(
		"input.jpg",
		&width,
		&height,
		&channels,
		3);
	if (!input_img)
	{
		printf("Failed to load image!\n");
		exit(1);
	}
	
	// create empty image
	struct Color* result = (struct Color*) calloc(
		width * height,
		sizeof(struct Color));
	struct Color* test_img = (struct Color*) calloc(
		width * height,
		sizeof(struct Color));
	
	// genetic algorithm
	const unsigned tricnt = 1000; // triangle count per generation
	const unsigned gencnt = 30;  // generation count
	
	struct Triangle tris[tricnt];
	double scores[tricnt];

	for (unsigned k = 0; k < 10000; ++k)
	{
		for (unsigned i = 0; i < tricnt; ++i)
			triangle_init_random(tris + i, width, height);
		
		for (unsigned g = 0; g < gencnt; ++g)
		{
			// calculate scores
			for (int t = 0; t < tricnt; ++t)
				scores[t] = triangle_score(
						tris + t,
						input_img,
						result,
						test_img,
						width,
						height);
			
			// sort triangles based on scores using bubble sort
			// https://www.geeksforgeeks.org/bubble-sort/
			for (unsigned i = 0; i < tricnt - 1; i++)
			for (unsigned j = 0; j < tricnt - i - 1; j++)
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
			if (g < gencnt - 1)
				for (unsigned i = 15; i < tricnt; ++i)
				{
					tris[i] = tris[i % 15];
					triangle_mutate(
						tris + i,
						width,
						height);
				}
		}
		draw_triangle(result, width, tris);
		
		// write to image file
		
		unsigned char* output_img = (unsigned char*) calloc(
			3 * width * height, 1);
		
		for (unsigned i = 0; i < width * height; ++i)
		{
			output_img[i * 3    ] = (unsigned char) result[i].r;
			output_img[i * 3 + 1] = (unsigned char) result[i].g;
			output_img[i * 3 + 2] = (unsigned char) result[i].b;
		}
		
		char filename[16];
		sprintf(filename, "output_%i.png", k);
		stbi_write_png(
			filename,
			width,
			height,
			3,
			output_img,
			3 * width);
		printf("output_%i.png -- score %f\n", k, scores[0]);
		free(output_img);
	}
	
	// deallocate stuff
	stbi_image_free(input_img);
	input_img = NULL;
	free(result);
	result = NULL;
	free(test_img);
	test_img = NULL;
	
	return 0;
}
