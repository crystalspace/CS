/*  -*- Mode: C++; -*- */
/*
    Copyright (C) 1998-2003 by Jorrit Tyberghein
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

    WS_NAME - The name of routine (WriteString8 etc).
    WS_PIXTYPE - The type of a pixel (uint8, uint16, uint32)
    WS_ALPHA_AVAILABLE - Whether alpha blending is available
    WS_A_R5G5B5,
    WS_A_R5G6B5,
    WS_A_R8G8B8 - Pixel format (only req. for alpha blending)
*/

#ifdef WS_ALPHA_AVAILABLE

#if defined (WS_A_R5G5B5)
#  define WS_A_MASK_RB   0x00007c1f
#  define WS_A_MASK_G    0x000003e0
#  define WS_A_ALPHASHIFT	  3
#  define WS_A_NORMSHIFT	  5
#elif defined (WS_A_R5G6B5)
#  define WS_A_MASK_RB	0x0000f81f
#  define WS_A_MASK_G	0x000007e0
#  define WS_A_ALPHASHIFT	  3
#  define WS_A_NORMSHIFT	  5
#elif defined (WS_A_R8G8B8)
#  define WS_A_MASK_RB	0x00ff00ff
#  define WS_A_MASK_G	0x0000ff00
#  define WS_A_ALPHASHIFT	  0
#  define WS_A_NORMSHIFT	  8
#else
#  error "ERROR: Pixel format not defined before including writechr.inc!"
#endif


#define WS_ALPHABLEND(src, dest, alpha)					\
      unsigned dst = *dest;						\
      unsigned alph = alpha >> WS_A_ALPHASHIFT;				\
      unsigned dst_rb = dst & WS_A_MASK_RB;				\
      dst_rb = dst_rb + ((alph * (fg_rb - dst_rb)) >> WS_A_NORMSHIFT);	\
      unsigned dst_g = dst & WS_A_MASK_G;				\
      dst_g = dst_g + ((alph * (fg_g - dst_g)) >> WS_A_NORMSHIFT);	\
      *dest = (dst_rb & WS_A_MASK_RB) | (dst_g & WS_A_MASK_G);		\

#define WS_ALPHABLEND_BG(src, dest, alpha)				\
      unsigned alph = alpha >> WS_A_ALPHASHIFT;				\
      unsigned dst_rb = bg_rb + ((alph * diff_rb) >> WS_A_NORMSHIFT);	\
      unsigned dst_g = bg_g + ((alph * diff_g) >> WS_A_NORMSHIFT);	\
      *dest = (dst_rb & WS_A_MASK_RB) | (dst_g & WS_A_MASK_G);		\

#define WS_PREPARE_ALPHABLEND(fg, bg)					\
      unsigned fg_rb = fg & WS_A_MASK_RB;				\
      unsigned fg_g = fg & WS_A_MASK_G;					\
      unsigned bg_rb = bg & WS_A_MASK_RB;				\
      unsigned bg_g = bg & WS_A_MASK_G;					\
      unsigned diff_rb = fg_rb - bg_rb;					\
      unsigned diff_g = fg_g - bg_g;					\

#endif

void WS_NAME (iFont *font, int pen_x, int pen_y,
  int fg, int bg, const utf8_char *text, uint flags)
{
  if (!font)
    return;
  
  if (!(flags & CS_WRITE_BASELINE)) pen_y += font->GetAscent ();

  KnownFont* knownFont = GetCachedFont (font);
  if (knownFont == 0) knownFont = CacheFont (font);

#ifdef WS_ALPHA_AVAILABLE
  WS_PREPARE_ALPHABLEND(fg, bg)
#endif

  int textLen = strlen ((char*)text);
  int charW, charH, advance = 0;
  bool firstchar = true;
  while (textLen > 0)
  {
    utf32_char glyph;
    int skip = csUnicodeTransform::UTF8Decode (text, textLen, glyph, 0);
    if (skip == 0) break;

    text += skip;
    textLen -= skip;

    SoftGlyphCacheData* cacheData = (SoftGlyphCacheData*)CacheGlyph 
      (knownFont, glyph, flags);
    if (!cacheData->hasGlyph) 
    {
      // fall back to the default glyph (CS_FONT_DEFAULT_GLYPH)
      cacheData = (SoftGlyphCacheData*)CacheGlyph (knownFont, 
        CS_FONT_DEFAULT_GLYPH, flags);
      if (!cacheData->hasGlyph) continue;
    }

#ifdef WS_ALPHA_AVAILABLE
    register uint8 *CharImageAlpha = cacheData->glyphAlphaData;
#else
    const int CharImageAlpha = 0;
#endif
    register uint8 *CharImage = cacheData->glyphData;
    if ((!CharImage) && (!CharImageAlpha))
      continue;

    csBitmapMetrics* bmetrics;
    if (CharImageAlpha)
    {
      bmetrics = &cacheData->alphaMetrics;
    }
    else
    {
      bmetrics = &cacheData->bitmapMetrics;
    }
    charW = bmetrics->width;
    charH = bmetrics->height;
    
    int y = pen_y - bmetrics->top;
    
    // If we are advancing more than the last char was wide, we have to
    // fill the 'gap' with bg.
    
    int x = pen_x - (advance > 0 ? advance : 0) + (bmetrics->left < 0 ? bmetrics->left : 0);
    advance += bmetrics->left;
    
    // Hack: in case the first char has a negative left bitmap offset,
    // some of the background isn't drawn. Fix that.
    if (firstchar)
    {
      if (advance < 0)
      {
	advance = 0;
      }
      firstchar = false;
    }
   
    if (bg >= 0)
    {
      while (advance > 0)
      {
	if (x >= ClipX2)
	  return;
  
	int cury = y;
	for (int i = 0; i < charH; i++, cury++)
	{
	  if ((cury < ClipY1) || (cury >= ClipY2)) continue;
	  register WS_PIXTYPE *VRAM = (WS_PIXTYPE *)G2D->GetPixelAt (x, cury);
	  if (x >= ClipX1) *VRAM = bg;
	}
	x++; advance--;
      }
    }
    else
    {
      if (advance > 0)
      {
	x += advance;
	advance = 0;
      }
    }

    if (x >= ClipX2)
      return;

    // If character is completely outside the clipping rectangle, continue
    if (!((x + charW <= ClipX1) || (x >= ClipX2)
     || (y + charH <= ClipY1) || (y >= ClipY2)))
    {
      int cury = y;
  
      int oldAdvance = advance;
      // If character should not be clipped, go the fast path
      if ((x < ClipX1) || (x + charW >= ClipX2)
       || (y < ClipY1) || (y + charH >= ClipY2))
      {
	// Perform full clipping
	int lX = x < ClipX1 ? ClipX1 - x : 0;
	int rX = x + charW >= ClipX2 ? ClipX2 - x : charW;
  #ifdef WS_ALPHA_AVAILABLE
	int lBytes = CharImageAlpha ? lX : lX >> 3;
	int shiftX = CharImageAlpha ? 0 : lX & 7;
	int bbl = CharImageAlpha ? charW : (charW + 7) / 8; // bytes per line
	int lAbsX = x + lX;
	uint8 *p = CharImageAlpha ? CharImageAlpha - bbl : CharImage - bbl;
  #else
	int lBytes = lX >> 3;
	int shiftX = lX & 7;
	int bbl = (charW + 7) / 8; // bytes per line
	int lAbsX = x + lX;
	uint8 *p = CharImage - bbl;
  #endif
	
  #ifdef WS_ALPHA_AVAILABLE
	if (CharImageAlpha)
	{
	  for (int i = 0; i < charH; i++, cury++)
	  {
	    advance = oldAdvance;
	    p += bbl;
	    if ((cury < ClipY1) || (cury >= ClipY2)) 
	    {
	      if (advance < 0) advance = MIN(0, advance + (rX - lX));
	      continue;
	    }
	    CharImageAlpha = p + lBytes;
	    register uint8 CharLine = (*CharImageAlpha++) << shiftX;
	    register WS_PIXTYPE *VRAM = (WS_PIXTYPE *)G2D->GetPixelAt (lAbsX, cury);
	    // If we are advancing less than the last char was wide, the current
	    // and last chars overlap. So we can't draw opaque, but have to draw
	    // transparent instead.
	    if ((bg >= 0) && (advance >= 0))
	    {
	      for (int j = lX; j < rX; j++)
	      {
		if (CharLine == 0xff)
		{
		  *VRAM = fg;
		}
		else if (CharLine == 0x00)
		{
		  *VRAM = bg;
		}
		else
		{
		  WS_ALPHABLEND_BG(fg, VRAM, CharLine)
		}
		VRAM++;
		if (j < rX-1) CharLine = (*CharImageAlpha++);
	      } /* endfor */
	    }
	    else
	    {
	      for (int j = lX; j < rX; j++)
	      {
		if (CharLine == 0xff)
		{
		  *VRAM = fg;
		}
		else if (CharLine != 0x00)
		{
		  WS_ALPHABLEND(fg, VRAM, CharLine)
		}
		VRAM++;
		if (j < rX-1) CharLine = (*CharImageAlpha++);
	      } /* endfor */
	      if (advance < 0) advance++;
	    }
	  }
	}
	else
  #endif
	{
	  for (int i = 0; i < charH; i++, cury++)
	  {
	    advance = oldAdvance;
	    p += bbl;
	    if ((cury < ClipY1) || (cury >= ClipY2))
	    {
	      if (advance < 0) advance = MIN(0, advance + (rX - lX));
	      continue;
	    }
	    CharImage = p + lBytes;
	    register uint8 CharLine = (*CharImage++) << shiftX;
	    register WS_PIXTYPE *VRAM = (WS_PIXTYPE *)G2D->GetPixelAt (lAbsX, cury);
	    if (bg >= 0)
	    {
	      for (int j = lX; j < rX; j++)
	      {
		if (advance >= 0)
		{
		  *VRAM++ = (CharLine & 0x80) ? fg : bg;
		}
		else
		{
		  if (CharLine & 0x80)
		    *VRAM++ = fg;
		  else
		    VRAM++;
		  advance++;
		}
		if ((j & 7) == 7)
		  CharLine = (*CharImage++);
		else
		  CharLine += CharLine;
	      } /* endfor */
	    }
	    else
	    {
	      for (int j = lX; j < rX; j++)
	      {
		if (CharLine & 0x80)
		  *VRAM++ = fg;
		else
		  VRAM++;
		if ((j & 7) == 7)
		  CharLine = (*CharImage++);
		else
		  CharLine += CharLine;
		if (advance < 0) advance++;
	      } /* endfor */
	    }
	  }
	}
      }
      else
      {
	// no clipping
  #ifdef WS_ALPHA_AVAILABLE
	if (CharImageAlpha)
	{
	  for (int i = 0; i < charH; i++, cury++)
	  {
	    advance = oldAdvance;
	    register WS_PIXTYPE *VRAM = (WS_PIXTYPE *)G2D->GetPixelAt (x, cury);
	    register unsigned pixW = charW;
	    register int pix;
	    for (pix = pixW; pix > 0; pix--)
	    {
	      register uint8 CharLine = (*CharImageAlpha++);
	      if ((bg < 0) || (advance < 0))
	      {
		if (CharLine == 0xff)
		{
		  *VRAM = fg;
		}
		else if (CharLine != 0x00)
		{
		  WS_ALPHABLEND(fg, VRAM, CharLine)
		}
	      }
	      else
	      {
		if (CharLine == 0xff)
		{
		  *VRAM = fg;
		}
		else if (CharLine == 0x00)
		{
		  *VRAM = bg;
		}
		else
		{
		  WS_ALPHABLEND_BG(fg, VRAM, CharLine)
		}
	      }
	      if (advance < 0) advance++;
	      VRAM++;
	    }
	  }
	}
	else
  #endif
	{
	  for (int i = 0; i < charH; i++, cury++)
	  {
	    register WS_PIXTYPE *VRAM = (WS_PIXTYPE *)G2D->GetPixelAt (x, cury);
	    register unsigned pixW = charW;
	    while (pixW)
	    {
	      register unsigned char CharLine = *CharImage++;
	      register int pix;
	      for (pix = pixW < 8 ? pixW : 8, pixW -= pix; CharLine && pix; pix--)
	      {
		if ((bg < 0) || (advance < 0))
		{
		  if (CharLine & 0x80)
		    *VRAM++ = fg;
		  else
		    VRAM++;
		  // Addition is faster than shift, at least on i586+
		  CharLine += CharLine;
		}
		else
		{
		  *VRAM++ = (CharLine & 0x80) ? fg : bg;
		  // Addition is faster than shift, at least on i586+
		  CharLine += CharLine;
		}
		if (advance < 0) advance++;
	      } /* endfor */
	      if (advance < 0)
	      {
		VRAM -= advance;
		pix += advance;
	      }
	      if (bg < 0)
	      {
		VRAM += pix;
	      }
	      else
	      {
		while (pix--)
		  *VRAM++ = bg;
	      }
	    } 
	  } 
	}
      } /* endif */
    }

    pen_x += cacheData->glyphMetrics.advance;
    advance += cacheData->glyphMetrics.advance - (charW + bmetrics->left);
  }
  PurgeEmptyPlanes ();
}

#undef WS_NAME
#undef WS_NAME2
#undef WS_PIXTYPE
#undef WS_ALPHA_AVAILABLE
#undef WS_A_R5G5B5
#undef WS_A_R5G6B5
#undef WS_A_R8G8B8
#undef WS_A_MASK_RB
#undef WS_A_MASK_G
#undef WS_A_ALPHASHIFT
#undef WS_A_NORMSHIFT
