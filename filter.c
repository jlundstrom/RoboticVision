#include "filter.h"

int32_t* apply_filter(pgm_info* pgm, pgm_filter* filter) {
	int32_t* tmp = calloc(sizeof(*tmp), pgm->height * pgm->width);
	uint16_t x, y, fx, fy;

	for(x = 1; x < pgm->width - 1; x++) {
		for(y = 1; y < pgm->height - 1; y++) {
			for(fx = 0; fx < filter->width; fx++) {
				for(fy = 0; fy < filter->height; fy++) {
					int val = filter->filter[fy * filter->width + fx] * getpixel(
						pgm,
						x - (filter->width)/2 + fx,
						y - (filter->height)/2 + fy
					);
					tmp[y * pgm->width + x] += val;
				}
			}
		}
	}
	return tmp;
}

void calculate_mag(pgm_info* pgm, int32_t* a, int32_t* b) {
	uint16_t x, y;
	for(x = 0; x < pgm->width; x++) {
		for(y = 0; y < pgm->height; y++) {
			a[y * pgm->width + x] = hypot(ABS(a[y * pgm->width + x]), ABS(b[y * pgm->width + x]));
		}
	}
}

void convert_int_arr(pgm_info* pgm, int32_t* img) {
	uint32_t maxDelta = 0;
	uint16_t x, y;

	for(x = 0; x < pgm->width; x++) {
		for(y = 0; y < pgm->height; y++) {
			maxDelta = MAX(maxDelta, ABS(img[y * pgm->width + x]));
		}
	}

	for(x = 0; x < pgm->width; x++) {
		for(y = 0; y < pgm->height; y++) {
			setpixel(pgm, x, y, ABS(img[y * pgm->width + x]) * pgm->maxval / maxDelta);
		}
	}
}

void aboveThreashold(pgm_info* pgm, uint16_t threashold) {
	int x, y;
	for(x = 0; x < pgm->width; x++) {
		for(y = 0; y < pgm->height; y++) {
			setpixel(pgm, x, y, (getpixel(pgm, x, y) >= threashold ? pgm->maxval : 0));
		}
	}
}

void belowThreashold(pgm_info* pgm, uint16_t threashold) {
	int x, y;
	for(x = 0; x < pgm->width; x++) {
		for(y = 0; y < pgm->height; y++) {
			setpixel(pgm, x, y, (getpixel(pgm, x, y) <= threashold ? pgm->maxval : 0));
		}
	}
}