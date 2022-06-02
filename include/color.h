#ifndef __COLOR_H__
#define __COLOR_H__

struct Color { float r, g, b; };

struct Color color_add(struct Color a, struct Color b);
struct Color color_sub(struct Color a, struct Color b);
struct Color color_scale(struct Color a, float f);

#endif
