/* Based on various stuff.
 *
 * Currently requires Boost, but could be replaced by a normal char[] since
 * it isn't used for anything. The transformations done in Matlab are mainly
 * done because the reshape() functions transposes the matrix as well.
 *
 * The Y, U and V layers are constructed correctly; if you write them out and
 * put them together in matlab (remember reshape()). You can convert them
 * successfully to rgb. No interpolation is done here thou..
 *
 * Whats missing is a correct YCbCr to RGB888 conversion.
 *
 */

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

      /* consider sub-sampling
       *
       * currently only doublig^2 pixel
       * a better solution would be to use nearest, or billinear
       * methods
       */
      matrix <char> Umatrix (height, width);
      for (int i = 0; i < UmatrixO.size1 (); i++) {
        for (int j = 0; j < UmatrixO.size2 (); j++) {
          Umatrix (2*i, 2*j) = UmatrixO (i, j);
          Umatrix (2*i, 2*j + 1) = UmatrixO (i, j);
          Umatrix (2*i + 1, 2*j) = UmatrixO (i, j);
          Umatrix (2*i + 1, 2*j + 1) = UmatrixO (i, j);
        }
      }

      matrix <char> Vmatrix (height, width);
      for (int i = 0; i < VmatrixO.size1 (); i++) {
        for (int j = 0; j < VmatrixO.size2 (); j++) {
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

    char clip (unsigned long i) {
      if (i > 255) i = 255;
      if (i < 0) i = 0;
      return (char) i;
    }

    void ycbcr2rgb () {
      RGB = new char[bmpsize];

      /*
      for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
          int pixel = i * width + j;
          unsigned long r, g, b;
          char y, u, v;
          y = Y(i, j);
          u = U(i, j);
          v = V(i, j);

          double C, D, E;
          C = y - 16;
          D = u - 128;
          E = v - 128;

          r = ( 298.0 * C             + 409.0 * E + 128.0);
          g = ( 298.0 * C - 100.0 * D - 208.0 * E + 128.0);
          b = ( 298.0 * C + 516.0 * D             + 128.0);

          r = r>>8;
          g = g>>8;
          b = b>>8;

          RGB[3*pixel + 0] = (char) clip(r);
          RGB[3*pixel + 1] = (char) clip(g);
          RGB[3*pixel + 2] = (char) clip(b);
        }
      } */

      for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
          double r, g, b;
          double y, u, v;

          y = static_cast <double> (Y(i, j));
          u = static_cast <double> (U(i, j));
          v = static_cast <double> (V(i, j));

          int p = i * width + j;

          /*r = 1.164 * (y - 16.0) + 1.596 * (v - 128.0);*/
          /*g = 1.164 * (y - 16.0) - 0.813 * (v - 128.0) - 0.392 * (u - 128);*/
          /*b = 1.164 * (y - 16.0) + 2.017 * (u - 128);*/

          r = y + 1.140 * v;
          g = y - 0.395*u - 0.581*v;
          b = y + 2.032*u;

          RGB[p*3 + 0] = static_cast <char> (r);
          RGB[p*3 + 1] = static_cast <char> (g);
          RGB[p*3 + 2] = static_cast <char> (b);
        }
      }
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
      for (int i = 1; i <= bmpsize; i++)
        out << RGB[bmpsize -i];

      /*out.write (RGB, bmpsize);*/
    }
};
