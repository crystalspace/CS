/*
    Copyright (C) 1998 by Jorrit Tyberghein
    The WriteString routine in a separate include file

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
    This is the generalized WriteStringXX routine. Since the only difference
    between WriteString routines is the datatype of a pixel, the former three
    different routines have been unified into a single file.

    Before including this file you should define the following macros:

    WR_NAME - The name of routine (WriteString8 etc).
    WR_PIXTYPE - The type of a pixel (UByte, UShort, ULong)
*/

void csGraphics2D::WR_NAME (csGraphics2D *This, int x, int y, int fg, int bg,
  const char *text)
{
  unsigned char c;
  int charW, charH;
  for (c = *text; (c = *text); ++text, x += charW)
  {
    if (x > This->ClipX2)
      return;

    register unsigned char *CharImage = This->FontServer->GetGlyphBitmap (
      This->Font, c, charW, charH);

    // If character is completely outside the clipping rectangle, continue
    if ((x + charW <= This->ClipX1) || (x >= This->ClipX2)
     || (y + charH <= This->ClipY1) || (y >= This->ClipY2))
      continue;

    int cury = y;

    // If character should not be clipped, go the fast path
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
      for (int i = 0; i < charH; i++, cury++)
      {
        p += bbl;
        if ((cury < This->ClipY1) || (cury >= This->ClipY2)) continue;
        CharImage = p + lBytes;
        register char CharLine = (*CharImage) << shiftX;
        register WR_PIXTYPE *VRAM = (WR_PIXTYPE *)This->GetPixelAt (x, cury);
        if (bg >= 0)
          for (int j = lX; j < rX; j++)
          {
            if (CharLine & 0x80)
              *VRAM++ = fg;
            else
              *VRAM++ = bg;
            if ((j & 7) == 7)
              CharLine = (*++CharImage);
            else
              CharLine <<= 1;
          } /* endfor */
        else
          for (int j = lX; j < rX; j++)
          {
            if (CharLine & 0x80)
              *VRAM++ = fg;
            else
              VRAM++;
            if ((j & 7) == 7)
              CharLine = (*++CharImage);
            else
              CharLine <<= 1;
          } /* endfor */
       }
    }
    else
    {
      register char CharLine = *CharImage;
      bool CharNotOnByteBound = charW & 7;
      for (int i = 0; i < charH; i++, cury++)
      {
        register WR_PIXTYPE *VRAM = (WR_PIXTYPE *)This->GetPixelAt (x, cury);
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
}

#undef WR_NAME
#undef WR_PIXTYPE
