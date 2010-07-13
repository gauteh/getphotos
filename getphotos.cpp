/* Copyright 2010 (c) Gaute Hope <eg@gaute.vetsj.com>
 * Distributed under the GNU General Public Licence v3
 *
 * This is used to extract YUV 4:2:0 images from an 120 GB
 * Video/Photo iPod.
 *
 * It could probably easily be adapted to similiar iPods.
 *
 * It does not (yet) convert the images to BMP format; use the Matlab
 * script by Åsmund Kjørstad for that.
 *
 */

# include <iostream>
# include <fstream>
# include <stdlib.h>
# include <stdio.h>
# include <sys/stat.h>
# include <stdint.h>
# include <cstring>

# include "dataobjects.h"

/* An attempt to implement YUV 4:2:0 conversion in C++

# include "bmp.h"
# include "yuv420.h"

*/

using namespace std;

# define VERSION "0.1"

void read_mhod (Mhod *, FILE *);
void load_image (char *image, string inputfile, int offset, int size);

void printhelp () {
    cerr << "Usage: getphotos [-h|--help|-s] source destination\n"
         << "       source          path to copy of Photos directory\n"
         << "       destination     path where to store extracted images\n"
         << "                       [Existing files will be overwritten]\n"
         << "       -h or --help    display this help message\n"
         << "       -s              only extract sources (yuv420, etc)\n"
         << endl;
}

int main (int argc, char *argv[]) {
  cout << "GetPhotos " << VERSION << endl;
  cout << "Copyright Gaute Hope <eg@gaute.vetsj.com> and Åsmund Kjørstad (c) 2010" << endl << endl;
  cout << "Warning: All existing photos in output directory will be overwritten!" << endl;

  if (argc < 3) {
    cerr << "Not enought arguments!" << endl;
    printhelp ();
    exit (1);
  }

  if (!strcmp (argv[1], "-h") || !strcmp(argv[1], "--help")) {
    printhelp ();
    exit (0);
  }

  int convert = 1; // attempt to create BMP files

  if (argc > 3) {
    if (!strcmp (argv[1], "-s")) {
      cout << "Only extracting sources." << endl;
      convert = 0;
      argv = &argv[1];
    } else {
      cerr << "Unknown argument!" << endl;
      printhelp ();
      exit (1);
    }
  }

  // .. and likely for ever..
  if (convert) {
    cerr << "The only thing that works right now is to extract sources; please specify '-s'" << endl;
    printhelp ();
    exit (1);
  }

  string datadir = string (argv[1]);
  string photodb_u = datadir + "/Photo Database";
  string outdir = string (argv[2]);

  // check for file
  if (access (photodb_u.c_str (), R_OK)) {
    cerr << "Could not access: " << photodb_u << endl;
    printhelp ();
    exit (1);
  }

  // check outputdir
  struct stat statbuf;
  if (stat (outdir.c_str (), &statbuf) < 0) {
    cerr << "Invalid output directory." << endl;
    printhelp ();
    exit (1);
  }

  if (outdir[0] == '-' || datadir[0] == '-') {
    cerr << "Invalid source or destination dir!" << endl;
    printhelp ();
    exit (1);
  }

  if (!S_ISDIR (statbuf.st_mode)) {
    cerr << "Output not a directory." << endl;
    printhelp ();
    exit (1);
  }

  // make sourcedir
  string sourcedir = outdir + "/source/";
  mkdir (sourcedir.c_str(), 0755);


  FILE *photodb = fopen (photodb_u.c_str (), "rb");
  int currenteoffset = 0;

  // read header
  cout << "Reading header..";
  Mhfd mhfd;
  fread (&mhfd, sizeof (mhfd), 1, photodb);
  cout << "done, total length: " << mhfd.totallength << " bytes." << endl;

  // goto mhsd
  cout << "Reading image dataset..";
  fseek (photodb, mhfd.headerlength, SEEK_SET);
  currenteoffset = mhfd.headerlength;

  Mhsd mhsd;
  fread (&mhsd, sizeof (mhsd), 1, photodb);
  cout << "done, total length: " << mhsd.totallength << " bytes." << endl;

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
  cout << "done, number of images: " << mhli.imagecount << "." << endl;

  // loading images
  ImageItem *items = new ImageItem[mhli.imagecount];
  currenteoffset += mhli.headerlength;
  fseek (photodb, currenteoffset, SEEK_SET);

  ImageItem *current = items;

  cout << "Reading image db..";
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

    for (int i = 0; i < current->image.childrencount; i++) {
      // read mhod header
      read_mhod (&(current->children[i].header), photodb);
      currenteoffset += current->children[i].header.headerlength;
      fseek (photodb, currenteoffset, SEEK_SET);

      // read mhni
      fread (&(current->children[i].imagename), sizeof (Mhni), 1, photodb);
      currenteoffset += current->children[i].imagename.headerlength;
      fseek (photodb, currenteoffset, SEEK_SET);

      // read mhod name
      read_mhod (&(current->children[i].name), photodb);
      currenteoffset += current->children[i].name.totallength;
      fseek (photodb, currenteoffset, SEEK_SET);
    }
    current++;
  }

  fclose (photodb);
  cout << "done." << endl;

  int fullresinfo = 0;
  for (int i = 0; i < mhli.imagecount; i++) {
    //for (int i = 0; i < 5; i++) {
    cout << "\r" << flush << "Extracting image " << (i + 1) << " of " << mhli.imagecount << "..";

    char no[9];
    sprintf (no, "%d", (i + 1)); // image number

    // copying each children
    for (int j = 0; j < items[i].image.childrencount; j++) {

      string name = string (items[i].children[j].name.name);
      for (int r = 0; r < name.length(); r++)
        if (name[r] == ':') name[r] = '/';
      name = datadir + name;

      string outfile = outdir;

      if (items[i].children[j].header.type == 5) {
        if (!fullresinfo) {
          cout << "\r" << flush << "Good news: Possible full resolution images found; check the 'Photos/Full resolution' folder on your iPod." << endl;
          fullresinfo = 1;
        }
      } else {
        int size = items[i].children[j].imagename.imagesize;
        int offset = items[i].children[j].imagename.ithmboffset;


        /* On an iPod video (5G) there are 4 different thumbnails type:
         *    http://www.ipodlinux.org/wiki/ITunesDB/Photo_Database
         *    This seems to be YUV 4:2:0..
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
                  load_image (image, name, offset , size);

                  string sourcef = sourcedir + "big";
                  mkdir (sourcef.c_str(), 0755);
                  sourcef = sourcef + "/" + no + ".yuv420";
                  ofstream source (sourcef.c_str(), ios::binary | ios::trunc);
                  source.write (image, size);
                  source.close ();

                  /*
                  if (convert) {
                    outfile = outfile + "/big";
                    mkdir (outfile.c_str(), 0755);
                    outfile = outfile + "/" + no + ".bmp";

                    YUV420 yuv (image, 720, 480, size, outfile);
                    yuv.load ();
                    yuv.write ();
                  }
                  */
                }
                break;

          case 1: break; // rgb565
          default: break;
        }
      }
    }
  }
  cout << "done." << endl;

  return 0;
}

# define MHOD_INITIAL_LENGTH 0x18
void read_mhod (Mhod * mhod, FILE *photodb) {
  fread (mhod, MHOD_INITIAL_LENGTH, 1, photodb);
  if (mhod->type == 3) {
    fread (&(mhod->stringsize), sizeof (int), 1, photodb);
    fseek (photodb, 7, SEEK_CUR); // skip the stuff before name

    mhod->name = new char[mhod->stringsize + 1];

    // probably wchar/unicode, we don't want that..
    for (int j = 0; j < mhod->stringsize; j++) {
      char t = fgetc (photodb);
      if (j % 2) mhod->name[j / 2] = t;
    }

    mhod->name[mhod->stringsize] = '\0';

  } else mhod->totallength = 0x18;
}

void load_image (char *image, string inputfile, int offset, int size) {
  ifstream input (inputfile.c_str(), ifstream::binary);
  if (!input.is_open ()) {
      cout << "Could not open file: " << inputfile << " for reading!" << endl;
      exit (1);
  }
  input.seekg (offset, ios::beg);
  input.read (image, size);
  input.close ();
}

