// Define this to zero to enable assertions
#define NDEBUG 0

#include "pgm.h"
#include "filter.h"
#include "sobel.h"
#include "canny.h"
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