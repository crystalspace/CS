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
#include "csgfxldr/csimage.h"
#include "csutil/util.h"

// Register all file formats we want
#define REGISTER_FORMAT(format) \
  extern bool Register##format (); \
  bool __##format##_supported = Register##format ();

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
