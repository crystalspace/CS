/*
    Copyright (C) 2000 by Jorrit Tyberghein
    Written by Samuel Humphreys

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

#include "sysdef.h"
#include "softex3d.h"
#include "isystem.h"
#include "igraph2d.h"

IMPLEMENT_IBASE (csDynamicTextureSoft3D)
  IMPLEMENTS_INTERFACE (iPlugIn)
  IMPLEMENTS_INTERFACE (iGraphics3D)
IMPLEMENT_IBASE_END

#define SysPrintf System->Printf

csDynamicTextureSoft3D::csDynamicTextureSoft3D (iSystem *iSys, iGraphics2D *iparentG2D) : csGraphics3DSoftwareCommon ()
{
  CONSTRUCT_IBASE (NULL);
  parentG2D = iparentG2D;
  (System = iSys)->IncRef ();
}



bool csDynamicTextureSoft3D::Open (const char *Title)
{
  if (!csGraphics3DSoftwareCommon::Open (Title))
    return false;

#ifdef CS_DEBUG
  bool bFullScreen = G2D->GetFullScreen ();
  SysPrintf(MSG_INITIALIZATION, "Using %s mode %dx%d (internal rendering at %dx%d).\n",
            bFullScreen ? "full screen" : "windowed", G2D->GetWidth (), G2D->GetHeight (), width, height);


  if (pfmt.PixelBytes == 4)
  {
    SysPrintf (MSG_INITIALIZATION, "Using truecolor mode with %d bytes per pixel and %d:%d:%d RGB mode.\n",
          pfmt.PixelBytes, pfmt.RedBits, pfmt.GreenBits, pfmt.BlueBits);

    pixel_shift = 2;
  }
  else if (pfmt.PixelBytes == 2)
  {
    SysPrintf (MSG_INITIALIZATION, "Using truecolor mode with %d bytes per pixel and %d:%d:%d RGB mode.\n",
          pfmt.PixelBytes, pfmt.RedBits, pfmt.GreenBits, pfmt.BlueBits);

    pixel_shift = 1;
  }
  else
  {
    SysPrintf (MSG_INITIALIZATION, "Using palette mode with 1 byte per pixel (256 colors).\n");
    pixel_shift = 0;
  }
#endif
  return true;

}

iGraphics3D *csDynamicTextureSoft3D::CreateOffScreenRenderer 
    ( int width, int height, csPixelFormat *pfmt, void *buffer, 
      RGBPixel *palette, int pal_size )
{
  G2D = parentG2D->CreateOffScreenCanvas (width, height, pfmt, buffer, 
					palette, pal_size);
  if (!G2D)
  {
    SysPrintf (MSG_FATAL_ERROR, "Error opening Graphics2D texture context.\n");
    return NULL;
  }

  if (!csGraphics3DSoftwareCommon::Initialize (System))
    return NULL;

  if (!Open (NULL))
    return NULL;

  return (iGraphics3D*)this;
}

