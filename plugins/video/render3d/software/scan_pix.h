/*
    Copyright (C) 2005 by Jorrit Tyberghein
              (C) 2005 by Frank Richter

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

#ifndef __CS_SOFT3D_SCAN_PIX_H__
#define __CS_SOFT3D_SCAN_PIX_H__

#include "ivideo/graph2d.h"

#include "csgeom/math.h"

namespace cspluginSoft3d
{
  template<typename Pix, 
    int sa, int ma,
    int sr, int mr,
    int sg, int mg, 
    int sb, int mb>
  struct Pix_Fix
  {
    typedef Pix PixType;

  private:
    template<int v>
    CS_FORCEINLINE
    static PixType Shift (const uint x)
    {
      if (v > 0)
        return x << ABS(v);
      else
        return x >> ABS(v);
    }
    template<int v>
    CS_FORCEINLINE
    static uint Unshift (const PixType x)
    {
      if (v > 0)
        return x >> ABS(v);
      else
        return x << ABS(v);
    }
  public:
    Pix_Fix (const csPixelFormat& /*pfmt*/) {}

    CS_FORCEINLINE
    void WritePix (PixType* dest, const Pixel p) const
    {
      *dest = Shift<sa> (p.a & ma)
	| Shift<sr> (p.r & mr)
	| Shift<sg> (p.g & mg)
	| Shift<sb> (p.b & mb);
    }

    Pixel GetPix (PixType* dest) const
    {
      Pixel p;
      const PixType px = *dest;
      p.r = Unshift<sr> (px) & mr;
      p.g = Unshift<sg> (px) & mg;
      p.b = Unshift<sb> (px) & mb;
      p.a = Unshift<sa> (px) & ma;
      return p;
    }
  };

  template<typename Pix>
  struct Pix_Generic
  {
    typedef Pix PixType;
    PixType rMask, gMask, bMask, aMask;
    int rShift, gShift, bShift, aShift;

    Pix_Generic (const csPixelFormat& pfmt) 
    {
      int delta = 8-pfmt.RedBits;
      if (pfmt.RedMask > pfmt.BlueMask)
      {
	rShift = pfmt.RedShift-delta;
	rMask = pfmt.RedMask >> rShift;
      }
      else
      {
	rShift = pfmt.BlueShift-delta;
	rMask = pfmt.BlueMask >> rShift;
      }
      delta = 8-pfmt.GreenBits;
      gShift = pfmt.GreenShift-delta;
      gMask = pfmt.GreenMask >> gShift;
      delta = 8-pfmt.BlueBits;
      if (pfmt.RedMask > pfmt.BlueMask)
      {
	bShift = delta;
	bMask = pfmt.BlueMask << bShift;
      }
      else
      {
	bShift = delta;
	bMask = pfmt.RedMask << bShift;
      }
      aMask = 0;
      aMask |= pfmt.RedMask;
      aMask |= pfmt.GreenMask;
      aMask |= pfmt.BlueMask;
      aMask = ~aMask;
      aShift = 0;
      if (aMask != 0)
      {
	while (!(aMask & (1 << aShift))) aShift++;
	aMask >>= aShift;
	while (!(aMask & 0x80))
	{
	  aMask <<= 1;
	  aShift--;
	}
      }
    }

    CS_FORCEINLINE
    void WritePix (PixType* dest, const Pixel p) const
    {
      *dest = ((p.a & aMask) << aShift) 
	| ((p.r & rMask) << rShift) 
	| ((p.g & gMask) << gShift) 
	| ((p.b & bMask) >> bShift);
    }

    CS_FORCEINLINE
    Pixel GetPix (PixType* dest) const
    {
      Pixel p;
      const PixType px = *dest;
      p.a = (px >> aShift) & aMask;
      p.r = (px >> rShift) & rMask;
      p.g = (px >> gShift) & gMask;
      p.b = (px << bShift) & bMask;
      return p;
    }
  };
} // namespace cspluginSoft3d

#endif // __CS_SOFT3D_SCAN_PIX_H__
