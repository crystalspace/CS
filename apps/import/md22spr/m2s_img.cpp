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

#include "cssysdef.h"
#include "m2s_img.h"

SCF_IMPLEMENT_IBASE (SkinImage)
  SCF_IMPLEMENTS_INTERFACE (iImage)
SCF_IMPLEMENT_IBASE_END

const void *SkinImage::GetImageData () { return Image; }
int SkinImage::GetWidth () const { return Width; }
int SkinImage::GetHeight () const { return Height; }
int SkinImage::GetFormat () const { return CS_IMGFMT_PALETTED8; }
const csRGBpixel *SkinImage::GetPalette () { return Palette; }

SkinImage::SkinImage(void* Data, const void* Pal, int w, int h) :
  Image(Data), Width(w), Height(h), Size(w * h)
{
  SCF_CONSTRUCT_IBASE(0);
  const unsigned char* src = (const unsigned char*)Pal;
  csRGBpixel* dst = Palette;
  int i;
  for (i = 256; i-- > 0; dst++)
  {
    dst->red   = *src++;
    dst->green = *src++;
    dst->blue  = *src++;
    dst->alpha = 0xff;
  }
}
