/*
    Copyright (C) 2003 by Jorrit Tyberghein
              (C) 2003 by Frank Richter

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

#include "cssysdef.h"

#include "csutil/scf.h"
#include "csplugincommon/opengl/glcommon2d.h"
#include "csplugincommon/opengl/glss.h"

void csGLScreenShot::IncRef ()
{
  scfRefCount++;
}

void csGLScreenShot::DecRef()
{
  if (scfRefCount == 1)
  {
    G2D->RecycleScreenShot (this);
    return;
  }
  scfRefCount--;
}

SCF_IMPLEMENT_IBASE_GETREFCOUNT(csGLScreenShot)
SCF_IMPLEMENT_IBASE_REFOWNER(csGLScreenShot)
SCF_IMPLEMENT_IBASE_REMOVE_REF_OWNERS(csGLScreenShot)
SCF_IMPLEMENT_IBASE_QUERY(csGLScreenShot)
  SCF_IMPLEMENTS_INTERFACE(iImage)
SCF_IMPLEMENT_IBASE_END

csGLScreenShot::csGLScreenShot (csGraphics2DGLCommon* G2D)
{
  SCF_CONSTRUCT_IBASE(0);

  poolNext = 0;
  csGLScreenShot::G2D = G2D;
  Format = CS_IMGFMT_TRUECOLOR;
  Data = 0;
  dataSize = 0;
}

csGLScreenShot::~csGLScreenShot ()
{
  delete[] Data;
  SCF_DESTRUCT_IBASE();
}

void csGLScreenShot::SetData (void* data)
{
  Width = G2D->GetWidth ();
  Height = G2D->GetHeight ();
  if (dataSize < (size_t)(Width * Height))
  {
    delete[] Data;
    Data = new csRGBpixel [Width * Height];
    dataSize = Width * Height;
  }

// Pixel format is read as RGBA (in a byte array) but as soon as we
// cast it to a 32 bit integer we have to deal with endianess, so convert
// to big endian and convert RGBA to ARGB
// On ABGR machines, we also need to swap B/R bytes
  uint32* s = (uint32*)data;
  int x, y;
  for (y = Height; y-- > 0;)
  {
    csRGBpixel* dest = Data + y * Width;
    for (x = 0 ; x < Width; x++)
    {
      uint32 pix = *s;
#ifdef CS_LITTLE_ENDIAN
  #if (CS_24BIT_PIXEL_LAYOUT == CS_24BIT_PIXEL_ABGR)
      dest->red   = (pix & 0x000000FF);
      dest->green = (pix & 0x0000FF00) >> 8;
      dest->blue  = (pix & 0x00FF0000) >> 16;
      dest->alpha = (pix & 0xFF000000) >> 24;
  #else 
      dest->blue  = (pix & 0x000000FF);
      dest->green = (pix & 0x0000FF00) >> 8;
      dest->red   = (pix & 0x00FF0000) >> 16;
      dest->alpha = (pix & 0xFF000000) >> 24;
  #endif
#else
  #if (CS_24BIT_PIXEL_LAYOUT == CS_24BIT_PIXEL_ABGR)
      dest->alpha = (pix & 0x000000FF);
      dest->blue  = (pix & 0x0000FF00) >> 8;
      dest->green = (pix & 0x00FF0000) >> 16;
      dest->red   = (pix & 0xFF000000) >> 24;
  #else 
      dest->alpha = (pix & 0x000000FF);
      dest->red   = (pix & 0x0000FF00) >> 8;
      dest->green = (pix & 0x00FF0000) >> 16;
      dest->blue  = (pix & 0xFF000000) >> 24;
  #endif
#endif
      s++; dest++;
    }
  }
}

