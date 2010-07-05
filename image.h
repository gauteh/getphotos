# pragma once
# include <stdint.h>

using namespace std;

// from: http://en.wikipedia.org/wiki/BMP_file_format

typedef struct {
  unsigned char magic[2]; // magic number used to identify BMP file, should be: BM
} bmpfile_magic;

typedef struct {
  uint32_t filesz;      // size of BMP file in bytes
  uint16_t creator1;    // app specific
  uint16_t creator2;    // app specific
  uint32_t bmp_offset;  // start of actual bitmap data
} bmpfile_header;

typedef struct {
  uint32_t header_sz;
  uint32_t width;
  uint32_t height;
  uint16_t nplanes;
  uint16_t bitspp;
  uint32_t compress_type;
  uint32_t bmp_bytesz;
  uint32_t hres;
  uint32_t vres;
  uint32_t ncolors;
  uint32_t nimpcolors;
} bmp_dib_v3_header_t;

# define BMP_COMPRESS_TYPE  0 // 0 = RGB
# define BMP_TOTAL_HEADER_SIZE 14 + 40 
# define DIB_HEADER_SIZE 40


