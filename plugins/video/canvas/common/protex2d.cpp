/*
    Copyright (C) 2000 by Samuel Humphreys

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
#include "protex2d.h"
#include "csutil/scf.h"
#include "isystem.h"

DECLARE_FACTORY (csProcTextureSoft2D)
IMPLEMENT_IBASE (csProcTextureSoft2D)
  IMPLEMENTS_INTERFACE (iPlugIn)
  IMPLEMENTS_INTERFACE (iGraphics2D)
IMPLEMENT_IBASE_END

// csProcTextureSoft2D functions
csProcTextureSoft2D::csProcTextureSoft2D (iSystem *isys) :
  csGraphics2D ()
{
  CONSTRUCT_IBASE (NULL);
  System = isys;
  image_buffer = NULL;
  destroy_memory = false;
}

csProcTextureSoft2D::~csProcTextureSoft2D ()
{
  if (destroy_memory)
    delete [] Memory;
}

iGraphics2D *csProcTextureSoft2D::CreateOffScreenCanvas 
  (int width, int height, void *buffer, csOffScreenBuffer hint, 
   csPixelFormat *ipfmt, RGBPixel *palette, int pal_size)
{
  Width = width;
  Height = height;
  FullScreen = false;
  Memory = (unsigned char*) buffer;

  // Four ways into this routine:
  // 1. via Opengl as a software renderer. Here we do not share resources
  // and the renderer renders at Opengl preferred format.
  //
  // 2. via Opengl as a software renderer, but sharing texture handles..it has
  // its own texture manager but retains a csVector relating the opengl 
  // texture handles to its own.
  //
  // 3. via Software drivers as a stand alone. Here we can render in (internal
  // format of the texture manager) 8bit.
  //
  // 4. via Software drivers sharing the texture manager and resources
  // with its parent driver, here we render at screen pfmt.


  if ((hint == csosbSoftwareAlone) || 
      ((hint == csosbSoftware) && (ipfmt->PixelBytes == 1)))
  {
    // We are software stand alone implementation, or software shared
    // implementation at 8bit.
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

    pfmt.complete ();
  }
  else
  {
    // We are a stand alone software renderer being used in hardware 
    // or a software shared implementation
    memcpy (&pfmt, ipfmt, sizeof (csPixelFormat));
    if (ipfmt->PixelBytes == 2)
    {
      // 16bit shared software or hardware
      Depth = 16;
      _DrawPixel = DrawPixel16;
      _WriteChar = WriteChar16;
      _GetPixelAt = GetPixelAt16;

      if (hint == csosbSoftware)
      {
	// Here we are in a software context while sharing the texture manager
	// We therefor render to a 16bit frame buffer and then unpack into an 
	// RGBPixel format from which the texture manager recalculates the 
	// texture
	destroy_memory = true;
	Memory = new unsigned char[width*height*2];

	image_buffer = (RGBPixel*) buffer;

	UShort *dst = (UShort*)Memory;
	UShort bb = 8 - pfmt.BlueBits;
	UShort gb = 8 - pfmt.GreenBits;
	UShort rb = 8 - pfmt.RedBits;
	for (int i = 0; i < width*height; i++, dst++)
	  *dst = ((((UShort)image_buffer[i].blue >> bb) << pfmt.BlueShift) +
		  (((UShort)image_buffer[i].green >> gb) << pfmt.GreenShift) +
		  (((UShort)image_buffer[i].red >> rb) << pfmt.RedShift));
      
      }
    } 
    else
    {
      // 32bit shared software or hardware
      Depth = 32;
      _DrawPixel = DrawPixel32;
      _WriteChar = WriteChar32;
      _GetPixelAt = GetPixelAt32;

      destroy_memory = true;
      Memory = new unsigned char[width*height*4];
      image_buffer = (RGBPixel*) buffer;
      ULong *dst = (ULong*) Memory;
      for (int i = 0; i < width*height; i++, dst++)
	*dst = (image_buffer[i].red << pfmt.RedShift) +
	  (image_buffer[i].green << pfmt.GreenShift) +
	  (image_buffer[i].blue << pfmt.BlueShift);
    }
  }

  // Get the font server, as we've bypassed csGraphics2D::Initialize
  FontServer = QUERY_PLUGIN_ID (System, CS_FUNCID_FONT, iFontServer);
  Font = 0;

  return (iGraphics2D*)this;
}

bool csProcTextureSoft2D::Open(const char *Title)
{
  // Open the graphic interface
  return csGraphics2D::Open (Title);
}

void csProcTextureSoft2D::Close ()
{
  // These arrays are shared with the texture, the texture will destroy them.
  Palette = NULL;
  // the font server is DecRefed in csGraphics2D
  csGraphics2D::Close ();
  System->QueueContextCloseEvent ((void*)this);
}

void csProcTextureSoft2D::Print (csRect*)
{
  if (image_buffer)
  {
    RGBPixel *dst = image_buffer;
    if (pfmt.PixelBytes == 2)
    {
      // As we are in 16bit mode we unpack the 16 bit frame buffer into 
      // the 32 bit image_buffer as this is the format required by the 
      // quantization routines in the texture manager.
      UShort *src = (UShort*) Memory;
      UShort bb = 8 - pfmt.BlueBits;
      UShort gb = 8 - pfmt.GreenBits;
      UShort rb = 8 - pfmt.RedBits;
      for (int i = 0; i < Width*Height; i++, src++, dst++)
      {
	dst->red = ((*src & pfmt.RedMask) >> pfmt.RedShift) << rb;
	dst->green = ((*src & pfmt.GreenMask) >> pfmt.GreenShift) << gb;
	dst->blue = ((*src & pfmt.BlueMask) >> pfmt.BlueShift) << bb; 
      }
    }
    // 32bit byte shuffle hmmm...if only RGBPixel ordered its red,green,blue,alpha
    // members differently....
    else
    {
      ULong *src = (ULong *)Memory;
      for (int i = 0; i < Width*Height; i++, src++, dst++)
      {
	dst->red = ((*src & pfmt.RedMask) >> pfmt.RedShift);
	dst->green = ((*src & pfmt.GreenMask) >> pfmt.GreenShift);
	dst->blue = ((*src & pfmt.BlueMask) >> pfmt.BlueShift);
      }
    }
  }
}

