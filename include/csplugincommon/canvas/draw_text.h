/*
    Copyright (C) 2004 by Jorrit Tyberghein
	      (C) 2004 by Frank Richter

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

#ifndef __CS_CSPLUGINCOMMON_CANVAS_DRAW_TEXT_H__
#define __CS_CSPLUGINCOMMON_CANVAS_DRAW_TEXT_H__

/**\file
 * Software text writing.
 */

#include "csutil/csuctransform.h"
#include "draw_common.h"
#include "csplugincommon/canvas/softfontcache.h"

/**
 * Class to write some text.
 * Needs 3 PixMixers: one for to mix with the foreground color, 
 * one to mix with the background color, one to interpolate between
 * mixed FG and BG.
 */
template<class Tpixel, class Tpixmixer1, class Tpixmixer2, class Tpixmixer3>
class csG2DDrawText
{
public:
  static void DrawText (csSoftFontCache* cache, iFont* font, int pen_x, int pen_y,
    Tpixel fg, uint8 alphaFG, Tpixel bg, uint8 alphaBG, const utf8_char *text, 
    uint flags)
  {
    csGraphics2D* G2D = cache->G2D;
    const int ClipX1 = cache->ClipX1, ClipY1 = cache->ClipY1, 
      ClipX2 = cache->ClipX2, ClipY2 = cache->ClipY2;
    Tpixmixer1 mixerFG (G2D, fg, alphaFG);
    Tpixmixer2 mixerBG (G2D, bg, alphaBG);
    
    if (!font)
      return;
    
    if (!(flags & CS_WRITE_BASELINE)) pen_y += font->GetAscent ();
  
    csSoftFontCache::KnownFont* knownFont = cache->GetCachedFont (font);
    if (knownFont == 0) knownFont = cache->CacheFont (font);
  
    size_t textLen = strlen ((char*)text);
    int charW, charH, advance = 0;
    bool firstchar = true;
    while (textLen > 0)
    {
      utf32_char glyph;
      int skip = csUnicodeTransform::UTF8Decode (text, textLen, glyph, 0);
      if (skip == 0) break;
  
      text += skip;
      textLen -= skip;
  
      csSoftFontCache::SoftGlyphCacheData* cacheData = 
	(csSoftFontCache::SoftGlyphCacheData*)cache->CacheGlyph (knownFont, 
	glyph, flags);
      if (!cacheData->hasGlyph) 
      {
	// fall back to the default glyph (CS_FONT_DEFAULT_GLYPH)
	cacheData = (csSoftFontCache::SoftGlyphCacheData*)cache->CacheGlyph (
	  knownFont, CS_FONT_DEFAULT_GLYPH, flags);
	if (!cacheData->hasGlyph) continue;
      }
  
      register uint8 *CharImageAlpha = cacheData->glyphAlphaData;
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
     
      if (alphaBG != 0)
      {
	while (advance > 0)
	{
	  if (x >= ClipX2)
	    return;
    
	  int cury = y;
	  for (int i = 0; i < charH; i++, cury++)
	  {
	    if ((cury < ClipY1) || (cury >= ClipY2)) continue;
	    register Tpixel *VRAM = (Tpixel*)G2D->GetPixelAt (x, cury);
	    if (x >= ClipX1) mixerBG.Mix (*VRAM);
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
	  int lBytes = CharImageAlpha ? lX : lX >> 3;
	  int shiftX = CharImageAlpha ? 0 : lX & 7;
	  int bbl = CharImageAlpha ? charW : (charW + 7) / 8; // bytes per line
	  int lAbsX = x + lX;
	  uint8 *p = CharImageAlpha ? CharImageAlpha - bbl : CharImage - bbl;
	  
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
	      register Tpixel* VRAM = (Tpixel*)G2D->GetPixelAt (lAbsX, cury);
	      // If we are advancing less than the last char was wide, the current
	      // and last chars overlap. So we can't draw opaque, but have to draw
	      // transparent instead.
	      if (advance >= 0)
	      {
		for (int j = lX; j < rX; j++)
		{
		  if (CharLine == 0xff)
		  {
		    mixerFG.Mix (*VRAM);
		  }
		  else if (CharLine == 0x00)
		  {
		    mixerBG.Mix (*VRAM);
		  }
		  else
		  {
		    // @@@ Could be more optimal, probably.
		    Tpixel mixedFG = *VRAM;
		    mixerFG.Mix (mixedFG);
		    mixerBG.Mix (*VRAM);
		    
		    Tpixmixer3 mixer (G2D, mixedFG, CharLine);
		    mixer.Mix (*VRAM);
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
		    mixerFG.Mix (*VRAM);
		  }
		  else if (CharLine != 0x00)
		  {
		    // @@@ Could be more optimal, probably.
		    Tpixel mixedFG = *VRAM;
		    mixerFG.Mix (mixedFG);
		    
		    Tpixmixer3 mixer (G2D, mixedFG, CharLine);
		    mixer.Mix (*VRAM);
		  }
		  VRAM++;
		  if (j < rX-1) CharLine = (*CharImageAlpha++);
		} /* endfor */
		if (advance < 0) advance++;
	      }
	    }
	  }
	  else
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
	      register Tpixel* VRAM = (Tpixel*)G2D->GetPixelAt (lAbsX, cury);
	      for (int j = lX; j < rX; j++)
	      {
		if (advance >= 0)
		{
		  if (CharLine & 0x80)
		    mixerFG.Mix (*VRAM++);
		  else
		    mixerBG.Mix (*VRAM++);
		}
		else
		{
		  if (CharLine & 0x80)
		    mixerFG.Mix (*VRAM++);
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
	  }
	}
	else
	{
	  // no clipping
	  if (CharImageAlpha)
	  {
	    for (int i = 0; i < charH; i++, cury++)
	    {
	      advance = oldAdvance;
	      register Tpixel* VRAM = (Tpixel*)G2D->GetPixelAt (x, cury);
	      register unsigned pixW = charW;
	      register int pix;
	      for (pix = pixW; pix > 0; pix--)
	      {
		register uint8 CharLine = (*CharImageAlpha++);
		if (advance < 0)
		{
		  if (CharLine == 0xff)
		  {
		    mixerFG.Mix (*VRAM);
		  }
		  else if (CharLine != 0x00)
		  {
		    // @@@ Could be more optimal, probably.
		    Tpixel mixedFG = *VRAM;
		    mixerFG.Mix (mixedFG);
		    
		    Tpixmixer3 mixer (G2D, mixedFG, CharLine);
		    mixer.Mix (*VRAM);
		  }
		}
		else
		{
		  if (CharLine == 0xff)
		  {
		    mixerFG.Mix (*VRAM);
		  }
		  else if (CharLine == 0x00)
		  {
		    mixerBG.Mix (*VRAM);
		  }
		  else
		  {
		    // @@@ Could be more optimal, probably.
		    Tpixel mixedFG = *VRAM;
		    mixerFG.Mix (mixedFG);
		    mixerBG.Mix (*VRAM);
		    
		    Tpixmixer3 mixer (G2D, mixedFG, CharLine);
		    mixer.Mix (*VRAM);
		  }
		}
		if (advance < 0) advance++;
		VRAM++;
	      }
	    }
	  }
	  else
	  {
	    for (int i = 0; i < charH; i++, cury++)
	    {
	      register Tpixel* VRAM = (Tpixel*)G2D->GetPixelAt (x, cury);
	      register unsigned pixW = charW;
	      while (pixW)
	      {
		register unsigned char CharLine = *CharImage++;
		register int pix;
		for (pix = pixW < 8 ? pixW : 8, pixW -= pix; CharLine && pix; pix--)
		{
		  if (advance < 0)
		  {
		    if (CharLine & 0x80)
		      mixerFG.Mix (*VRAM++);
		    else
		      VRAM++;
		    // Addition is faster than shift, at least on i586+
		    CharLine += CharLine;
		  }
		  else
		  {
		    if (CharLine & 0x80)
		      mixerFG.Mix (*VRAM++);
		    else
		      mixerBG.Mix (*VRAM++);
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
		while (pix--)
		  mixerBG.Mix (*VRAM++);
	      } 
	    } 
	  }
	} /* endif */
      }
  
      pen_x += cacheData->glyphMetrics.advance;
      advance += cacheData->glyphMetrics.advance - (charW + bmetrics->left);
    }
    cache->PurgeEmptyPlanes ();
    
  }
};

#endif // __CS_CSPLUGINCOMMON_CANVAS_DRAW_TEXT_H___
