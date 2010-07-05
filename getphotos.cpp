# include <iostream>
# include <fstream>
# include <stdlib.h>
# include <stdio.h>
# include <sys/stat.h>
# include <stdint.h>

# include "dataobjects.h"
# include "image.h"

using namespace std;

# define VERSION "0.1"

void read_mhod (Mhod *, FILE *);
void load_image (char *image, string inputfile, int offset, int size);

int main (int argc, char *argv[]) {
  cout << "getphotos " << VERSION << endl;

  if (argc < 3) {
    cerr << "Usage: " << argv[0] << " [iPod photo directory] [output directory]" << endl;
    exit (1);
  }


  string datadir = string (argv[1]);
  string photodb_u = datadir + "/Photo Database";
  string outdir = string (argv[2]);

  // check for file
  if (access (photodb_u.c_str (), R_OK)) {
    cerr << "Could not access: " << photodb_u << endl;
    exit (1);
  }

  // check outputdir
  struct stat statbuf;
  if (stat (outdir.c_str (), &statbuf) < 0) {
    cerr << "Invalid output directory." << endl;
    exit (1);
  }

  if (!S_ISDIR (statbuf.st_mode)) {
    cerr << "Output not a directory." << endl;
    exit (1);
  }


  FILE *photodb = fopen (photodb_u.c_str (), "rb");
  int currenteoffset = 0;

  // read header
  cout << "Reading header..";
  Mhfd mhfd;
  fread (&mhfd, sizeof (mhfd), 1, photodb);
  cout << "done. total length: " << mhfd.totallength << " bytes." << endl;

  // goto mhsd
  cout << "Reading image dataset..";
  fseek (photodb, mhfd.headerlength, SEEK_SET);
  currenteoffset = mhfd.headerlength;

  Mhsd mhsd;
  fread (&mhsd, sizeof (mhsd), 1, photodb);
  cout << "done. total length: " << mhsd.totallength << " bytes." << endl;

  if (mhsd.index != 1) {
    cerr << "mhsd.index=" << mhsd.index << ", should be 1 for images!";
    exit (1);
  }

  // read image list
  cout << "Reading image list..";
  currenteoffset += mhsd.headerlength;
  fseek (photodb, currenteoffset, SEEK_SET);
  Mhli mhli;
  fread (&mhli, sizeof (mhli), 1, photodb);
  cout << "done. number of images: " << mhli.imagecount << endl;

  // loading images
  ImageItem *items = new ImageItem[mhli.imagecount];
  currenteoffset += mhli.headerlength;
  fseek (photodb, currenteoffset, SEEK_SET);

  ImageItem *current = items;

  for (int loaded = 0; loaded < mhli.imagecount; loaded++) {
    //cout << "Loading image " << (loaded + 1) << " of " << mhli.imagecount;

    // Mhii
    fread (&(current->image), sizeof(current->image), 1, photodb);
    currenteoffset += current->image.headerlength;
    fseek (photodb, currenteoffset, SEEK_SET);

    if (current->image.childrencount > 5) {
      cerr << endl << "Got too big number of children (>5) on image: " << current->image.childrencount << endl;
      exit (1);
    }

    current->children = new ImageItemChild[current->image.childrencount];

    // loading children
    for (int i = 0; i < current->image.childrencount; i++) {
      //cout << "\r" << flush << "Loading " << (i + 1) << " of " << current->image.childrencount << " children.. current offset: " << currenteoffset;

      // read mhod header
      read_mhod (&(current->children[i].header), photodb);
      currenteoffset += current->children[i].header.headerlength;
      fseek (photodb, currenteoffset, SEEK_SET);

      // read mhni
      fread (&(current->children[i].imagename), sizeof (Mhni), 1, photodb);

      currenteoffset += current->children[i].imagename.headerlength;
      fseek (photodb, currenteoffset, SEEK_SET);

      /*
      cout << "dimensions[" << i << "]: " << current->children[i].imagename.imagewidth << "x" << current->children[i].imagename.imageheight
           << " (" << current->children[i].imagename.imagesize << ")" << endl;
      */

      // read mhod name
      read_mhod (&(current->children[i].name), photodb);
      currenteoffset += current->children[i].name.totallength;
      fseek (photodb, currenteoffset, SEEK_SET);
    }
    current++;
  }

  // Copying image
  current = items;

  //for (int i = 0; i < mhli.imagecount; i++) {
  for (int i = 0; i < 1; i++) {
    cout << "Copying image " << (i + 1) << " of " << mhli.imagecount << ".." << endl;

    // copying each children
    for (int j = 0; j < current->image.childrencount; j++) {

      string name = string (current->children[j].name.name);
      for (int r = 0; r < name.length(); r++)
        if (name[r] == ':') name[r] = '/';
      name = datadir + name;

      string outfile = outdir;

      if (current->children[j].header.type == 5) {
        // full screen
        // doesn work at the moment
        exit (1);
        outfile = outfile + "/full";
        mkdir (outfile.c_str(), 0755);
        outfile = outfile + "/";
      } else {
        int size = current->children[j].imagename.imagesize;

        char no[9];
        sprintf (no, "%d", (i + 1)); // image number

        /* On an iPod video (5G) there are 4 different thumbnails type:
         *    http://www.ipodlinux.org/wiki/ITunesDB/Photo_Database
         *    720x480 interlaced UYVY (YUV 4:2:2) - used for TV output - 691200 bytes each single thumbnail
         *    320x240 byte swapped RGB565 - used for fullscreen on the iPod - 153600 bytes each single thumbnail
         *    130x88 byte swapped RGB565 - used on the iPod during slideshow, when current photo is displayed on TV - 22880 bytes
              each single thumbnail
         *    50x41 byte swapped RGB565 - used on the iPod when listing and during slideshow - 4100 bytes each single thumbnail
         */

        switch (j) {
          case 0: 
                {
                    char image[size];
                    int offset = current->children[j].imagename.ithmboffset;
                    cout << "Reading from: " << name << endl <<
                            "offset=" << offset << endl <<
                            "size=" << size << endl;
                    ifstream in (name.c_str(), ios::in | ios::binary | ios::ate);
                    in.seekg (offset, ios::beg);
                    in.read (image, size);
                    in.close ();

                    // rgb 888, 3 x 1 byte
                    int bmpsize = 720 * 480 * 3;

                    outfile = outfile + "/big";
                    mkdir (outfile.c_str(), 0755);
                    outfile = outfile + "/" + no + ".bmp";

                    string sourcef = outfile + ".source";
                    ofstream source (sourcef.c_str(), ios::binary | ios::trunc);
                    source.write (image, size);
                    source.close ();

                    ofstream output (outfile.c_str(), ios::binary | ios::trunc);
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
                    dib.width = 720;  // current->children[j].imagename.imagewidth;
                    dib.height = 480; // current->children[j].imagename.imageheight;
                    dib.nplanes = 1;
                    dib.bitspp = 24; // RGB888
                    dib.compress_type = BMP_COMPRESS_TYPE;
                    dib.bmp_bytesz = bmpsize; 

                    dib.hres = 2835;
                    dib.vres = 2835;
                    dib.ncolors = 0;
                    dib.nimpcolors = 0;

                    // convert to rgb888
                    // from uyvy
                    char rgb[bmpsize];

                    for (int p = 0; p < size; p = p + 4) {
                      char u  = image[p];
                      char y1 = image[p + 1];
                      char v  = image[p + 2];
                      char y2 = image[p + 3];

                      char C = y1 - 16;
                      char D = u - 128;
                      char E = v - 128;

                      rgb[(p / 4 * 6)] = (298 * C + 409 * E + 128) >> 8;
                      rgb[(p / 4 * 6) + 1] = (298 * C - 100 * D - 208 * E + 128) >> 8;
                      rgb[(p / 4 * 6) + 2] = (298 * C + 516 * D + 128) >> 8;


                      C = y2 - 16;

                      rgb[(p / 4 * 6) + 3] = (298 * C + 409 * E + 128) >> 8;
                      rgb[(p / 4 * 6) + 4] = (298 * C - 100 * D - 208 * E + 128) >> 8;
                      rgb[(p / 4 * 6) + 5] = (298 * C + 516 * D + 128) >> 8;
                    }

                    output.write (reinterpret_cast<char*> (&magic), sizeof (magic));
                    output.write (reinterpret_cast<char*> (&header), sizeof (header));
                    output.write (reinterpret_cast<char*> (&dib), sizeof (dib));
                    output.write (rgb, bmpsize);

                    output.close ();
                }
                break;

          case 1:
                {
                    char image[size];
                    int offset = current->children[j].imagename.ithmboffset;
                    cout << "Reading from: " << name << endl <<
                            "offset=" << offset << endl <<
                            "size=" << size << endl;
                    ifstream in (name.c_str(), ios::in | ios::binary | ios::ate);
                    in.seekg (offset, ios::beg);
                    in.read (image, size);
                    in.close ();

                    outfile = outfile + "/medium";
                    mkdir (outfile.c_str(), 0755);
                    outfile = outfile + "/" + no + ".bmp";

                    string sourcef = outfile + ".source";
                    ofstream source (sourcef.c_str(), ios::binary | ios::trunc);
                    source.write (image, size);
                    source.close ();

                    ofstream output (outfile.c_str(), ios::binary | ios::trunc);
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
                    dib.width = 320;  // current->children[j].imagename.imagewidth;
                    dib.height = 240; // current->children[j].imagename.imageheight;
                    dib.nplanes = 1;
                    dib.bitspp = 16; // RGB565
                    dib.compress_type = BMP_COMPRESS_TYPE;
                    dib.bmp_bytesz = size; 

                    dib.hres = 2835;
                    dib.vres = 2835;
                    dib.ncolors = 0;
                    dib.nimpcolors = 0;

                    output.write (reinterpret_cast<char*> (&magic), sizeof (magic));
                    output.write (reinterpret_cast<char*> (&header), sizeof (header));
                    output.write (reinterpret_cast<char*> (&dib), sizeof (dib));
                    // backwards..
                    for (int p = 0; p < size; p++)
                      output << image[size - p];

                    output.close ();
                }
                break;

          //default:  cerr << "unknown dimensions: " << current->children[j].imagename.imagewidth << endl;
                    //exit (1);
        }
      }
    }
    current++;
  }

  fclose (photodb);
  return 0;
}

# define MHOD_INITIAL_LENGTH 0x18
// reads an mhod
void read_mhod (Mhod * mhod, FILE *photodb) {
  fread (mhod, MHOD_INITIAL_LENGTH, 1, photodb);
  if (mhod->type == 3) {
    fread (&(mhod->stringsize), sizeof (int), 1, photodb);
    fseek (photodb, 7, SEEK_CUR); // skip the stuff before name

    mhod->name = new char[mhod->stringsize + 1];

    for (int j = 0; j < mhod->stringsize; j++) {
      char t;
      t = fgetc (photodb);
      if (j % 2) mhod->name[j / 2] = t;
    }

    mhod->name[mhod->stringsize] = '\0';
  } else {
    mhod->totallength = 0x18;
  }
}

void load_image (char *image, string inputfile, int offset, int size) {
  ifstream input (inputfile.c_str(), ifstream::binary);
  cout << "loading from: " << inputfile << ", offset=" << offset << ", size=" << size << endl;
  input.seekg (offset, ios::beg);
  input.read (image, size);
  input.close ();
}

