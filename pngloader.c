
#include "pngloader.h"

#define DEBUG

#ifndef DEBUG
void debug(void *vp, char *cp, ...) {};
#else
#define debug fprintf
#endif

GLuint createTexture(struct image *i)
{
	glGenTextures(1, &i->texture);
	glBindTexture(GL_TEXTURE_2D, i->texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, i->width, i->height, 0,
				 i->colorFmt, GL_UNSIGNED_BYTE, i->data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
					GL_TEXTURE_MAX_LOD);
	return i->texture;
}

struct image *readPNG(char *fname)
{
	FILE *f = fopen(fname, "r");
	if(!f) {
		debug(stderr, "Failed to open %s\n", fname);
		return NULL;
	}
	fseek(f, 0, SEEK_END);
	if(ftell(f) <= 8) {
		debug(stderr, "%s is not a PNG\n", fname);
		return NULL;
	}
	fseek(f, 0, SEEK_SET);
	/* Check for the signature */
	void *sig = malloc(8);
	size_t check = fread(sig, 1, 8, f);
	if(check != 8 || png_sig_cmp(sig, 0, 8)) {
		debug(stderr, "%s is not a PNG\n", fname);
		return NULL;
	}
	/* Create datastructures used for reading the PNG */
	png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL,
											 NULL, NULL);
	if(!png) {
		debug(stderr, "Unable to create a png_struct for %s\n", fname);
		free(sig);
		return NULL;
	}
	png_infop info = png_create_info_struct(png);
	if(!info) {
		debug(stderr, "Unable to create an png_info struct for %s\n", fname);
		png_destroy_read_struct(&png, NULL, NULL);
		free(sig);
		return NULL;
	}
	/* Prepare to read the PNG */
	png_init_io(png, f);
	png_set_sig_bytes(png, 8);
	/* Read the PNG */
	png_read_png(png, info, PNG_TRANSFORM_STRIP_16 |
				 PNG_TRANSFORM_PACKING | PNG_TRANSFORM_EXPAND,
				 NULL);

	struct image *image = malloc(sizeof(struct image));
	image->width = png_get_image_width(png, info);
	image->height = png_get_image_height(png, info);
	image->bits = png_get_bit_depth(png, info);
	switch(png_get_color_type(png, info)) {
	case PNG_COLOR_TYPE_GRAY:
		image->colorFmt = GL_LUMINANCE;
		debug(stdout, "GL_LUMINANCE\n");
		break;
	case PNG_COLOR_TYPE_GRAY_ALPHA:
		image->colorFmt = GL_LUMINANCE_ALPHA;
		debug(stdout, "GL_LUMINANCE_ALPHA\n");
		break;
	case PNG_COLOR_TYPE_RGB:
		image->colorFmt = GL_RGB;
		debug(stdout, "GL_RGB\n");
		break;
	case PNG_COLOR_TYPE_RGB_ALPHA:
		image->colorFmt = GL_RGBA;
		debug(stdout, "GL_RGB_ALPHA\n");
		break;
	}
	unsigned int rowBytes = png_get_rowbytes(png, info);
	image->data = malloc(rowBytes * image->height);
	debug(stdout, "row bytes: %d\nwidth: %d\nheight: %d\n", rowBytes,
		  image->width, image->height);

	png_bytep *rows = png_get_rows(png, info);
	int i, j;
	for(i = 0; i < image->height; i++) {
		for(j = 0; j < image->width; j++) {
			/* There's some bug that I don't remember here,
			 * so I'm just leaving it and compensating in
			 * the UV map :(
			 * Actually I'm exploiting it for the roof color*/
			/* ((char *)img->data)[i * img->width * 2 + j * 2] = */
			/* 	~rows[img->height - i - 1][j * 2]; */
			/* ((char *)img->data)[i * img->width * 2 + j * 2 + 1] = */
			/* 	~rows[img->height - i - 1][j * 2 + 1]; */
			((char *)image->data)[i * image->width + j] =
				rows[image->height - i - 1][j];
		}
	}
	fclose(f);
	
	/* Cleanup, and that's a wrap */
	/* png_free(png, rows); */
	png_destroy_read_struct(&png, &info, NULL);
	return image;
}
