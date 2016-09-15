#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <assert.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>

// Information about a PGM image
typedef struct pgm_info {
	uint16_t width;
	uint16_t height;
	uint16_t maxval;
	void* pixels;
} pgm_info;

inline uint16_t getpixel(const pgm_info* pgm, int x, int y);

inline void setpixel(pgm_info* pgm, uint16_t x, uint16_t y, uint16_t val);

bool read_pgm(void* data, size_t size, pgm_info* pgm);

void write_pgm(pgm_info* pgm, FILE* fp);

void save_pgm(pgm_info* pgm, char* fn);

pgm_info* create_pgm(uint16_t width, uint16_t height, uint16_t maxval);

pgm_info* copy_pgm(const pgm_info* pgm);

void free_pgm(pgm_info* pgm);