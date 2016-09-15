#include "sobel.h"

void run_sobel(pgm_info* pgm, sobel_opts* opt) {
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