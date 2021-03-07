#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <getopt.h>
#include <dirent.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <magic.h>
#include <pixdim.h>

const char *pics2html_version_string = "0.1.13";

#define OPTION_NONE			0
#define OPTION_VERBOSE		1
unsigned int options;

const struct option long_options[] = {
	{"help", no_argument, NULL, 'h'},
	{"version", no_argument, NULL, 'V'},
	{"verbose", no_argument, NULL, 'v'},
	{NULL, 0, NULL, 0}
};
const char *short_options = "hVv";

void HelpShow(void) {
	printf("Usage: pix2html [ -h/--help | -V/--version | -v/--verbose ] DIRNAME\n");
}

void VersionShow(void) {
	printf("pix2html %s\n", pics2html_version_string);
}

int main(int argc, char **argv) {
	int c;
	while (1) {
		c = getopt_long(argc, argv, short_options, long_options, NULL);
		if (c == -1) break;
		switch (c) {
		case 'h':
			HelpShow();
			exit(0);
			break;
		case 'V':
			VersionShow();
			exit(0);
			break;
		case 'v':
			options |= OPTION_VERBOSE;
			break;
		default:
			fprintf(stderr, "Unknown option: %d / <%c>\n", c, (char)c);
			break;
		}
	}

	if (argc < 2) {
		HelpShow();
		exit(EINVAL);
	}

	// Retrieve directory to be parsed
	struct stat st;
	char *dirname = NULL;
	for (c = 1; c < argc; c++) {
		if (argv[c][0] == '-') continue;
		else {
			if (stat(argv[c], &st) < 0) {
				fprintf(stderr, "Cannot open %s: %s\n", argv[c], strerror(errno));
				exit(1);
			}
			if (st.st_mode & S_IFDIR) {
				dirname = argv[c];
				break;
			}
		}
	}

	// Create the output directory
	unsigned int pagedirlen = strlen(dirname)+strlen("-html");
	char *pagedir = malloc(pagedirlen+1);
	memset(pagedir, 0, pagedirlen+1);
	sprintf(pagedir, "%s-html", dirname);
	if (mkdir(pagedir, S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH) < 0) {
		if (errno != EEXIST) {
			fprintf(stderr, "Cannot create directory: %s\n", strerror(errno));
			exit(errno);
		}
	}

	// Open the directory and get the file count
	DIR *dir = opendir(dirname);
	if (dir == NULL) {
		fprintf(stderr, "Cannot open %s: %s\n", dirname, strerror(errno));
		exit(errno);
	}
	unsigned int pics_total = 0, page_total = 0;
	struct dirent *de;
	while (1) {
		de = readdir(dir);
		if (de == NULL)
			break;
		else if (strncmp(de->d_name, ".", 1) == 0) continue;
		else if (strncmp(de->d_name, "..", 2) == 0) continue;
		else
			++pics_total;
	}
	rewinddir(dir);

	magic_t mg = magic_open(MAGIC_MIME_TYPE);
	if (mg == NULL) {
		fprintf(stderr, "magic_open() failed: %s\n", magic_error(mg));
		exit(1);
	}
	magic_load(mg, NULL);

	unsigned int pagecnt = 1, pics_per_page = 100, picscnt = 0;
	page_total = pics_total / pics_per_page;
	if ((pics_total % pics_per_page) > 0)
		++page_total;
	char *fullname = (char *)malloc(4096);
	char *fullname_parent = (char *)malloc(4096);
	memset(fullname, 0, 4096);
	char *pagename = malloc(1024);
	memset(pagename, 0, 1024);
	sprintf(pagename, "page-%04u.html", pagecnt);
	char *pagefullname = malloc(1024);
	memset(pagefullname, 0, 1024);
	sprintf(pagefullname, "%s/%s", pagedir, pagename);
	char *pagenamenext = (char *)malloc(1024);
	sprintf(pagenamenext, "page-%04u.html", pagecnt+1);
	FILE *fp = fopen(pagefullname, "w+");
	if (fp == NULL) {
		fprintf(stderr, "Cannot open %s: %s\n", pagefullname, strerror(errno));
		exit(errno);
	}
	fprintf(fp, "<html>\n<head>\n<title>%s</title>\n</head>\n"
		"<style type=\"text/css\">\nbody {\n  background: #081018;\n"
		"  color: #a0a8b0;\n}\n</style>\n<body>\n<table>\n  <tr>\n",
		pagename);

	const char *mgstr;
	unsigned int width, height, depth, preview_width, preview_height;
	float ratio;
	int linecnt = -1;
	struct linespec {
		char name[1024], fullname[1024];
		unsigned int width, height, depth, size;
	} spec[4];
	while (1) {
		de = readdir(dir);
		if (de == NULL) break;
		else if (de->d_type != DT_REG) continue;
		else if (strncmp(de->d_name, ".", 1) == 0) continue;
		else if (strncmp(de->d_name, "..", 2) == 0) continue;

		++picscnt;

		sprintf(fullname, "%s/%s", dirname, de->d_name);
		sprintf(fullname_parent, "../%s/%s", dirname, de->d_name);
		mgstr = magic_file(mg, fullname);
		if (strncmp(mgstr, "image/png", 9) == 0)
			pixdimPNG_GetSize(fullname, &width, &height, &depth);
		else if (strncmp(mgstr, "image/jpeg", 10) == 0)
			pixdimJPG_GetSize(fullname, &width, &height, &depth);
		else {
			width = 1;
			height = 1;
		}
		
		if (width > height) {
			ratio = (float)width/(float)height;
			preview_width = 200;
			preview_height = 200/ratio;
		}
		else if (width < height) {
			ratio = (float)height/(float)width;
			preview_width = 200/ratio;
			preview_height = 200;
		}
		else {
			ratio = 1.0;
			preview_width = 200;
			preview_height = 200;
		}

		++linecnt;
		if (linecnt >= 4) {
			fprintf(fp, "  <td><a href=\"%s\">%s</a><br>%ux%u@%u<br>%u</td>\n",
				spec[0].fullname, spec[0].name, spec[0].width, spec[0].height,
				spec[0].depth, spec[0].size);
			fprintf(fp, "  <td><a href=\"%s\">%s</a><br>%ux%u@%u<br>%u</td>\n",
				spec[1].fullname, spec[1].name, spec[1].width, spec[1].height,
				spec[1].depth, spec[1].size);
			fprintf(fp, "  <td><a href=\"%s\">%s</a><br>%ux%u@%u<br>%u</td>\n",
				spec[2].fullname, spec[2].name, spec[2].width, spec[2].height,
				spec[2].depth, spec[2].size);
			fprintf(fp, "  <td><a href=\"%s\">%s</a><br>%ux%u@%u<br>%u</td>\n</tr><tr>\n",
				spec[3].fullname, spec[3].name, spec[3].width, spec[3].height,
				spec[3].depth, spec[3].size);
			linecnt = 0;
			memset(spec[0].name, 0, 1024);
			memset(spec[0].fullname, 0, 1024);
			spec[0].size = 0;
			memset(spec[1].name, 0, 1024);
			memset(spec[1].fullname, 0, 1024);
			spec[1].size = 0;
			memset(spec[2].name, 0, 1024);
			memset(spec[2].fullname, 0, 1024);
			spec[2].size = 0;
			memset(spec[3].name, 0, 1024);
			memset(spec[3].fullname, 0, 1024);
			spec[3].size = 0;
		}
		sprintf(spec[linecnt].name, "%s", de->d_name);
		sprintf(spec[linecnt].fullname, "%s", fullname_parent);
		spec[linecnt].width = width;
		spec[linecnt].height = height;
		spec[linecnt].depth = depth;
		stat(fullname, &st);
		spec[linecnt].size = st.st_size;

		fprintf(fp, "  <td><a href=\"../%s\"><img src=\"../%s\" width=\"%u\" height=\"%u\"/></a></td>\n", 
			fullname, fullname, preview_width, preview_height);
		
		if ((picscnt != 0 && (picscnt % pics_per_page) == 0) || picscnt == pics_total) {
			if (spec[0].size)
				fprintf(fp, "</tr><tr>\n  <td><a href=\"%s\">%s</a><br>%ux%u@%u<br>%u</td>\n",
					spec[0].fullname, spec[0].name, spec[0].width, spec[0].height,
					spec[0].depth, spec[0].size);
			if (spec[1].size)
				fprintf(fp, "  <td><a href=\"%s\">%s</a><br>%ux%u@%u<br>%u</td>\n",
					spec[1].fullname, spec[1].name, spec[1].width, spec[1].height,
					spec[1].depth, spec[1].size);
			if (spec[2].size)
				fprintf(fp, "  <td><a href=\"%s\">%s</a><br>%ux%u@%u<br>%u</td>\n",
					spec[2].fullname, spec[2].name, spec[2].width, spec[2].height,
					spec[2].depth, spec[2].size);
			if (spec[3].size)
				fprintf(fp, "  <td><a href=\"%s\">%s</a><br>%ux%u@%u<br>%u</td>\n</tr><tr>\n",
					spec[3].fullname, spec[3].name, spec[3].width, spec[3].height,
					spec[3].depth, spec[3].size);
			linecnt = 0;
			--linecnt;

			if (picscnt != pics_total) {
				sprintf(pagenamenext, "page-%04u.html", pagecnt+1);
				fprintf(fp, "  <td><a href=\"%s\">next</a></td></tr></table>\n"
					"<table width=\"100%%\"><tr><td>\n",
					pagenamenext);

				unsigned int cnt;
				for (cnt = 1; cnt <= page_total; cnt++)
					fprintf(fp, "<a href=\"page-%04u.html\">%u</a> ", cnt, cnt);

				fprintf(fp, "\n</td></tr>\n</table>\n</body>\n</html>");
				fclose(fp);

				sprintf(pagename, "page-%04u.html", ++pagecnt);
				sprintf(pagefullname, "%s/%s", pagedir, pagename);
				fp = fopen(pagefullname, "w+");
				if (fp == NULL) {
					fprintf(stderr, "Cannot open %s: %s\n", pagefullname, strerror(errno));
					exit(errno);
				}
				fprintf(fp, "<html>\n<head>\n<title>%s</title>\n</head>\n"
					"<style type=\"text/css\">\nbody {\n  background: #081018;\n"
					"  color: #a0a8b0;\n}\n</style>\n<body>\n<table>\n  <tr>\n",
					pagename);
			}
		}
		else if ((picscnt % 4) == 0)
			fprintf(fp, "</tr><tr>\n");

		if (options & OPTION_VERBOSE) {
			printf("\r%u/%.2f pages  %u/%u pictures", pagecnt, 
				(float)pics_total/(float)pics_per_page, picscnt, pics_total);
			fflush(stdout);
		}
	}

	fprintf(fp, "  </tr>\n</table>\n</body>\n</html>");

	fclose(fp);
	magic_close(mg);
	closedir(dir);

	if (options & OPTION_VERBOSE)
		printf("\n");
	
	return 0;
}

