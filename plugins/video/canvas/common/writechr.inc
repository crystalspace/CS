/*
    Copyright (C) 1998 by Jorrit Tyberghein
    The WriteChar routine in a separate include file

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
  int ff = This->GetFontID ();
  iFontServer *FS = This->GetFontServer ();
  int charH = FS->GetCharHeight (ff, c);
  int charW = FS->GetCharWidth (ff, c);

  if ((x + charW <= This->ClipX1) || (x >= This->ClipX2)
   || (y + charH <= This->ClipY1) || (y >= This->ClipY2))
    return;

  register unsigned char *CharImage = FS->GetCharBitmap (ff, c);

  if ((x < This->ClipX1) || (x + charW > This->ClipX2)
   || (y < This->ClipY1) || (y + charH > This->ClipY2))
  {
    // Perform full clipping
    int lX = x < This->ClipX1 ? This->ClipX1 - x : 0;
    int rX = x + charW >= This->ClipX2 ? This->ClipX2 - x : charW;
    int lBytes = lX >> 3;
    int shiftX = lX & 7;
    int bbl = (charW + 7) / 8; // bytes per line
    x += lX;
    unsigned char *p = CharImage - bbl;
    for (int i = 0; i < charH; i++, y++)
    {
      p += bbl;
      if ((y < This->ClipY1) || (y >= This->ClipY2)) continue;
      CharImage = p + lBytes;
      register char CharLine = (*CharImage) << shiftX;
      register WRITECHAR_PIXTYPE *VRAM = (WRITECHAR_PIXTYPE *)This->GetPixelAt (x, y);
      for (int j = lX; j < rX; j++)
      {
        if (CharLine & 0x80)
          *VRAM++ = fg;
        else if (bg >= 0)
          *VRAM++ = bg;
        else
          VRAM++;
        if ((j & 7) == 7)
          CharLine = (*++CharImage);
        else
          CharLine <<= 1;
      } /* endfor */
    } /* endfor */
  }
  else
  {
    register char CharLine = *CharImage;
    bool CharNotOnByteBound = charW & 7;
    for (int i = 0; i < charH; i++, y++)
    {
      register WRITECHAR_PIXTYPE *VRAM = (WRITECHAR_PIXTYPE *)This->GetPixelAt (x, y);
      if (bg < 0)
        for (int j = 0; j < charW; j++)
        {
          if (CharLine & 0x80)
            *VRAM++ = fg;
          else
            VRAM++;
          if ((j & 7) == 7)
            CharLine = *++CharImage;
          else
            CharLine <<= 1;
        }
      else
        for (int j = 0; j < charW; j++)
        {
          if (CharLine & 0x80)
            *VRAM++ = fg;
          else
            *VRAM++ = bg;
          if ((j & 7) == 7)
            CharLine = *++CharImage;
          else
            CharLine <<= 1;
        } /* endfor */
      if (CharNotOnByteBound)
        CharLine = *++CharImage;
    } /* endfor */
  } /* endif */
}

#undef WRITECHAR_NAME
#undef WRITECHAR_PIXTYPE
