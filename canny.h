#pragma once
#include "pgm.h"
#include "filter.h"

typedef struct canny_opts {
	char* mout;
	char* above;
	char* doubleThreashold;
	double sigma;
	uint16_t upper_threashold;
	uint16_t lower_threashold;
} canny_opts;

void run_canney(pgm_info* pgm, canny_opts* opt);