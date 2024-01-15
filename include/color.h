#ifndef __COLOR_H__
#define __COLOR_H__

typedef struct { float r, g, b; } Color;

Color color_add(Color a, Color b);
Color color_sub(Color a, Color b);
Color color_scale(Color a, float f);

#endif
