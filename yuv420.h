# pragma once

# include "bmp.h"

using namespace std;

class YUV420 {
  private:
    char *source;
    int width;  // 720
    int height; // 480
    int size;
    int bmpsize;
    char *YUV;
    char *RGB;

    ofstream out;
  public:
    YUV420 (char *s, int w, int h, int sz, string outfile) {
      source = s;
      width = w;
      height = h;
      size = sz;
      bmpsize = height * width * 3;
      out.open (outfile.c_str(), ios::binary | ios::trunc);
    }

    ~YUV420 () {
      out.close ();
    }

    void write_bmp_header () {
        bmpfile_magic magic;
        magic.magic[0] = 'B';
        magic.magic[1] = 'M';

        bmpfile_header header;
        header.filesz = BMP_TOTAL_HEADER_SIZE + size;
        header.creator1 = 0;
        header.creator2 = 0;
        header.bmp_offset = 40;

        bmp_dib_v3_header_t dib;
        dib.header_sz = DIB_HEADER_SIZE;
        dib.width = width;  // current->children[j].imagename.imagewidth;
        dib.height = height; // current->children[j].imagename.imageheight;
        dib.nplanes = 1;
        dib.bitspp = 24; // RGB888
        dib.compress_type = BMP_COMPRESS_TYPE;
        dib.bmp_bytesz = bmpsize;

        dib.hres = 2835;
        dib.vres = 2835;
        dib.ncolors = 0;
        dib.nimpcolors = 0;
        out.write (reinterpret_cast<char*> (&magic), sizeof (magic));
        out.write (reinterpret_cast<char*> (&header), sizeof (header));
        out.write (reinterpret_cast<char*> (&dib), sizeof (dib));
    }

    void load_yuv () {
      // convert yuv420 to YUV444
      char YMatrix[width * height]; // char = uint8
      int p = 0;
      for (; p < (width*height); p++) YMatrix[p] = source[p];

      int width_h = width / 2;
      int height_h = height / 2;

      char UMatrix[width_h * height_h];
      for (; p < ((width * height) + width_h * height_h); p++)
        UMatrix[p - width * height] = source[p];

      char VMatrix [width_h * height_h];
      for (; p < ((width * height) + 2 * width_h * height_h); p++)
        VMatrix[p - (width * height) + width_h * height_h] = source[p];

      // fix sub-sampling
      char UMatrix1 [height_h * width];
      for (int p = 0; p < height_h * width; p++)
        UMatrix1[p] = 0;

      for (int p = 0; p < (height_h * width) / 2 ; p++)
        UMatrix1[(p / height_h) * width + 2*p] = UMatrix[p];
      for (int p = 0; p < (height_h * width) / 2; p++)
        UMatrix1[(p / height_h) * width + 2*(p+1)] = UMatrix[p];

      char VMatrix1 [height_h * width];
      for (int p = 0; p < height_h * width; p++)
        VMatrix1[p] = 0;

      for (int p = 0; p < (height_h * width) / 2 ; p++)
        VMatrix1[(p / height_h) * width + 2*p] = VMatrix[p];
      for (int p = 0; p < (height_h * width) / 2; p++)
        VMatrix1[(p / height_h) * width + 2*(p+1)] = VMatrix[p];

      // compose YUV-matrix
      YUV = new char [height * width];

      for (int p = 0; p < height * width; p++)
        YUV[p] = YMatrix[p];

      for (int p = 0; p < height * width; p = p + 2)
        YUV[p] = 0;

      for (int p = 0; p < ((height_h-1)  * width/2); p++) {
        YUV[2 * p / height * width + 2*p] = UMatrix1 [p];
        YUV[2 * (p+1) / height * width + 2*p] = UMatrix1 [p];
      }

      for (int p = 0; p < ((height_h-1)  * width/3); p++) {
        YUV[2 * p / height * width + 3*p] = VMatrix1 [p];
        YUV[2 * (p+1) / height * width + 3*p] = VMatrix1 [p];
      }
    }

    void ycbcr2rgb () {
      RGB = new char[bmpsize];
    }

    void write_rgb () {
      out.write (RGB, bmpsize);
    }
};
