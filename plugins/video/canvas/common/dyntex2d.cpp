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
#include "cssysdef.h"
#include "dyntex2d.h"
#include "csutil/scf.h"
#include "isystem.h"

DECLARE_FACTORY (csDynamicTextureSoft2D)
IMPLEMENT_IBASE (csDynamicTextureSoft2D)
  IMPLEMENTS_INTERFACE (iPlugIn)
  IMPLEMENTS_INTERFACE (iGraphics2D)
IMPLEMENT_IBASE_END

// csDynamicTextureSoft2D functions
csDynamicTextureSoft2D::csDynamicTextureSoft2D (iSystem *isys) :
  csGraphics2D ()
{
  CONSTRUCT_IBASE (NULL);
  System = isys;
  image_buffer = NULL;
}

csDynamicTextureSoft2D::~csDynamicTextureSoft2D ()
{

}

iGraphics2D *csDynamicTextureSoft2D::CreateOffScreenCanvas 
  (int width, int height, csPixelFormat *ipfmt, void *buffer,  
   RGBPixel *palette, int pal_size, int flags)
{
  Width = width;
  Height = height;
  Font = 0;
  FullScreen = false;
  Memory = (unsigned char*) buffer;
  // If flags is set then we are sharing the texture manager and so we can't 
  // render in 8-bit unless screen depth is also 8-bit
  if (!flags || (ipfmt->PixelBytes == 1))
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

    Palette = palette;

    if (!flags)
      for (int i = 0; i < 256; i++)
	PaletteAlloc [i] = false;
    else
      for (int i = 0; i < 256; i++)
	PaletteAlloc [i] = true;
  }
  else
  {
    pfmt.PalEntries = 0;
    pfmt.RedMask = ipfmt->RedMask;
    pfmt.GreenMask = ipfmt->GreenMask;
    pfmt.BlueMask = ipfmt->RedMask;
    // For other than software renderer falling back on software texture 
    // buffers
    if (ipfmt->PixelBytes == 2)
    {
      Depth = 16;
      pfmt.PixelBytes = 2;
      _DrawPixel = DrawPixel16;
      _WriteChar = WriteChar16;
      _GetPixelAt = GetPixelAt16;

      if (flags)
      {
	// Here we are in a software context while sharing the texture manager
	// We therefor render to a 16bit frame buffer and then unpack into an 
	// RGBPixel format from which the texture manager recalculates the 
	// texture
	Memory = new unsigned char[width*height*2];
	image_buffer = (RGBPixel*) buffer;
      }
    } 
    else
    {
      Depth = 32;
      pfmt.PixelBytes = 4;
      _DrawPixel = DrawPixel32;
      _WriteChar = WriteChar32;
      _GetPixelAt = GetPixelAt32;
    }
  }

  pfmt.complete ();



  // Get the font server
  const char *p = System->ConfigGetStr ("FontServer", CS_FUNCID_FONT, 
					"crystalspace.font.server.csfont");
  FontServer = LOAD_PLUGIN (System, p, CS_FUNCID_FONT, iFontServer);
  Font = 0;

  return (iGraphics2D*)this;
}

bool csDynamicTextureSoft2D::Open(const char *Title)
{
  CsPrintf (MSG_INITIALIZATION, "Crystal Space dynamic texture buffer\n");

  // Open your graphic interface
  if (!csGraphics2D::Open (Title))
    return false;

  Clear (0);
  return true;
}

void csDynamicTextureSoft2D::Close ()
{
  // These arrays are shared with the texture, the texture will destroy them.
  Palette = NULL;
  Memory = NULL;
  csGraphics2D::Close ();
}

void csDynamicTextureSoft2D::Print (csRect*)
{
  if (image_buffer)
  {
    // Here we unpack the frame buffer into the image_buffer
    UShort *mem = (UShort*) Memory;
    RGBPixel *im = image_buffer;
    for (int i = 0; i < Width*Height; i++, im++, mem++)
    {
      im->red = ((*mem & pfmt.RedMask) >> pfmt.RedShift) << (8 - pfmt.RedBits);
      im->green = ((*mem & pfmt.GreenMask) >> pfmt.GreenShift) 
					   << (8 - pfmt.GreenBits);
      im->blue = ((*mem & pfmt.BlueMask) >> pfmt.BlueShift) 
					 << (8 - pfmt.BlueBits);
      
    }
  }
}
