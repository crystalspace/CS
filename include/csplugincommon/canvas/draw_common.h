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

#ifndef __CS_CSPLUGINCOMMON_CANVAS_DRAW_COMMON_H__
#define __CS_CSPLUGINCOMMON_CANVAS_DRAW_COMMON_H__

/**\file
 * Common software canvas drawing tools.
 */

#include "graph2d.h"

/**
 * \addtogroup plugincommon
 * @{ */

/// Split a g2d color into the color and alpha part.
template<class Tpixel>
static void SplitAlpha (const int color, Tpixel& colorPart, uint8& alphaPart)
{
  colorPart = color & 0x00ffffff;
  alphaPart = 255 - (color >> 24);
}

/// PixMixer that ignores alpha.
template<class Tpixel>
class csPixMixerCopy
{
  Tpixel color;
public:
  csPixMixerCopy (csGraphics2D* /*G2D*/, Tpixel color, uint8 /*alpha*/)
  {
    csPixMixerCopy::color = color;
  }
  inline void Mix (Tpixel& dest)
  {
    dest = color;
  }
};

/// PixMixer that mixes the pixel into the background with alpha
template<class Tpixel>
class csPixMixerRGBA
{
  uint32 rbMask, gaMask;
  int intAlphaInv;
  uint32 intRB, intGA;
  int gShift, gShift2;
  int alphaPrec;
public:
  csPixMixerRGBA (csGraphics2D* G2D, Tpixel color, uint8 alpha)
  {
    alphaPrec = G2D->pfmt.GreenBits;
    const int alphaShift = 8 - alphaPrec;
    const int intAlpha = (alpha + 1) >> alphaShift;
    intAlphaInv = (255 - alpha + 1) >> alphaShift;
    gShift = G2D->pfmt.GreenShift;
    gShift2 = alphaPrec - gShift;
    // Makes the assumption that colors are laid out ARGB ...
    rbMask = G2D->pfmt.RedMask | G2D->pfmt.BlueMask;
    intRB = ((color & rbMask) * intAlpha) >> alphaPrec;
    gaMask = G2D->pfmt.GreenMask | G2D->pfmt.AlphaMask;
    intGA = (((color & gaMask) >> gShift) * intAlpha) >> gShift2;
  }
  inline void Mix (Tpixel& dest) const
  {
    const Tpixel destCol = dest;
    const Tpixel newRB = 
      ((intRB + (((destCol & rbMask) * intAlphaInv) >> alphaPrec)) & rbMask);
    const Tpixel newGA = 
      ((intGA + (((((destCol & gaMask) >> gShift) * intAlphaInv)) >> gShift2)) 
	& gaMask);
    dest = newRB | newGA;
  }
};

/// PixMixer that doesn't do anything.
template<class Tpixel>
class csPixMixerNoop
{
public:
  csPixMixerNoop (csGraphics2D* /*G2D*/, Tpixel /*color*/, uint8 /*alpha*/)
  { }
  inline void Mix (Tpixel& dest)
  { }
};

/** @} */

#endif // __CS_CSPLUGINCOMMON_CANVAS_DRAW_COMMON_H__
