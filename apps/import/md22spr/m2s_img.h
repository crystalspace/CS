/*
  Crystal Space Quake MDL/MD2 convertor
  Copyright (C) 2000 by Eric Sunshine <sunshine@sunshineco.com>

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

#ifndef __M2S_IMG_H__
#define __M2S_IMG_H__

#include "csgfx/imagebase.h"
#include "csgfx/rgbpixel.h"

class SkinImage : public csImageBase
{
protected:
  void* Image;
  csRGBpixel Palette[256];
  int Width, Height, Size;
public:
  SCF_DECLARE_IBASE;
  SkinImage(void* Data, const void* Pal, int w, int h);
  virtual ~SkinImage () { SCF_DESTRUCT_IBASE(); }
  virtual const void *GetImageData ();
  virtual int GetWidth () const;
  virtual int GetHeight () const;
  virtual int GetFormat () const;
  virtual const csRGBpixel *GetPalette ();
};

#endif // __M2S_IMG_H__

