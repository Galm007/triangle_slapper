#include <stdio.h>
#include "stb_image.h"
#include "color.h"

Color color_add(Color a, Color b)
{
	return (Color){
		.r = a.r + b.r,
		.g = a.g + b.g,
		.b = a.b + b.b
	};
}

Color color_sub(Color a, Color b)
{
	return (Color){
		.r = a.r - b.r,
		.g = a.g - b.g,
		.b = a.b - b.b
	};
}

Color color_scale(Color a, float f)
{
	return (Color){
		.r = a.r * f,
		.g = a.g * f,
		.b = a.b * f
	};
}

Color* img_load(char* filename, int* width, int* height)
{
	int w, h, channels;
	unsigned char* input_img = stbi_load(filename, &w, &h, &channels, 3);
	if (!input_img)
	{
		fprintf(stderr, "Failed to load image \"%s\"\n", filename);
		return NULL;
	}

	Color* color_img = malloc(w * h * sizeof(Color));
	for (int i = 0; i < w * h; i++)
	{
		color_img[i].r = input_img[i * 3];
		color_img[i].g = input_img[i * 3 + 1];
		color_img[i].b = input_img[i * 3 + 2];
	}

	if (width)
		*width = w;
	if (height)
		*height = h;

	stbi_image_free(input_img);
	return color_img;
}

void img_free(Color* dest)
{
	free(dest);
}
