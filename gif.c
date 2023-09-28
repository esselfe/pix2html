#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <gif_lib.h>

int GIF_GetSize(char *filename, unsigned int *width,
	unsigned int *height, unsigned int *pixel_size) {
	int errorcode = 0;
	GifFileType *gifptr = DGifOpenFileName(filename, &errorcode);
	if (errorcode != 0) {
		printf("pix2html error: %s: %s\n", filename, GifErrorString(errorcode));
		*width = 0;
		*height = 0;
		if (pixel_size != NULL)
			*pixel_size = 0;
		
		return 1;
	}
	
	*width = gifptr->SWidth;
	*height = gifptr->SHeight;
	if (pixel_size != NULL) {
		if (gifptr->SColorMap != NULL)
			*pixel_size = gifptr->SColorMap->BitsPerPixel;
		else
			*pixel_size = 0;
	}
	
	DGifCloseFile(gifptr, &errorcode);
	return 0;
}

