#include "canny.h"

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

void run_canney(pgm_info* pgm, canny_opts* opt) {
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