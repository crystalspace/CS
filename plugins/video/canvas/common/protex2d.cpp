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
#include "csgeom/csrect.h"
#include "iutil/plugin.h"
#include "iutil/event.h"
#include "iutil/eventq.h"
#include "iutil/objreg.h"
#include "iutil/eventh.h"
#include "iutil/comp.h"

csProcTextureSoft2D::csProcTextureSoft2D (iObjectRegistry *r)
	: csGraphics2D (NULL)
{
  object_reg = r;
  image_buffer = NULL;
  destroy_memory = false;
}

csProcTextureSoft2D::~csProcTextureSoft2D ()
{
  if (destroy_memory)
    delete [] Memory;
  if (Depth == 8)
    Palette = NULL;
}

iGraphics2D *csProcTextureSoft2D::CreateOffScreenCanvas 
  (int width, int height, void *buffer, bool use8bit, 
   csPixelFormat *ipfmt, csRGBpixel *palette, int pal_size)
{
  Width = width;
  Height = height;
  FullScreen = false;
  Memory = (unsigned char*) buffer;
  int i;

  // Three ways into this routine:
  // 1. via Opengl as a software renderer, but sharing texture handles..it has
  // its own texture manager but retains a csVector relating the opengl 
  // texture handles to its own.
  //
  // 2. via Software drivers as a stand alone. Here we can render in (internal
  // format of the texture manager) 8bit.
  //
  // 3. via Software drivers sharing the texture manager and resources
  // with its parent driver, here we render at screen pfmt.

  if (use8bit || (ipfmt->PixelBytes == 1))
  {
    // We are software stand alone implementation, or software shared
    // implementation at 8bit.
    Depth = 8;
    pfmt.PalEntries = pal_size;
    pfmt.PixelBytes = 1;

    // Initialize pointers to default drawing methods
    _DrawPixel = DrawPixel8;
    _WriteString = WriteString8;
    _GetPixelAt = GetPixelAt8;

    Palette = palette;
    pfmt.RedMask = 0;
    pfmt.GreenMask = 0;
    pfmt.BlueMask = 0;

    for (i = 0; i < 256; i++)
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
      _WriteString = WriteString16;
      _GetPixelAt = GetPixelAt16;

      // Here we are in a software context while sharing the texture manager
      // We therefor render to a 16bit frame buffer and then unpack into an 
      // csRGBpixel format from which the texture manager recalculates the 
      // texture
      destroy_memory = true;
      Memory = new unsigned char[width*height*2];

      image_buffer = (csRGBpixel*) buffer;

      UShort *dst = (UShort*)Memory;
      UShort bb = 8 - pfmt.BlueBits;
      UShort gb = 8 - pfmt.GreenBits;
      UShort rb = 8 - pfmt.RedBits;
      for (i = 0; i < width*height; i++, dst++)
	*dst = ((((UShort)image_buffer[i].blue >> bb) << pfmt.BlueShift) +
		(((UShort)image_buffer[i].green >> gb) << pfmt.GreenShift) +
		(((UShort)image_buffer[i].red >> rb) << pfmt.RedShift));
    } 
    else
    {
      // 32bit shared software or hardware
      Depth = 32;
      _DrawPixel = DrawPixel32;
      _WriteString = WriteString32;
      _GetPixelAt = GetPixelAt32;

      destroy_memory = true;
      Memory = new unsigned char[width*height*4];
      image_buffer = (csRGBpixel*) buffer;
      ULong *dst = (ULong*) Memory;
      for (i = 0; i < width*height; i++, dst++)
	*dst = (image_buffer[i].red << pfmt.RedShift) +
	  (image_buffer[i].green << pfmt.GreenShift) +
	  (image_buffer[i].blue << pfmt.BlueShift);
    }
  }

  // Get the font server, as we've bypassed csGraphics2D::Initialize
  FontServer = CS_QUERY_REGISTRY (object_reg, iFontServer);

  return (iGraphics2D*)this;
}

void csProcTextureSoft2D::Close ()
{
  // The font server is DecRefed in csGraphics2D
  // These arrays are shared with the texture, the texture will destroy them.
  Palette = NULL;
  csGraphics2D::Close ();
  iEventQueue* q = CS_QUERY_REGISTRY(object_reg, iEventQueue);
  if (q != 0)
  {
    q->GetEventOutlet()->Broadcast (cscmdContextClose, this);
    q->DecRef ();
  }
}

void csProcTextureSoft2D::Print (csRect *area)
{
  int i,j;
  if (image_buffer)
  {
    csRGBpixel *dst = image_buffer;
    if (pfmt.PixelBytes == 2)
    {
      // As we are in 16bit mode we unpack the 16 bit frame buffer into 
      // the 32 bit image_buffer as this is the format required by the 
      // quantization routines in the texture manager.
      UShort *src = (UShort*) Memory;

      int rb = 8 - pfmt.RedBits;
      int gb = 8 - pfmt.GreenBits;
      int bb = 8 - pfmt.BlueBits;

      if (area)
      {
	int stride = area->xmin + Width - area->xmax;
	int offset = area->ymin*Width + area->xmin;
	src += offset;
	dst += offset;
	for (i = area->ymin; i < (area->ymax+1); i++)
	{
	  for (j = area->xmin; j < area->xmax; j++, src++, dst++)
	  {
	    dst->red = ((*src & pfmt.RedMask) >> pfmt.RedShift) << rb;
	    dst->green = ((*src & pfmt.GreenMask) >> pfmt.GreenShift) << gb;
	    dst->blue = ((*src & pfmt.BlueMask) >> pfmt.BlueShift) << bb; 
	  }
	  src += stride;
	  dst += stride;
	}
      }
      else
      {
	for (i = 0; i < Width*Height; i++, src++, dst++)
	{
	  dst->red = ((*src & pfmt.RedMask) >> pfmt.RedShift) << rb;
	  dst->green = ((*src & pfmt.GreenMask) >> pfmt.GreenShift) << gb;
	  dst->blue = ((*src & pfmt.BlueMask) >> pfmt.BlueShift) << bb; 
	}
      }
    }
    else  // 32bit: byte shuffle 
    {
      ULong *src = (ULong *)Memory;
      if (area)
      {
	int stride = area->xmin + Width - area->xmax;
	int offset = area->ymin*Width + area->xmin;
	src += offset;
	dst += offset;
	for (i = area->ymin; i < (area->ymax+1); i++)
	{
	  for (j = area->xmin; j < area->xmax; j++, src++, dst++)
	  {
	    dst->red = ((*src & pfmt.RedMask) >> pfmt.RedShift);
	    dst->green = ((*src & pfmt.GreenMask) >> pfmt.GreenShift);
	    dst->blue = ((*src & pfmt.BlueMask) >> pfmt.BlueShift);
	  }
	  src += stride;
	  dst += stride;
	}
      }
      else
      {
	for (i = 0; i < Width*Height; i++, src++, dst++)
	{
	  dst->red = ((*src & pfmt.RedMask) >> pfmt.RedShift);
	  dst->green = ((*src & pfmt.GreenMask) >> pfmt.GreenShift);
	  dst->blue = ((*src & pfmt.BlueMask) >> pfmt.BlueShift);
	}
      }
    }
  } // end if (image_buffer)
}
