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
#include "cssysdef.h"
#include "csgfx/memimage.h"
#include "csgfx/rgbpixel.h"

csImageMemory::csImageMemory (int width, int height)
  : csImageFile (CS_IMGFMT_TRUECOLOR)
{
  CONSTRUCT_IBASE (NULL);
  Width = width;
  Height = height;
  Image = (void*) new csRGBpixel[Width*Height];
  short_cut = true;
  destroy_image = true;
}

csImageMemory::csImageMemory (int width, int height, csRGBpixel *buffer, bool destroy)
  : csImageFile (CS_IMGFMT_TRUECOLOR)
{
  CONSTRUCT_IBASE (NULL);
  Width = width;
  Height = height;
  Image = buffer;
  short_cut = false;
  destroy_image = destroy;
}

csImageMemory::~csImageMemory ()
{
  if (!destroy_image)
    Image = NULL;
}

void csImageMemory::Clear (const csRGBpixel &colour)
{
  ULong *src = (ULong*) &colour;
  ULong *dst = (ULong*)Image;
  for (int i = 0; i < Width*Height; i++, dst++)
    *dst = *src;
}

// short cut
void csImageMemory::Rescale (int NewWidth, int NewHeight)
{
  if (short_cut)
  {
    Width = NewWidth;
    Height = NewHeight;
    delete [] (csRGBpixel *) Image;
    Image = (void*) new csRGBpixel[Width*Height];
  }
  else
    csImageFile::Rescale (NewWidth, NewHeight);
}
