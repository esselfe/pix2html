
CFLAGS = -std=c11 -Wall -O2 -g -D_GNU_SOURCE
LDFLAGS = -lmagic -ljpeg -lpng
PROGNAME = pix2html

.PHONY: default all clean

default: all

all: $(PROGNAME)

$(PROGNAME): pix2html.h pix2html.c jpg.c png.c
	gcc $(CFLAGS) jpg.c png.c pix2html.c -o $(PROGNAME) $(LDFLAGS)

clean:
	@rm -v $(PROGNAME) 2>/dev/null || true

