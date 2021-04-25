#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <magic.h>
#include <jpeglib.h>

int JPG_GetSize(char *filename, unsigned int *width, 
	unsigned int *height, unsigned int *pixel_size) {
	struct stat jpg_stat;
	int jpg_fd, row_stride, bmp_size;
	unsigned long jpg_size;
	unsigned char *jpg_buffer, *bmp_buffer;
	struct jpeg_decompress_struct jinfo;
	struct jpeg_error_mgr jerr;

	*width = 0;
	*height = 0;
	if (pixel_size != NULL)
		*pixel_size = 0;

	if (stat(filename, &jpg_stat)) {
		fprintf (stderr, "pix2html:%s:%s() error: Cannot open %s: %s\n",
			__FILE__, __FUNCTION__, filename, strerror(errno));
		return errno;
	}
	jpg_size = jpg_stat.st_size;
	jpg_buffer = malloc(jpg_size + 100);

	jpg_fd = open(filename, O_RDONLY);
	off_t jpg_bytes_read = read(jpg_fd, jpg_buffer, jpg_size);
	++jpg_bytes_read;
	--jpg_bytes_read;
	close(jpg_fd);

	jinfo.err = jpeg_std_error(&jerr);
	jpeg_create_decompress(&jinfo);
	jpeg_mem_src(&jinfo, jpg_buffer, jpg_size);
	jpeg_read_header(&jinfo, 1);
	jpeg_start_decompress(&jinfo);

	*width = (unsigned int)jinfo.output_width;
	*height = (unsigned int)jinfo.output_height;
	if (pixel_size != NULL)
		*pixel_size = jinfo.output_components;

	// libjpeg.so complains if there are no scanline read, so make everyone happy
	row_stride = (*width) * (*pixel_size);
	bmp_size = (*width) * (*height) * jinfo.output_components;
	bmp_buffer = malloc(bmp_size);
	while (jinfo.output_scanline < jinfo.output_height) {
		unsigned char *buffer_array[1];
		buffer_array[0] = bmp_buffer + (jinfo.output_scanline) * row_stride;
		jpeg_read_scanlines(&jinfo, buffer_array, 1);
	}
	
	jpeg_finish_decompress(&jinfo);
	jpeg_destroy_decompress(&jinfo);
	free(jpg_buffer);
	free(bmp_buffer);

	return 0;
}

