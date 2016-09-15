#pragma once
#include "pgm.h"
#include "filter.h"

typedef struct sobel_opts {
	char* output;
	char* hout;
	char* vout;
	char* above;
	char* below;
	uint16_t above_threashold;
	uint16_t below_threashold;
} sobel_opts;

void run_sobel(pgm_info* pgm, sobel_opts* opt);