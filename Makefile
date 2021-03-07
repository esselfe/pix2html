
CFLAGS = -std=c11 -Wall -Werror -O2 -g -D_GNU_SOURCE
LDFLAGS = -lpixdim -lmagic
PROGNAME = pix2html

.PHONY: default all clean

default: all

all: $(PROGNAME)

$(PROGNAME): pix2html.c
	gcc $(CFLAGS) pix2html.c -o $(PROGNAME) $(LDFLAGS)

clean:
	@rm -v $(PROGNAME) 2>/dev/null || true

