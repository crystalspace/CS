/*
    Copyright (C) 1998 by Jorrit Tyberghein
    Contributions made by Ivan Avramovic <ivan@avramovic.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/
#include <stdio.h>
#include <math.h>
#include <assert.h>

#include "cssysdef.h"
#include "isys/system.h"
#include "igraphic/loader.h"
#include "csgfx/csimage.h"
#include "csutil/util.h"
#include "imgload.h"

static csImageLoader *loaderlist = NULL;

bool csImageLoader::Register (csImageLoader* loader)
{
  // check if already present
  for (csImageLoader *l = loaderlist; l; l = l->Next)
    if(l == loader)
      return true;
  // add it
  loader->Next = loaderlist;
  loaderlist = loader;
  return true;
}

iImage* csImageLoader::Load (UByte* iBuffer, ULong iSize, int iFormat)
{
  for (csImageLoader *l = loaderlist; l; l = l->Next)
  {
    csImageFile *img = l->LoadImage (iBuffer, iSize, iFormat);
    if (img) return img;
  }
  return NULL;
}

void csImageLoader::SetDithering (bool iEnable)
{
  extern bool csImage_dither;
  csImage_dither = iEnable;
}

//------------------------------------------------------ Plug-in stuff --------

class csImageLoaderPlugin : public iImageLoader {
public:
  DECLARE_IBASE;

  csImageLoaderPlugin(iBase *p)
  {
    CONSTRUCT_IBASE(p);
  }

  ~csImageLoaderPlugin()
  {
    csImageLoader *ldr, *next;
    for (ldr = loaderlist; ldr != NULL; ldr = next)
    {
      next = ldr->GetNext();
      delete ldr;
    }
    loaderlist = NULL;
  }

  virtual bool Initialize(iSystem *)
  {
    // Register all file formats we want
    #define REGISTER_FORMAT(format) \
    extern bool Register##format (); \
    Register##format();

    #if defined (DO_GIF)
      REGISTER_FORMAT (GIF)
    #endif
    #if defined (DO_PNG)
      REGISTER_FORMAT (PNG)
    #endif
    #if defined (DO_JPG)
      REGISTER_FORMAT (JPG)
    #endif
    #if defined (DO_BMP)
      REGISTER_FORMAT (BMP)
    #endif
    #if defined (DO_WAL)
      REGISTER_FORMAT(WAL)
    #endif
    #if defined (DO_SGI)
      REGISTER_FORMAT(SGI)
    #endif
    #if defined (DO_TGA)
      REGISTER_FORMAT (TGA)
    #endif

    return true;
  }

  virtual iImage *Load (UByte* iBuffer, ULong iSize, int iFormat)
  {
    return csImageLoader::Load(iBuffer, iSize, iFormat);
  }

  /**
   * Set global image dithering option.<p>
   * By default this option is disabled. If you enable it, all images will
   * be dithered both after loading and after mipmapping/scaling. This will
   * affect all truecolor->paletted image conversions.
   */
  virtual void SetDithering (bool iEnable)
  {
    csImageLoader::SetDithering(iEnable);
  }
};

IMPLEMENT_IBASE(csImageLoaderPlugin);
  IMPLEMENTS_INTERFACE(iImageLoader);
  IMPLEMENTS_INTERFACE(iPlugIn);
IMPLEMENT_IBASE_END;

IMPLEMENT_FACTORY(csImageLoaderPlugin);

EXPORT_CLASS_TABLE (imgload)
  EXPORT_CLASS (csImageLoaderPlugin, "crystalspace.image.loader",
    "Image file loader plug-in.")
EXPORT_CLASS_TABLE_END
