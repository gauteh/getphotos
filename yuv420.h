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
    char *Y;
    char *U;
    char *V;
    char *RGB;

    ofstream out;
    string outfile;
  public:
    YUV420 (char *s, int w, int h, int sz, string outfilen) {
      source = s;
      width = w;
      height = h;
      size = sz;
      bmpsize = height * width * 3;
      outfile = outfilen;
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
      char YMatrixO[width * height]; // char = uint8
      for (int p = 0; p < (width*height); p++) YMatrixO[p] = source[p];


      // transpose
      char YMatrix[width * height];
      for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
          YMatrix[j * width + i] = YMatrixO[i * width + j];
        }
      }


      int width_h = width / 2;
      int height_h = height / 2;

      char UMatrixO[width_h * height_h];
      for (int p = 0; p < width_h * height_h; p++)
        UMatrixO[p] = source[p + (width * height)];
      
      // transpose
      char UMatrix[width * height];
      for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
          UMatrix[j * width + i] = UMatrixO[i * width + j];
        }
      }

      char VMatrixO [width_h * height_h];
      for (int p = 0; p < (width_h * height_h); p++)
        VMatrixO[p] = source[p + (width * height) + (width_h * height_h)];
      // transpose
      char VMatrix[width * height];
      for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
          VMatrix[j * width + i] = VMatrixO[i * width + j];
        }
      }

      // fix sub-sampling
      char UMatrix1 [height_h * width];
      for (int p = 0; p < height_h * width; p++) UMatrix1[p] = 0;

      for (int p = 0; p < (height_h * width) / 2 ; p++)
        UMatrix1[2*p] = UMatrix[p];

      for (int p = 0; p < (height_h * width) / 2 - (width / 2); p++)
        UMatrix1[2*p +1] = UMatrix[p];

      char VMatrix1 [height_h * width];
      for (int p = 0; p < height_h * width; p++) VMatrix1[p] = 0;

      
      for (int p = 0; p < (height_h * width) / 2 ; p++)
        VMatrix1[2*p] = VMatrix[p];

      for (int p = 0; p < (height_h * width) / 2 - (width / 2); p++)
        VMatrix1[2*p + 1] = VMatrix[p];

      
      // compose YUV-matrix, 3 dimensions
      int pixels = height * width;
      Y = new char[pixels];
      U = new char[pixels];
      V = new char[pixels];

      for (int p = 0; p < pixels; p++) Y[p] = YMatrix[p];


      // fill even rows
      for (int p = 0; p < pixels / 2 - width; p++) {
        int h = p / width;
        int w = p % width;
        U[2*h*width + w] = UMatrix1[p];
        U[(2*h+1)*width + w] = UMatrix1[p];
      }
      //
      // fill even rows
      for (int p = 0; p < pixels / 2 - width; p++) {
        int h = p / width;
        int w = p % width;
        V[2*h*width + w] = VMatrix1[p];
        V[(2*h+1)*width + w] = VMatrix1[p];
      }

      string Yf = outfile + ".Y";
      ofstream Yo (Yf.c_str(), ios::binary | ios::trunc);
      Yo.write (Y, pixels);
      Yo.close ();
      string Uf = outfile + ".U";
      ofstream Uo (Uf.c_str(), ios::binary | ios::trunc);
      Uo.write (U, pixels);
      Uo.close ();
      string Vf = outfile + ".V";
      ofstream Vo (Vf.c_str(), ios::binary | ios::trunc);
      Vo.write (V, pixels);
      Vo.close ();
    }

    char clip (long i) {
      if (i > 255) i = 255;
      if (i < 0) i = 0;
      return (char) i;
    }

    void ycbcr2rgb () {
      RGB = new char[bmpsize];

      int pixels = height * width;

      for (int p = 0; p < pixels; p++) {
        long r, g, b;
        double y, u, v;
        y = Y[p];
        u = U[p];
        v = V[p];

        double C, D, E;
        C = y - 16;
        D = u - 128;
        E = v - 128;

        r = ( 298.0 * C           + 409.0 * E + 128.0);
        g = ( 298.0 * C - 100.0 * D - 208.0 * E + 128.0);
        b = ( 298.0 * C + 516.0 * D           + 128.0);

        r = r>>8;
        g = g>>8;
        b = b>>8;

        RGB[3*p + 0] = clip(r);
        RGB[3*p + 1] = clip(g);
        RGB[3*p + 2] = clip(b);
      }

      /*
      for (int p = 0; p < pixels; p++) {
        double r, g, b;
        double y, u, v;

        y = Y[p];
        u = U[p];
        v = V[p];

        r = (255.0 / 219.0) * (y - 16.0) +                                                         (255.0 / 112.0) * 0.701 * (v - 128.0); 
        g = (255.0 / 219.0) * (y - 16.0) - (255.0 / 112.0) * 0.886 * (0.114 / 0.587) * (u - 128) - (255.0 / 112.0) * 0.701 * (0.299 / 0.587) * (v - 128);
        b = (255.0 / 219.0) * (y - 16.0) + (255.0 / 112.0) * 0.886                   * (u - 128);

        RGB[p*3 + 0] = (char) r;
        RGB[p*3 + 1] = (char) g;
        RGB[p*3 + 2] = (char) b;
      } */
    }

    void write_rgb () {
      out.write (RGB, bmpsize);
    }
};
