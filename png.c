#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <png.h>
#include <magic.h>
#include <sys/types.h>
#include <sys/stat.h>

int PNG_GetSize(char *filename, unsigned int *width,
	unsigned int *height, unsigned int *pixel_size) {

	*width = 0;
	*height = 0;
	if (pixel_size != NULL)
		*pixel_size = 0;
	
	FILE *fp = fopen (filename, "rb");
	if (fp == NULL) {
		fprintf (stderr, "pix2html:%s:%s() error: Cannot open %s: %s\n",
		__FILE__, __FUNCTION__, filename, strerror(errno));
		return errno;
	}

	png_structp png = png_create_read_struct (PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!png) {
		fprintf (stderr, "pix2html:%s:%s() error: png_create_read_struct() failed\n", 
			__FILE__, __FUNCTION__);
		return 1;
	}

	png_infop info = png_create_info_struct (png);
	if (!info) {
		fprintf (stderr, "pix2html:%s:%s() error: png_create_info_struct() failed\n",
			__FILE__, __FUNCTION__);
		return 1;
	}

	if (setjmp(png_jmpbuf(png))) {
		fprintf (stderr, "pix2html:%s:%s() error: setjmp(png_jmpbuf(png)) failed\n",
			__FILE__, __FUNCTION__);
		return 1;
	}

	png_init_io (png, fp);
	png_read_info (png, info);
	
	*width = png_get_image_width(png, info);
	*height = png_get_image_height(png, info);
	if (pixel_size != NULL)
		*pixel_size = (unsigned int)png_get_channels(png, info);

	png_destroy_read_struct (&png, &info, NULL);
	fclose (fp);
	return 0;
}

