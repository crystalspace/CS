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
#include "csgfxldr/memimage.h"
#include "csgfxldr/rgbpixel.h"

IMPLEMENT_IBASE (csImageMemory)
  IMPLEMENTS_INTERFACE (iImage)
IMPLEMENT_IBASE_END

csImageMemory::csImageMemory (int width, int height)
  : Width (width), Height (height)
{
  CONSTRUCT_IBASE (NULL);
  Data = (void*) new RGBPixel[Width*Height];
};

csImageMemory::~csImageMemory ()
{
  delete [] (RGBPixel *)Data;
}

void csImageMemory::Rescale (int NewWidth, int NewHeight)
{
  Width = NewWidth;
  Height = NewHeight;
  delete [] (RGBPixel *)Data;
  Data = (void*) new RGBPixel[Width*Height];
}
