/* data objects; database, sets, albums and photo information for reading
 * the Photo Database.
 *
 * Not necessarily correct, but works on i686, low-endian :p
 *
 * based on: http://www.ipodlinux.org/wiki/ITunesDB#Photo_Database
 * for 5G photo ipods or compatible
 */

# pragma once

// top-level element, offset = 0
typedef struct _Mhfd {
  char headerid[4];             // offset = 0, size = 4, = 'mhfd'
  int headerlength;             // offset = 4, size = 4
  int totallength;              // offset = 8, size = 4
  int unknown1;                 // offset = 12, size = 4
  int unknown2;                 // offset = 16, size = 4
  int childrencount;            // offset = 20, size = 4
  int unknown3;                 // offset = 24, size = 4
  int nextidmhii;               // offset = 28, size = 4
  double unknown5;              // offset = 32, size = 8
  double unknown6;              // offset = 40, size = 8
  int unknown7;                 // offset = 48, size = 4
  int unknown8;                 // offset = 52, size = 4
  int unknown9;                 // offset = 56, size = 4
  int unknown10;                // offset = 60, size = 4
  int unknown11;                // offset = 64, size = 4
  // offset = 68, rest of header is zero padded
} Mhfd;

// Dataset
typedef struct _Mhsd {
  char headerid[4];             // offset = 0, size = 4, = 'mhsd'
  int headerlength;             // offset = 4, size = 4
  int totallength;              // offset = 8, size = 4
  int index;                    // offset = 12, size = 4
                                // should be 1 for images
  // offset = 16, rest is zero padded
} Mhsd;

// Image list
typedef struct _Mhli {
  char headerid[4];             // offset = 0, size = 4, = 'mhli'
  int headerlength;             // offset = 4, size = 4
  int imagecount;               // offset = 8, size = 4
  // rest is zero padded
} Mhli;

// Image item
typedef struct _Mhii {
  char headerid[4];             // offset = 0, size = 4, = 'mhii'
  int headerlength;             // offset = 4, size = 4
  int totallength;              // offset = 8, size = 4

  // one for each thumnail type + full resolution
  int childrencount;            // offset = 12, size = 4

  // First mhii is 0x40, second 0x41 and so on
  int id;                       // offset = 16, size = 4

  // uniqe id corresponding to a dbid field in iTunes db mhit record
  double songid;                // offset = 20, size = 8

  int unknown4;                 // offset = 28, size = 4
  int rating;                   // offset = 32, size = 4
  int unknown6;                 // offset = 36, size = 4

  // timestamp of file creation
  int originaldate;             // offset = 40, size = 4

  // timestamp of date taken
  int digitizeddate;            // offset = 44, size = 4

  // size in bytes of original source image
  int sourceimagesize;          // offset = 48, size = 4

  // rest is zero padded.
} Mhii;

// image name header
typedef struct _Mhod {
  char headerid[4];             // offset = 0, size = 4, = 'mhod'

  // headerlength, except end string
  char headerlength;            // offset = 4, size = 1, = 18

  // header + full size of mhod (including filename string)
  int totallength;              // offset = 5, size = 4

  // type:
  // 5 = full resolution header
  // 2 = thumbnail
  // 3 = child of Mhni, file name, etc..
  short type;                     // offset = 9, size = 4
  short unknown1;

  int unknown2;                 // offset = 15, size = 4
  int unknown3;                 // offset = 19, size = 4

  // type 5 and 2 stops here.

  // size of following string, either 42 (for thumbs) or
  // some other for full resolutions
  int stringsize;               // offset = 23, size = 4

  /*int unknown4;                 // offset = 27, size = 4 */
  /*int unknown5;                 // offset = 31, size = 4*/
  /*int unknown6;                 // offset = 35, size = 4*/

  char *name;                   // offset = , size = stringsize
  // total size = 24 ?
} Mhod;

// image name
typedef struct _Mhni {
  char headerid[4];             // offset = 0, size = 4, = 'mhod'
  int headerlength;             // offset = 4, size = 4, = 18
  int totallength;              // offset = 8, size = 4
  int childrencount;            // offset = 12, size = 4

  // used to generate file name containing image
  int correlationid;            // offset = 16, size = 4

  // where to find image in ithmb
  int ithmboffset;              // offset = 20, size = 4
  int imagesize;                // offset = 24, size = 4

  signed short int verticalpadding;   // offset = 28, size = 2
  signed short int horizontalpadding; // offset = 30, size = 2

  short int imageheight;        // offset = 32, size = 2
  short int imagewidth;         // offset = 34, size = 2

  int unknown;                  // offset = 36, size = 4

  // same as previous
  int imagesize2;               // offset = 40, size = 4

  // rest is zero padded
} Mhni;

typedef struct _ImageItemChild {
  Mhod header;
  Mhni imagename;
  Mhod name;
} ImageItemChild;

typedef struct _ImageItem {
  Mhii image;
  ImageItemChild *children; // either 4 or 5 (with full res)
} ImageItem;

// Album list
// never got that far..

