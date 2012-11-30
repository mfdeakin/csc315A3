
#ifndef _PNGLOADER_H_
#define _PNGLOADER_H_

/* Just some code I had from before */

#include <stdlib.h>
#include <GL/gl.h>
#include <GL/glut.h>
#include <png.h>

struct image
{
	unsigned width;
	unsigned height;
	void *data;
	unsigned colorFmt;
	unsigned bits;
	GLuint texture;
};

void initGL(void);
void render(void);
void resize(int width, int height);
GLuint createTexture(struct image *img);
struct image *readPNG(char *filename);

#endif
