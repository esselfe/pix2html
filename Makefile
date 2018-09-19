
CFLAGS = -std=c11 -Wall -Werror -O2 -g -D_GNU_SOURCE
LDFLAGS = -lpixdim -lmagic
PROGNAME = pix2html.x

.PHONY: default all clean

default: all

all: $(PROGNAME)

$(PROGNAME): pix2html.c
	gcc $(CFLAGS) $(LDFLAGS) pix2html.c -o $(PROGNAME)

clean:
	@rm -v $(PROGNAME) 2>/dev/null || true

