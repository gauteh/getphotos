# pragma once
                /* {{{ - rgb565 / medium not working
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
                    dib.bitspp = 16; // RGB565, 24 = RGB888

                    dib.compress_type = BMP_COMPRESS_TYPE;
                    dib.bmp_bytesz = size;

                    dib.hres = 2835;
                    dib.vres = 2835;
                    dib.ncolors = 0;
                    dib.nimpcolors = 0;

                    output.write (reinterpret_cast<char*> (&magic), sizeof (magic));
                    output.write (reinterpret_cast<char*> (&header), sizeof (header));
                    output.write (reinterpret_cast<char*> (&dib), sizeof (dib));

                    // converting to rgb888
                    //char rgb[bmpsize];
                    //int r = 0;
                    //for (int p = 0; p < size; p += 2) {
                      //uint16_t *s = &(image[2*p]);
                      //rgb[r] = (*s) &
                    //}

                    // swap bytes
                    //for (int p = 0; p < size; p = p + 2) {
                      //char tmp = image[p];
                      //image[p] = image[p + 1];
                      //image[p + 1] = tmp;
                    //}

                    // backwards..
                    for (int p = 0; p < size; p++)
                      output << image[size - p];

                    output.close ();
                }
                }}} */
