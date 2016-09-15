// Define this to zero to enable assertions
#define NDEBUG 0

#include "pgm.h"
#include "filter.h"
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>

enum mode {
	none,
	sobel,
	canny
} mode;

typedef struct sobel_opts {
	char* output;
	char* hout;
	char* vout;
	char* above;
	char* below;
	uint16_t above_threashold;
	uint16_t below_threashold;
} sobel_opts;

typedef struct canny_opts {
	char* mout;
	char* above;
	char* doubleThreashold;
	double sigma;
	uint16_t upper_threashold;
	uint16_t lower_threashold;
} canny_opts;

static void run_sobel(pgm_info* pgm, sobel_opts* opt) {
	pgm_info* tmpImg = NULL;

	pgm_filter maskx;
	maskx.width = 3;
	maskx.height = 3;
	int8_t maskx_filter[] = {-1,0,1,-2,0,2,-1,0,1};
	maskx.filter = maskx_filter;
	int32_t* xImg = apply_filter(pgm, &maskx);
	if(opt && opt->hout) {
		tmpImg = create_pgm(pgm->width, pgm->height, pgm->maxval);
		convert_int_arr(tmpImg, xImg);
		save_pgm(tmpImg, opt->hout);
		free_pgm(tmpImg);
		tmpImg = NULL;
	}

	pgm_filter masky;
	masky.width = 3;
	masky.height = 3;
	int8_t masky_filter[] = {1,2,1,0,0,0,-1,-2,-1};
	masky.filter = masky_filter;
	int32_t* yImg = apply_filter(pgm, &masky);
	if(opt && opt->vout) {
		tmpImg = create_pgm(pgm->width, pgm->height, pgm->maxval);
		convert_int_arr(tmpImg, yImg);
		save_pgm(tmpImg, opt->vout);
		free_pgm(tmpImg);
		tmpImg = NULL;
	}

	calculate_mag(pgm, xImg, yImg);
	free(yImg);
	convert_int_arr(pgm, xImg);
	free(xImg);
	if(opt && opt->output) {
		save_pgm(pgm, opt->output);
	}

	if(opt && opt->above) {
		tmpImg = copy_pgm(pgm);
		aboveThreashold(tmpImg, opt->above_threashold);
		save_pgm(tmpImg, opt->above);
		free_pgm(tmpImg);
		tmpImg = NULL;
	}
	
	if(opt && opt->below) {
		tmpImg = copy_pgm(pgm);
		belowThreashold(tmpImg, opt->below_threashold);
		save_pgm(tmpImg, opt->below);
		free_pgm(tmpImg);
		tmpImg = NULL;
	}
}

static uint16_t getUpperCuttoff(pgm_info* pgm, canny_opts* opt) {
	uint64_t cuttoff = ABS((double)pgm->height * pgm->width * opt->sigma);
	uint16_t x, y, val;

	for(val = pgm->maxval; val >= 0; val--) {
		for(x = 0; x <= pgm->width; x++) {
			for(y = 0; y <= pgm->height; y++) {
				if(getpixel(pgm, x, y) == val) {
					cuttoff--;
				}

				if(cuttoff == 0) {
					return val;
				}
			}
		}
	}
	return 0;
}

static void follow_edge(pgm_info* pgm, canny_opts* opt, int8_t* alpha, int32_t* tmp, uint16_t x, uint16_t y) {
	uint16_t xt, yt;

	for(xt = x - 1; xt <= x + 1; xt++) {
		for(yt = y -1; yt <= y + 1; yt++) {
			if((xt == x && yt == y) || xt < 0 || xt >= pgm->width || yt < 0 || yt >= pgm->height) {
				continue;
			}

			if(getpixel(pgm, xt, yt) > opt->lower_threashold && 
			   tmp[yt * pgm->width + xt] == pgm->maxval / 2 &&
			   ABS(alpha[yt * pgm->width + xt] - alpha[yt * pgm->width + xt]) < 22) {
				tmp[yt * pgm->width + xt] = pgm->maxval;
				follow_edge(pgm, opt, alpha, tmp, xt, yt);
			}
		}
	}
}

static int32_t* search_edge(pgm_info* pgm, canny_opts* opt, int8_t* alpha) {
	int32_t* tmp = calloc(sizeof(*tmp), pgm->height * pgm->width);
	uint16_t x, y;

	for(x = 1; x < pgm->width - 1; x++) {
		for(y = 1; y < pgm->height - 1; y++) {
			if(getpixel(pgm, x, y) >= opt->upper_threashold) {
				tmp[y * pgm->width + x] = pgm->maxval;
			}
			else if(getpixel(pgm, x, y) > opt->lower_threashold) {
				tmp[y * pgm->width + x] = pgm->maxval / 2;
			}
		}
	}

	for(x = 1; x < pgm->width - 1; x++) {
		for(y = 1; y < pgm->height - 1; y++) {
			if(getpixel(pgm, x, y) >= opt->upper_threashold) {
				tmp[y * pgm->width + x] = pgm->maxval;
				follow_edge(pgm, opt, alpha, tmp, x, y);
			}
		}
	}

	for(x = 1; x < pgm->width - 1; x++) {
		for(y = 1; y < pgm->height - 1; y++) {
			if(tmp[y * pgm->width + x] != pgm->maxval) {
				tmp[y * pgm->width + x] = 0;
			}
		}
	}

	return tmp;
}

static void run_canney(pgm_info* pgm, canny_opts* opt) {
	pgm_info* tmpImg = NULL;
	pgm_filter mask;
	int8_t* alpha = calloc(sizeof(*alpha), pgm->height * pgm->width);
	uint16_t x, y;

	// Step 1
	mask.width = 5;
	mask.height = 5;
	int8_t mask_filter[] = {4,5,6,4,2,4,9,12,9,4,5,12,15,12,5,4,9,12,9,42,4,5,4,2};
	mask.filter = mask_filter;
	int32_t* Img = apply_filter(pgm, &mask);
	convert_int_arr(pgm, Img);
	free(Img);

	// Step 2
	pgm_filter maskx;
	maskx.width = 3;
	maskx.height = 3;
	int8_t maskx_filter[] = {-1,0,1,-2,0,2,-1,0,1};
	maskx.filter = maskx_filter;
	int32_t* xImg = apply_filter(pgm, &maskx);

	pgm_filter masky;
	masky.width = 3;
	masky.height = 3;
	int8_t masky_filter[] = {1,2,1,0,0,0,-1,-2,-1};
	masky.filter = masky_filter;
	int32_t* yImg = apply_filter(pgm, &masky);

	for(x = 0; x < pgm->width; x++) {
		for(y = 0; y < pgm->height; y++) {
			alpha[y * pgm->width + x] = atan2(yImg[y * pgm->width + x], xImg[y * pgm->width + x]) * 180 / M_PI;
		}
	}

	calculate_mag(pgm, xImg, yImg);
	free(yImg);
	convert_int_arr(pgm, xImg);	
	free(xImg);
	if(opt && opt->mout) {
		save_pgm(pgm, opt->mout);
	}

	opt->upper_threashold = getUpperCuttoff(pgm, opt);
	opt->lower_threashold = opt->upper_threashold/3;
	fprintf(stdout, "Upper Threashold: %d\n", opt->upper_threashold);
	fprintf(stdout, "Lower Threashold: %d\n", opt->lower_threashold);

	if(opt && opt->above) {
		tmpImg = copy_pgm(pgm);
		aboveThreashold(tmpImg, opt->upper_threashold);
		save_pgm(tmpImg, opt->above);
		free_pgm(tmpImg);
		tmpImg = NULL;
	}

	xImg = search_edge(pgm, opt, alpha);
	convert_int_arr(pgm, xImg);
	free(xImg);

	if(opt && opt->doubleThreashold) {
		save_pgm(pgm, opt->doubleThreashold);
	}
}

int main(int argc, char* argv[]) {
	int argi = 1;
	enum mode selectedMode = none;
	char *inputFile;
	sobel_opts sobel_opt;
	canny_opts canny_opt;

	while(argi < argc) {
		if(strncmp(argv[argi], "-i", 2) == 0 || strncmp(argv[argi], "--input", 7) == 0) {
			if(argi + 1 >= argc) {
				argi = -1;
				break;
			}
			argi++;
			inputFile = argv[argi];
		}
		else if(strncmp(argv[argi], "-m", 2) == 0 || strncmp(argv[argi], "--mode", 6) == 0) {
			if(argi + 1 >= argc) {
				argi = -1;
				break;
			}
			argi++;
			if(strncmp(argv[argi], "sobel", 5) == 0) {
				selectedMode = sobel;
			}
			else if(strncmp(argv[argi], "canny", 6) == 0) {
				selectedMode = canny;
			}
			else {
				argi = -1;
				break;
			}
		}
		else if(selectedMode == sobel) {
			if(strncmp(argv[argi], "-h", 2) == 0 || strncmp(argv[argi], "--hout", 6) == 0) {
				if(argi + 1 >= argc) {
					argi = -1;
					break;
				}
				argi++;
				sobel_opt.hout = argv[argi];
			}
			else if(strncmp(argv[argi], "-v", 2) == 0 || strncmp(argv[argi], "--vout", 6) == 0) {
				if(argi + 1 >= argc) {
					argi = -1;
					break;
				}
				argi++;
				sobel_opt.vout = argv[argi];
			}
			else if(strncmp(argv[argi], "-g", 2) == 0 || strncmp(argv[argi], "--gradeout", 10) == 0) {
				if(argi + 1 >= argc) {
					argi = -1;
					break;
				}
				argi++;
				sobel_opt.output = argv[argi];
			}
			else if(strncmp(argv[argi], "-a", 2) == 0 || strncmp(argv[argi], "--above", 7) == 0) {
				if(argi + 2 >= argc) {
					argi = -1;
					break;
				}
				argi++;
				sobel_opt.above_threashold = atoi(argv[argi]);
				argi++;
				sobel_opt.above = argv[argi];
			}
			else if(strncmp(argv[argi], "-b", 2) == 0 || strncmp(argv[argi], "--below", 7) == 0) {
				if(argi + 2 >= argc) {
					argi = -1;
					break;
				}
				argi++;
				sobel_opt.below_threashold = atoi(argv[argi]);
				argi++;
				sobel_opt.below = argv[argi];
			}
		}
		else if(selectedMode == canny) {
			if(strncmp(argv[argi], "-s", 2) == 0 || strncmp(argv[argi], "--sigma", 7) == 0) {
				if(argi + 1 >= argc) {
					argi = -1;
					break;
				}
				argi++;
				canny_opt.sigma = atof(argv[argi])/100;
			}
			else if(strncmp(argv[argi], "-g", 2) == 0 || strncmp(argv[argi], "--gradeout", 9) == 0) {
				if(argi + 1 >= argc) {
					argi = -1;
					break;
				}
				argi++;
				canny_opt.mout = argv[argi];
			}
			else if(strncmp(argv[argi], "-a", 2) == 0 || strncmp(argv[argi], "--above", 7) == 0) {
				if(argi + 1 >= argc) {
					argi = -1;
					break;
				}
				argi++;
				canny_opt.above = argv[argi];
			}
			else if(strncmp(argv[argi], "-d", 2) == 0 || strncmp(argv[argi], "--double", 8) == 0) {
				if(argi + 1 >= argc) {
					argi = -1;
					break;
				}
				argi++;
				canny_opt.doubleThreashold = argv[argi];
			}
		}
		else if(strncmp(argv[argi], "-h", 2) == 0 || strncmp(argv[argi], "--help", 6) == 0) {
			argi = -1;
			break;
		}

		argi++;
	}

	if(selectedMode == none || inputFile == NULL) {
		fprintf(stderr, "No mode selected\n");
		argi = -1;
	}

	if(selectedMode == canny && !(canny_opt.sigma > 0 && canny_opt.sigma < 1)) {
		fprintf(stderr, "Invalid canny options %f\n", canny_opt.sigma);
		argi = -1;
	}

	if(argi == -1 || argc < 2) {
		fprintf(stderr, "Usage: %s -i [input] -m [mode] [options]\n", argv[0]);
		fprintf(stderr, "Read in a PGM to perform some image processing and save the results\n\n");
		fprintf(stderr, "Parameters:\n");
		fprintf(stderr, "  -i, --input [input]    Path to a source PGM image\n");
		fprintf(stderr, "  -m, --mode [mode]      Select the type of operation that will be perfomred on the image\n");
		fprintf(stderr, "\nModes:\n");
		fprintf(stderr, "  \"sobel\" - perform canney sobel detection\n");
		fprintf(stderr, "  \"canney\" - perform canney edge detection\n");
		fprintf(stderr, "\nsobel:\n");
		fprintf(stderr, "  -h --hout [output]     Save horizontal grandient\n");
		fprintf(stderr, "  -v --vout [output]     Save vertical grandient\n");
		fprintf(stderr, "  -g --gradout [output]  Save overall grandient\n");
		fprintf(stderr, "  -a --above [threashold] [output]    Save image with pixels above threashold\n");
		fprintf(stderr, "  -b --below [threashold] [output]    Save image with pixels below threashold\n");
		fprintf(stderr, "\ncanny:\n");
		fprintf(stderr, "  -s --sigma [float]     Sigma value used for threasholding\n");
		fprintf(stderr, "  -g --gradout [output]  Save overall grandient\n");
		fprintf(stderr, "  -a --above [output]    Save image with pixels above threashold\n");
		fprintf(stderr, "  -d --double [output]   Save image with pixels between threasholds\n");
		return EXIT_FAILURE;
	}
	
	// Open a file descriptor to give to mmap()
	int fd = open(inputFile, O_RDONLY);
	if(fd == -1) {
		perror(argv[1]);
		return EXIT_FAILURE;
	}
	
	// Determine size of file
	size_t filesize = lseek(fd, 0, SEEK_END);
	
	// Map the contents of the file
	void* map = mmap(NULL, filesize, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
	if(map == MAP_FAILED) {
		perror("mmap");
		close(fd);
		return EXIT_FAILURE;
	}
	
	// Attempt to print its segment names
	pgm_info pgm;
	if(!read_pgm(map, filesize, &pgm)) {
		fprintf(stderr, "Error reading PGM image file!\n");
		return EXIT_FAILURE;
	}
	
	// Do stuff with PGM file...
	if(selectedMode == sobel) {
		run_sobel(&pgm, &sobel_opt);		
	}
	else if(selectedMode == canny) {
		run_canney(&pgm, &canny_opt);
	}
	
	// Clean up
	munmap(map, filesize);
	close(fd);
	return EXIT_SUCCESS;
}