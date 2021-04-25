#ifndef PIX2HTML_H
#define PIX2HTML_H 1

int JPG_GetSize(char *filename, unsigned int *width,
    unsigned int *height, unsigned int *pixel_size);
int PNG_GetSize(char *filename, unsigned int *width,
    unsigned int *height, unsigned int *pixel_size);

#endif /* PIX2HTML_H */
