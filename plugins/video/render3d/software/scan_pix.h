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
    CS_FORCEINLINE
    static PixType Shift (const uint8 x, const int v)
    {
      return (v > 0) ? (x << v) : (x >> -v);
    }
    CS_FORCEINLINE
    static PixType Unshift (const PixType x, const int v)
    {
      return (v > 0) ? (x >> v) : (x << -v);
    }
  public:
    Pix_Fix (const csPixelFormat& /*pfmt*/) {}

    CS_FORCEINLINE
    void WritePix (PixType* dest, uint8 r, uint8 g, uint8 b, uint8 a)
    {
      *dest = Shift (a & ma, sa)
	| Shift (r & mr, sr)
	| Shift (g & mg, sg)
	| Shift (b & mb, sb);
    }

    CS_FORCEINLINE
    void MultiplyDstAdd (PixType* dest, 
      uint8 Mr, uint8 Mg, uint8 Mb, uint8 Ma,
      uint8 ar, uint8 ag, uint8 ab, uint8 aa)
    {
      // @@@ Bleh: optimize.
      uint8 dr, dg, db, da;
      GetPix (dest, dr, dg, db, da);
      WritePix (dest, 
	csClamp<uint> (((dr * (Mr+1)) >> 8) + ar, 255, 0),
	csClamp<uint> (((dg * (Mg+1)) >> 8) + ag, 255, 0),
	csClamp<uint> (((db * (Mb+1)) >> 8) + ab, 255, 0),
	csClamp<uint> (((da * (Ma+1)) >> 8) + aa, 255, 0));
    }

    CS_FORCEINLINE
    void MultiplyDstAdd (PixType* dest, 
      uint8 M, uint8 ar, uint8 ag, uint8 ab, uint8 aa)
    {
      const uint v = M+1;
      // @@@ Bleh: optimize.
      uint8 dr, dg, db, da;
      GetPix (dest, dr, dg, db, da);
      WritePix (dest, 
	csClamp<uint> (((dr * v) >> 8) + ar, 255, 0),
	csClamp<uint> (((dg * v) >> 8) + ag, 255, 0),
	csClamp<uint> (((db * v) >> 8) + ab, 255, 0),
	csClamp<uint> (((da * v) >> 8) + aa, 255, 0));
    }

    CS_FORCEINLINE
    void GetPix (PixType* dest, uint8& r, uint8& g, uint8& b, uint8& a) const
    {
      const PixType px = *dest;
      r = Unshift (px, sr) & mr;
      g = Unshift (px, sg) & mg;
      b = Unshift (px, sb) & mb;
      a = Unshift (px, sa) & ma;
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
    void WritePix (PixType* dest, uint8 r, uint8 g, uint8 b, uint8 a)
    {
      *dest = ((a & aMask) << aShift) 
	| ((r & rMask) << rShift) 
	| ((g & gMask) << gShift) 
	| ((b & bMask) >> bShift);
    }

    CS_FORCEINLINE
    void MultiplyDstAdd (PixType* dest, 
      uint8 Mr, uint8 Mg, uint8 Mb, uint8 Ma,
      uint8 ar, uint8 ag, uint8 ab, uint8 aa)
    {
      // @@@ Bleh: optimize.
      uint8 dr, dg, db, da;
      GetPix (dest, dr, dg, db, da);
      WritePix (dest, 
	csClamp<uint> (((dr * (Mr+1)) >> 8) + ar, 255, 0),
	csClamp<uint> (((dg * (Mg+1)) >> 8) + ag, 255, 0),
	csClamp<uint> (((db * (Mb+1)) >> 8) + ab, 255, 0),
	csClamp<uint> (((da * (Ma+1)) >> 8) + aa, 255, 0));
    }

    CS_FORCEINLINE
    void MultiplyDstAdd (PixType* dest, 
      uint8 M, uint8 ar, uint8 ag, uint8 ab, uint8 aa)
    {
      const uint v = M+1;
      // @@@ Bleh: optimize.
      uint8 dr, dg, db, da;
      GetPix (dest, dr, dg, db, da);
      WritePix (dest, 
	csClamp<uint> (((dr * v) >> 8) + ar, 255, 0),
	csClamp<uint> (((dg * v) >> 8) + ag, 255, 0),
	csClamp<uint> (((db * v) >> 8) + ab, 255, 0),
	csClamp<uint> (((da * v) >> 8) + aa, 255, 0));
    }

    CS_FORCEINLINE
    void GetPix (PixType* dest, uint8& r, uint8& g, uint8& b, uint8& a) const
    {
      const PixType px = *dest;
      a = (px >> aShift) & aMask;
      r = (px >> rShift) & rMask;
      g = (px >> gShift) & gMask;
      b = (px << bShift) & bMask;
    }
  };
} // namespace cspluginSoft3d

#endif // __CS_SOFT3D_SCAN_PIX_H__
