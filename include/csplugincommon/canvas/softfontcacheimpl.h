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

#ifndef __CS_CANVAS_COMMON_SOFTFONTCACHEIMPL_H__
#define __CS_CANVAS_COMMON_SOFTFONTCACHEIMPL_H__

#include "graph2d.h"
#include "draw_text.h"

template <class Tpixel, class Tpixmixer>
class csSoftFontCacheImpl : public csSoftFontCache
{
public:
  csSoftFontCacheImpl (csGraphics2D* G2D) : csSoftFontCache (G2D)
  {
  }
  virtual void WriteString (iFont *font, int x, int y, int fg, int bg, 
    const utf8_char* text, uint flags)
  {
    int realColorFG;
    uint8 alphaFG;
    SplitAlpha (fg, realColorFG, alphaFG);
    int realColorBG;
    uint8 alphaBG;
    SplitAlpha (bg, realColorBG, alphaBG);

    if (alphaBG == 0)
    {
      if (alphaFG == 0)
	return;
      realColorBG = realColorFG;
      if (alphaFG == 255)
      {
	csG2DDrawText<Tpixel, csPixMixerCopy<Tpixel>, csPixMixerNoop<Tpixel>, 
	  Tpixmixer>::DrawText (this, font, x, y, realColorFG, alphaFG, 
	  realColorBG, alphaBG, text, flags);
      }
      else
      {
	csG2DDrawText<Tpixel, Tpixmixer, csPixMixerNoop<Tpixel>, 
	  Tpixmixer>::DrawText (this, font, x, y, realColorFG, alphaFG, 
	  realColorBG, alphaBG, text, flags);
      }
    }
    else if (alphaBG == 255)
    {
      if (alphaFG == 0)
      {
	csG2DDrawText<Tpixel, csPixMixerNoop<Tpixel>, csPixMixerCopy<Tpixel>, 
	  Tpixmixer>::DrawText (this, font, x, y, realColorFG, alphaFG, 
	  realColorBG, alphaBG, text, flags);
      }
      else if (alphaFG == 255)
      {
	csG2DDrawText<Tpixel, csPixMixerCopy<Tpixel>, csPixMixerCopy<Tpixel>, 
	  Tpixmixer>::DrawText (this, font, x, y, realColorFG, alphaFG, 
	  realColorBG, alphaBG, text, flags);
      }
      else
      {
	csG2DDrawText<Tpixel, Tpixmixer, csPixMixerCopy<Tpixel>, 
	  Tpixmixer>::DrawText (this, font, x, y, realColorFG, alphaFG, 
	  realColorBG, alphaBG, text, flags);
      }
    }
    else
    {
      if (alphaFG == 0)
      {
	csG2DDrawText<Tpixel, csPixMixerNoop<Tpixel>, Tpixmixer, 
	  Tpixmixer>::DrawText (this, font, x, y, realColorFG, alphaFG, 
	  realColorBG, alphaBG, text, flags);
      }
      else if (alphaFG == 255)
      {
	csG2DDrawText<Tpixel, csPixMixerCopy<Tpixel>, Tpixmixer, 
	  Tpixmixer>::DrawText (this, font, x, y, realColorFG, alphaFG, 
	  realColorBG, alphaBG, text, flags);
      }
      else
      {
	csG2DDrawText<Tpixel, Tpixmixer, Tpixmixer, Tpixmixer>::DrawText (
	  this, font, x, y, realColorFG, alphaFG, realColorBG, alphaBG, text, 
	  flags);
      }
    }
  }
};

#endif // __CS_CANVAS_COMMON_SOFTFONTCACHEIMPL_H__
