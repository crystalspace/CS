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

iImage* csCreateXORPatternImage(int width, int height, int patsize) {
  int x,y;
  iImage *image = new csImageMemory(width, height);
  csRGBpixel *pixel = (csRGBpixel*)image->GetImageData();

  if (patsize>8) patsize = 8;

  for (x=0; x<width; x++) {
    for (y=0; y<width; y++) {
      pixel->red = pixel->green = pixel->blue = ((x & ((1<<patsize)-1)) ^ (y & ((1<<patsize)-1))) << (8-patsize); 
      pixel++;
    }
  }

  return image;
}


