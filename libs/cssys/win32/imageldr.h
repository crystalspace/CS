#ifndef _IMAGELDR_H_
#define _IMAGELDR_H_

extern "C" {
  #define Byte z_Byte     /* Kludge to avoid conflicting typedef in zconf.h */
  #include "zlib.h"
  #undef Byte
  #include "png.h"
}
#include "csgfxldr/gifimage.h"
#include "csgfxldr/jpgimage.h"
#include "csgfxldr/pngimage.h"
#include "csgfxldr/tgaimage.h"

class init_ImageLoader
{
private:
  GIFImageLoader* gif;
  JPGImageLoader* jpg;
  TGAImageLoader* tga;
  PNGImageLoader* png;

public:
  init_ImageLoader() : gif(NULL), jpg(NULL), tga(NULL), png(NULL), 
     initialized(false) {}

  ~init_ImageLoader()
  {
    if (gif) CHKB( delete gif );
    if (jpg) CHKB( delete jpg );
    if (png) CHKB( delete png );
    if (tga) CHKB( delete tga );
  }

  bool initialized;

  void init();
};

extern init_ImageLoader image_loader;

#endif

