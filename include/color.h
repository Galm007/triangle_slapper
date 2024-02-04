#ifndef __COLOR_H__
#define __COLOR_H__

typedef struct { float r, g, b; } Color;

Color color_add(Color a, Color b);
Color color_sub(Color a, Color b);
Color color_scale(Color a, float f);

Color* img_load(char* filename, int* width, int* height);
void img_free(Color* dest);

#endif
