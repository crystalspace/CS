/*
    Copyright (C) 2000 by Jorrit Tyberghein
	      (C) 2001 by F.Richter

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
#include "igraphic/image.h"
#include "csgfx/memimage.h"
#include "csgfx/rgbpixel.h"
#include "csqint.h"
#include "csgfx/xorpat.h"

csPtr<iImage> csCreateXORPatternImage(int width, int height, int recdepth,
				      float red, float green, float blue)
{
  int x,y;
  csImageMemory* image = new csImageMemory (width, height);
  csRGBpixel *pixel = (csRGBpixel*)image->GetImagePtr();

  if (recdepth<1) recdepth = 1;
  if (recdepth>8) recdepth = 8;
  int coordmask = ((1<<recdepth)-1);
  int shlpixel = (8-recdepth);
  int valueadd = (1<<shlpixel)-1;
  int valueshr = recdepth-1; 

  for (x=0; x<width; x++)
  {
    for (y=0; y<height; y++)
    {
      unsigned char value = ((x & coordmask) ^ (y & coordmask));
      int v = (value << shlpixel) + ((value >> valueshr) * valueadd);
      pixel->red = csQint (v * red);
      pixel->green = csQint (v * green);
      pixel->blue = csQint (v * blue);
      pixel++;
    }
  }

  csRef<iImage> imageRef;
  imageRef.AttachNew (image);
  return csPtr<iImage> (imageRef);
}


