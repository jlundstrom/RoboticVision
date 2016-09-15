#include "pgm.h"

inline uint16_t getpixel(const pgm_info* pgm, int x, int y) {
	if(x < 0 || x >= pgm->width) {
		return 0;
	}
	if(y < 0 || y >= pgm->height) {
		return 0;
	}
	
	// Check if each pixel is one byte or two
	if(pgm->maxval < 256) {
		return *((uint8_t*)pgm->pixels + pgm->width * y + x);
	}
	else {
		return *((uint16_t*)pgm->pixels + pgm->width * y + x);
	}
}

inline void setpixel(pgm_info* pgm, uint16_t x, uint16_t y, uint16_t val) {
	assert(x < pgm->width);
	assert(y < pgm->height);
	assert(val <= pgm->maxval);
	
	// Check if each pixel is one byte or two
	if(pgm->maxval < 256) {
		*((uint8_t*)pgm->pixels + pgm->width * y + x) = val;
	}
	else {
		*((uint16_t*)pgm->pixels + pgm->width * y + x) = val;
	}
}

// Read an ASCII integer from a binary data stream, updating read pointer.
// Also make sure not to read past end of mapped data
static int read_next_uint16(char** str, char* end) {
	if(*str == end) {
		return -1;
	}
	
	int ret = 0;
	while(isdigit(**str)) {
		ret *= 10;
		ret += **str - '0';
		if(++*str == end || ret > 0xffff) {
			return -1;
		}
	}
	return ret;
}

// Returns true if any spaces were skipped.
// Also make sure not to read past end of mapped data
static bool skipspaces(char** str, char* end) {
	if(*str == end || !isspace(**str)) {
		return false;
	}
	
	do {
		if(++*str == end) {
			break;
		}
	} while(isspace(**str));
	
	return true;
}

// Read a PGM image's header to populate the PGM info struct
bool read_pgm(void* data, size_t size, pgm_info* pgm) {
	char* header = data;
	char* end = header + size;
	
	// Zero output parameter
	memset(pgm, 0, sizeof(*pgm));
	
	// Validate magic
	if(strncmp(header, "P5", 2) != 0) {
		return false;
	}
	header += 2;
	if(!skipspaces(&header, end)) {
		return false;
	}
	
	// Read width
	int width = read_next_uint16(&header, end);
	if(width < 0) {
		return false;
	}
	if(!skipspaces(&header, end) || header == end) {
		return false;
	}
	
	// Read height
	int height = read_next_uint16(&header, end);
	if(height < 0) {
		return false;
	}
	if(!skipspaces(&header, end) || header == end) {
		return false;
	}
	
	// Read max val
	int maxval = read_next_uint16(&header, end);
	if(maxval < 0 || !isspace(*header++)) {
		return false;
	}
	
	// Validate the whole image is contained in the file
	size_t header_size = header - (char*)data; 
	uint64_t claimed_size = header_size + (maxval >= 256 ? 2 : 1) * (uint64_t)width * height;
	if(claimed_size > size) {
		return false;
	}
	
	// Write output parameter and return success
	pgm->width = width;
	pgm->height = height;
	pgm->maxval = maxval;
	pgm->pixels = header;
	return true;
}

void save_pgm(pgm_info* pgm, char* fn) {
	FILE* fp = fopen(fn, "wb");
	write_pgm(pgm, fp);
	fclose(fp);
}

// Write pgm to tile
void write_pgm(pgm_info* pgm, FILE* fp) {
    // Write header
    fprintf(fp, "P5\n%d %d\n%d\n", pgm->width, pgm->height, pgm->maxval);

    // Write pixel data
    fwrite(pgm->pixels, pgm->maxval > 0xff ? 2 : 1, pgm->width * pgm->height, fp);
}

pgm_info* create_pgm(uint16_t width, uint16_t height, uint16_t maxval) {
	// Allocate zero-filled structure for pgm
	pgm_info* ret = calloc(1, sizeof(*ret));
	if(!ret) {
		return NULL;
	}
	
	// Set metadata
	ret->width = width;
	ret->height = height;
	ret->maxval = maxval;
	
	// Allocate pixel data
	ret->pixels = calloc(maxval > 0xff ? 2 : 1, width * height);
	if(!ret->pixels) {
		free(ret);
		return NULL;
	}
	
	return ret;
}

pgm_info* copy_pgm(const pgm_info* pgm) {
	// Allocate zero-filled structure for pgm
	pgm_info* ret = create_pgm(pgm->width, pgm->height, pgm->maxval);
	if(!ret) {
		return NULL;
	}
	
	// Copy pixels
	size_t pixel_size = ret->width * ret->height * (ret->maxval > 0xff ? 2 : 1);
	memcpy(ret->pixels, pgm->pixels, pixel_size);
	return ret;
}

void free_pgm(pgm_info* pgm) {
	if(!pgm) {
		return;
	}
	
	free(pgm->pixels);
	free(pgm);
}