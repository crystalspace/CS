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
#include "csgfx/packrgb.h"
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
  Format = CS_IMGFMT_TRUECOLOR | CS_IMGFMT_ALPHA;
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

  // Pixel format is read as RGBA (in a byte array)
  uint8* s = (uint8*)data;
  int y;
  for (y = Height; y-- > 0;)
  {
    csRGBpixel* dest = Data + y * Width;
    csPackRGBA::UnpackRGBAtoRGBpixel (dest, s, Width);
    s += Width*4;
  }
}

