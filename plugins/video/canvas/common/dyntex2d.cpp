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

#include <stdarg.h>
#include "sysdef.h"
#include "dyntex2d.h"
#include "csutil/scf.h"
#include "isystem.h"

IMPLEMENT_IBASE (csDynamicTexture2D)
  IMPLEMENTS_INTERFACE (iPlugIn)
  IMPLEMENTS_INTERFACE (iGraphics2D)
IMPLEMENT_IBASE_END

// csDynamicTexture2D functions
csDynamicTexture2D::csDynamicTexture2D (iSystem *isys) :
  csGraphics2D ()
{
  CONSTRUCT_IBASE (NULL);
  System = isys;
}

csDynamicTexture2D::~csDynamicTexture2D ()
{
  // This array is shared with the texture, let the texture destroy it.
  Palette = NULL;
}

iGraphics2D *csDynamicTexture2D::CreateOffScreenCanvas (int width, int height, 
    csPixelFormat *ipfmt, void *buffer,  RGBPixel *palette, int pal_size)
{
  Width = width;
  Height = height;
  Font = 0;
  FullScreen = false;

  if (palette)
  {
    Depth = 8;
    pfmt.PalEntries = pal_size;
    pfmt.PixelBytes = 1;

    // Initialize pointers to default drawing methods
    _DrawPixel = DrawPixel8;
    _WriteChar = WriteChar8;
    _GetPixelAt = GetPixelAt8;

    Palette = palette;
    pfmt.RedMask = 0;
    pfmt.GreenMask = 0;
    pfmt.BlueMask = 0;

    for (int i = 0; i < 256; i++)
      PaletteAlloc [i] = false;
  }
  else
  {
    // For other than software Renderers falling back on software texture 
    // buffers
    if (ipfmt->PixelBytes == 2)
    {
      Depth = 16;
      pfmt.PixelBytes = 2;
      _DrawPixel = DrawPixel16;
      _WriteChar = WriteChar16;
      _GetPixelAt = GetPixelAt16;
    }
    else
    {
      pfmt.PixelBytes = 4;
      _DrawPixel = DrawPixel32;
      _WriteChar = WriteChar32;
      _GetPixelAt = GetPixelAt32;
    }
    pfmt.PalEntries = 0;
    pfmt.RedMask = ipfmt->RedMask;
    pfmt.GreenMask = ipfmt->GreenMask;
    pfmt.BlueMask = ipfmt->RedMask;
  }

  pfmt.complete ();

  Memory = (unsigned char*) buffer;

  // get the fontrenderer
  const char *p = System->ConfigGetStr ("FontRender", CS_FUNCID_FONT, 
					"crystalspace.font.render.csfont");
  FontRenderer = LOAD_PLUGIN (System, p, CS_FUNCID_FONT, iFontRender);
  Font = 0;

  return (iGraphics2D*)this;
}

bool csDynamicTexture2D::Open(const char *Title)
{
  CsPrintf (MSG_INITIALIZATION, "Crystal Space dynamic texture buffer");

  // Open your graphic interface
  if (!csGraphics2D::Open (Title))
    return false;

  Clear (0);
  return true;
}

void csDynamicTexture2D::Close ()
{
  // The Memory array is a shared resource with the texture, let the texture
  // manager destroy it.
  Memory = NULL;
  csGraphics2D::Close ();
}

void csDynamicTexture2D::Print (csRect*)
{
  // do nothing as its not buffered
}
