// (Not) Used for writing common BMP stuff

# pragma once
# include <stdint.h>
# include <fstream>

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

// returns header size, which should be offset = BMP_TOTAL_HEADER_SIZE
int write_bmp_header (ofstream &out, int width, int height, int bpc) {
    uint32_t size = width * height * 3;

    bmpfile_magic magic;
    magic.magic[0] = 'B';
    magic.magic[1] = 'M';

    bmpfile_header header;
    header.filesz = BMP_TOTAL_HEADER_SIZE + size;
    header.creator1 = 0;
    header.creator2 = 0;
    header.bmp_offset = BMP_TOTAL_HEADER_SIZE;

    bmp_dib_v3_header_t dib;
    dib.header_sz = DIB_HEADER_SIZE;
    dib.width = width;  // current->children[j].imagename.imagewidth;
    dib.height = height; // current->children[j].imagename.imageheight;
    dib.nplanes = 1;
    dib.bitspp = bpc * 8; // 8 bit per color
    dib.compress_type = BMP_COMPRESS_TYPE;
    dib.bmp_bytesz = size;

    dib.hres = 0;
    dib.vres = 0;
    dib.ncolors = 0;
    dib.nimpcolors = 0;
    out.write (reinterpret_cast<char*> (&magic), sizeof (magic));
    out.write (reinterpret_cast<char*> (&header), sizeof (header));
    out.write (reinterpret_cast<char*> (&dib), sizeof (dib));

    return BMP_TOTAL_HEADER_SIZE;
}

