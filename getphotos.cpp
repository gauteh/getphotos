# include <iostream>
# include <fstream>
# include <stdlib.h>
# include <stdio.h>
# include <sys/stat.h>

# include "dataobjects.h"

using namespace std;

# define VERSION "0.1"

void read_mhod (Mhod *, FILE *);
void load_image (char *image, string inputfile, int offset, int size);
void write_image (char * image, string outputfile, int size);

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

      // read mhod name
      read_mhod (&(current->children[i].name), photodb);
      currenteoffset += current->children[i].name.totallength;
      fseek (photodb, currenteoffset, SEEK_SET);
    }
    //cout << " done." << endl;
    current++;
  }

  current = items;

  for (int i = 0; i < mhli.imagecount; i++) {
    cout << "Copying image " << (i + 1) << " of " << mhli.imagecount << "..";

    // copying each children
    for (int j = 0; j < current->image.childrencount; j++) {
      string name = string (current->children[j].name.name);
      for (int r = 0; r < name.length(); r++)
        if (name[r] == ':') name[r] = '/';
      name = photodb_u + name;

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
        char image[size];
        load_image (image, name, current->children[j].imagename.ithmboffset, size);

        /* On an iPod video (5G) there are 4 different thumbnails type:
         *    http://www.ipodlinux.org/wiki/ITunesDB/Photo_Database
         *    720x480 interlaced UYVY (YUV 4:2:2) - used for TV output - 691200 bytes each single thumbnail
         *    320x240 byte swapped RGB565 - used for fullscreen on the iPod - 153600 bytes each single thumbnail
         *    130x88 byte swapped RGB565 - used on the iPod during slideshow, when current photo is displayed on TV - 22880 bytes
              each single thumbnail
         *    50x41 byte swapped RGB565 - used on the iPod when listing and during slideshow - 4100 bytes each single thumbnail
         */

        switch (current->children[j].imagename.imagewidth) {
          case 720: outfile = outfile + "/big";
                    break;
          case 320: outfile = outfile + "/medium";
                    break;
          case 130: outfile = outfile + "/small";
                    break;
          case 50:  outfile = outfile + "/smallest";
                    break;
          default:  cerr << "unknown dimensions: " << current->children[j].imagename.imagewidth << endl;
                    exit (1);
        }

        mkdir (outfile.c_str(), 0755);

        char buf[9];
        sprintf (buf, "%d", (i + 1));
        outfile = outfile + "/" + buf;
        write_image (image, outfile, size);
      }


    }


    cout << " done." << endl;
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
  input.seekg (offset, ios::beg);
  input.read (image, size);
  input.close ();
}

void write_image (char * image, string outputfile, int size) {
  ofstream output (outputfile.c_str(), ios::binary | ios::trunc);
  output.write (image, size);
  output.close ();
}
