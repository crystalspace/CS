#ifndef _IMAGELDR_H_
#define _IMAGELDR_H_

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
    if (gif)  delete gif ;
    if (jpg)  delete jpg ;
    if (png)  delete png ;
    if (tga)  delete tga ;
  }

  bool initialized;

  void init();
};

extern init_ImageLoader image_loader;

#endif

