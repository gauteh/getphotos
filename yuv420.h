# pragma once

# include <boost/numeric/ublas/matrix.hpp>
# include <boost/numeric/ublas/io.hpp>
# include "bmp.h"

using namespace std;
using namespace boost::numeric::ublas;

class YUV420 {
  private:
    char *source;
    int width;  // 720
    int height; // 480
    int size;
    int bmpsize;
    matrix <char> Y;
    matrix <char> U;
    matrix <char> V;
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


    void load_yuv () {
      // convert yuv420 to YUV444
      matrix <char> Ymatrix (height, width);
      for (int i = 0; i < Ymatrix.size1 (); i++)
        for (int j = 0; j < Ymatrix.size2 (); j++)
          Ymatrix(i, j) = source[i * width + j];

      int width_h = width / 2;
      int height_h = height / 2;

      char *ustart = &source[width * height];
      
      matrix <char> UmatrixO (height_h, width_h);
      for (int i = 0; i < UmatrixO.size1 (); i++)
        for (int j = 0; j < UmatrixO.size2 (); j++)
          UmatrixO(i, j) = ustart[i * width_h + j];

      char *vstart = &ustart[width_h * height_h];

      matrix <char> VmatrixO (height_h, width_h);
      for (int i = 0; i < VmatrixO.size1 (); i++)
        for (int j = 0; j < VmatrixO.size2 (); j++)
          VmatrixO(i, j) = vstart[i * width_h + j];

      // consider sub-sampling
      // a better solution might be to take average of surrounding
      // pixels
      matrix <char> Umatrix (height, width);
      for (int i = 0; i < UmatrixO.size1 (); i++) {
        for (int j = 0; j < UmatrixO.size2 (); j = j + 2) {
          Umatrix (2*i, 2*j) = UmatrixO (i, j);
          Umatrix (2*i, 2*j + 1) = UmatrixO (i, j);
          Umatrix (2*i + 1, 2*j) = UmatrixO (i, j);
          Umatrix (2*i + 1, 2*j + 1) = UmatrixO (i, j);
        }
      }

      matrix <char> Vmatrix (height, width);
      for (int i = 0; i < VmatrixO.size1 (); i++) {
        for (int j = 0; j < VmatrixO.size2 (); j = j + 2) {
          Vmatrix (2*i, 2*j) = VmatrixO (i, j);
          Vmatrix (2*i, 2*j + 1) = VmatrixO (i, j);
          Vmatrix (2*i + 1, 2*j) = VmatrixO (i, j);
          Vmatrix (2*i + 1, 2*j + 1) = VmatrixO (i, j);
        }
      }

      Y = Ymatrix;
      U = Umatrix;
      V = Vmatrix;

      string Yf = outfile + ".Y";
      ofstream Yo (Yf.c_str(), ios::binary | ios::trunc);
      for (int i = 0; i < Ymatrix.size1 (); i++)
        for (int j = 0; j < Ymatrix.size2 (); j++)
          Yo << (char) Ymatrix(i, j);
      Yo.close ();

      
      string Uf = outfile + ".U";
      ofstream Uo (Uf.c_str(), ios::binary | ios::trunc);
      for (int i = 0; i < Umatrix.size1 (); i++)
        for (int j = 0; j < Umatrix.size2 (); j++)
          Uo << (char) Umatrix(i, j);
      Uo.close ();

      string Vf = outfile + ".V";
      ofstream Vo (Vf.c_str(), ios::binary | ios::trunc);
      for (int i = 0; i < Vmatrix.size1 (); i++)
        for (int j = 0; j < Vmatrix.size2 (); j++)
          Vo << (char) Vmatrix(i, j);
      Vo.close ();
    }

    char clip (long i) {
      if (i > 255) i = 255;
      if (i < 0) i = 0;
      return (char) i;
    }

    void ycbcr2rgb () {
      RGB = new char[bmpsize];


      for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
          int pixel = i * width + j;
          long r, g, b;
          double y, u, v;
          y = Y(i, j);
          u = U(i, j);
          v = V(i, j);

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

          RGB[3*pixel + 0] = clip(r);
          RGB[3*pixel + 1] = clip(g);
          RGB[3*pixel + 2] = clip(b);
        }
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

    void write () {
      write_bmp_header (out, width, height, 3);
      write_rgb ();
    }

    void load () {
      load_yuv ();
      ycbcr2rgb ();
    }

    void write_rgb () {
      out.write (RGB, bmpsize);
    }
};
