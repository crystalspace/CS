/*
    Copyright (C) 1998 by Jorrit Tyberghein
    The DrawSprite routine in a separate include file

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

/*
    This is the generalized WriteCharXX routine. Since the only difference
    between WriteChar routines is the datatype of a pixel, the former three
    different routines have been unified into a single file.

    Before including this file you should define the following macros:

    WRITECHAR_NAME - The name of routine (WriteChar8 etc).
    WRITECHAR_PIXTYPE - The type of a pixel (UByte, UShort, ULong)
*/

void csGraphics2D::WRITECHAR_NAME (csGraphics2D *This, int x, int y,
  int fg, int bg, char c)
{
  int charH = FontList [This->Font].Height;
  int charW;
  if (FontList [This->Font].IndividualWidth)
    charW = FontList [This->Font].IndividualWidth[(unsigned char)c];
  else
    charW = FontList [This->Font].Width;

  if ((x + charW <= This->ClipX1) || (x >= This->ClipX2)
   || (y + charH <= This->ClipY1) || (y >= This->ClipY2))
    return;

  register unsigned char *CharImage =
    &FontList [This->Font].FontBitmap[((unsigned char)c) *
     FontList [This->Font].BytesPerChar];

  if ((x < This->ClipX1) || (x + charW > This->ClipX2)
   || (y < This->ClipY1) || (y + charH > This->ClipY2))
  {
    // Perform full clipping
    int lX = x < This->ClipX1 ? This->ClipX1 - x : 0;
    int rX = x + charW >= This->ClipX2 ? This->ClipX2 - x : charW;
    x += lX;
    for (int i = 0; i < charH; i++, y++)
    {
      register char CharLine = (*CharImage++) << lX;
      if ((y < This->ClipY1) || (y >= This->ClipY2)) continue;
      register WRITECHAR_PIXTYPE *VRAM = (WRITECHAR_PIXTYPE *)This->GetPixelAt (x, y);
      for (int j = lX; j < rX; j++)
      {
        if (CharLine & 0x80)
          *VRAM++ = fg;
        else if (bg >= 0)
          *VRAM++ = bg;
        else
          VRAM++;
        CharLine <<= 1;
      } /* endfor */
    } /* endfor */
  } else
  for (int i = 0; i < charH; i++, y++)
  {
    register WRITECHAR_PIXTYPE *VRAM = (WRITECHAR_PIXTYPE *)This->GetPixelAt (x, y);
    register char CharLine = *CharImage++;
    if (bg < 0)
      for (int j = 0; j < charW; j++)
      {
        if (CharLine & 0x80)
          *VRAM++ = fg;
        else
          VRAM++;
        CharLine <<= 1;
      } else
      for (int j = 0; j < charW; j++)
      {
        if (CharLine & 0x80)
          *VRAM++ = fg;
        else
          *VRAM++ = bg;
        CharLine <<= 1;
      } /* endfor */
  } /* endfor */
}

#undef WRITECHAR_NAME
#undef WRITECHAR_PIXTYPE
