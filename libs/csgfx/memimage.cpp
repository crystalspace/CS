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
#include "csutil/debug.h"

csImageMemory::csImageMemory (int width, int height, int format)
  : csImageFile (format)
{
  DG_TYPE (this, "csImageMemory");
  Width = width;
  Height = height;
  switch (format & CS_IMGFMT_MASK)
  {
    case CS_IMGFMT_PALETTED8:
      Image = (void*) new uint8[Width*Height];
      if (format & CS_IMGFMT_ALPHA)
      {
        Alpha =  new uint8[Width*Height];
      }
      Palette = new csRGBpixel[256];
      break;
    case CS_IMGFMT_TRUECOLOR:
      Image = (void*) new csRGBpixel[Width*Height];
      break;
  }
  short_cut = true;
  destroy_image = true;
}

csImageMemory::csImageMemory (int width, int height, void *buffer, bool destroy,
			      int format, csRGBpixel *palette)
  : csImageFile (format)
{
  DG_TYPE (this, "csImageMemory");
  Width = width;
  Height = height;
  Image = buffer;
  Palette = palette;
  short_cut = false;
  destroy_image = destroy;
}

void csImageMemory::FreeImage ()
{
  if (!destroy_image)
  {
    // Clear the 'Image' and 'Palette' pointers so that they won't be
    // deallocated later.
    Image = 0;
    Palette = 0;
  }
  csImageFile::FreeImage();
}

csImageMemory::~csImageMemory ()
{
  if (!destroy_image)
  {
    // Before the csImageFile destructor fires we first
    // clear the 'Image' and 'Palette' pointers so that 
    // it will not try to deallocate.
    Image = 0;
    Palette = 0;
  }
}

void csImageMemory::Clear (const csRGBpixel &colour)
{
  if ((Format & CS_IMGFMT_MASK) != CS_IMGFMT_TRUECOLOR) return;
  uint32 *src = (uint32*) &colour;
  uint32 *dst = (uint32*)Image;

  int i;
  for (i = 0; i < Width*Height; i++, dst++)
    *dst = *src;
}

// short cut
void csImageMemory::Rescale (int NewWidth, int NewHeight)
{
#if 0
  if (short_cut)
  {
    Width = NewWidth;
    Height = NewHeight;
    delete [] (csRGBpixel *) Image;
    Image = (void*) new csRGBpixel[Width*Height];
  }
  else
#endif
    csImageFile::Rescale (NewWidth, NewHeight);
}

void csImageMemory::SetKeyColor (int r, int g, int b)
{
  has_keycolour = true;
  keycolour.Set (r, g, b);
}

void csImageMemory::ClearKeyColor ()
{
  has_keycolour = false;
}

void csImageMemory::ApplyKeyColor ()
{
  if (has_keycolour 
    && ((Format & CS_IMGFMT_MASK) == CS_IMGFMT_PALETTED8))
  {
    uint8* imageData = (uint8*)Image;
    uint8* imagePtr;
    const int pixcount = Width * Height;
    int i;

    // Find out what colors in the palette are actually used
    bool usedEntries[256];
    memset (usedEntries, 0, sizeof (usedEntries));
    int remainingEntries = 256;
    imagePtr = imageData;
    for (i = 0; (i < pixcount) && (remainingEntries > 0); i++)
    {
      if (!usedEntries[*imagePtr])
      {
	usedEntries[*imagePtr] = true;
	remainingEntries--;
      }
      imagePtr++;
    }

    // Find original keycolor index
    int keyIndex = -1;
    for (i = 0; i < 256; i++)
    {
      if (Palette[i].eq (keycolour))
      {
	keyIndex = i;
	break;
      }
    }
    if (keyIndex <= 0)
      return; // We're finished already

    // First, find a palette entry that can take the old color at index 0
    int newIndex = -1;
    for (i = 0; i < 256; i++)
    {
      if (!usedEntries[i])
      {
	newIndex = i;
	break;
      }
    }
    if (newIndex == -1)
    {
      newIndex = closest_index (Palette + 0);
    }
    else
    {
      Palette[newIndex] = Palette[0];
    }
    Palette[0] = keycolour;

    imagePtr = imageData;
    for (i = 0; i < pixcount; i++)
    {
      if (*imagePtr == 0)
	*imagePtr = newIndex;
      else if (*imagePtr == keyIndex)
	*imagePtr = 0;
      imagePtr++;
    }
  }
}
