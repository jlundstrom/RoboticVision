#pragma once
#include "pgm.h"

#define CMP(op, a, b) ({ \
	__typeof__(a) _a = (a); \
	__typeof__(b) _b = (b); \
	(_a op _b) ? _a : _b; \
})
#define MIN(a, b) CMP(<=, a, b)
#define MAX(a, b) CMP(>, a, b)
#define ABS(x) ({ \
	__typeof__(x) _x = (x); \
	_x < 0 ? -x : x; \
})

typedef struct pgm_filter {
	uint8_t width;
	uint8_t height;
	int8_t* filter;
} pgm_filter;

int32_t* apply_filter(pgm_info* pgm, pgm_filter* filter);

void calculate_mag(pgm_info* pgm, int32_t* a, int32_t* b);

void convert_int_arr(pgm_info* pgm, int32_t* img);

void aboveThreashold(pgm_info* pgm, uint16_t threashold);

void belowThreashold(pgm_info* pgm, uint16_t threashold);