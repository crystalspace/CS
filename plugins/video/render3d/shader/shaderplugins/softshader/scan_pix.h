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

#ifndef __CS_SOFTSHADER_SCAN_PIX_H__
#define __CS_SOFTSHADER_SCAN_PIX_H__

#include "ivideo/graph2d.h"

#include "csgeom/math.h"

namespace cspluginSoftshader
{
  template<typename Pix, 
    int sal, int sar, int ma,
    int srl, int srr, int mr,
    int sgl, int sgr, int mg, 
    int sbl, int sbr, int mb>
  struct Pix_Fix
  {
    typedef Pix PixType;

    Pix_Fix (const csPixelFormat& /*pfmt*/) {}

    CS_FORCEINLINE
    void WritePix (PixType* dest, uint8 r, uint8 g, uint8 b, uint8 a)
    {
      *dest = (((a & ma) << sal) >> sar)
	| (((r & mr) << srl) >> srr)
	| (((g & mg) << sgl) >> sgr)
	| (((b & mb) << sbl) >> sbr);
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
      r = ((px << srr) >> srl) & mr;
      g = ((px << sgr) >> sgl) & mg;
      b = ((px << sbr) >> sbl) & mb;
      a = ((px << sar) >> sal) & ma;
    }
  };

  template<typename Pix, int OrderRGB>
  struct Pix_Generic
  {
    typedef Pix PixType;
    PixType rMask, gMask, bMask, aMask;
    int rShift, gShift, bShift, aShift;

    Pix_Generic (const csPixelFormat& pfmt) 
    {
      int delta = 8-pfmt.RedBits;
      if (OrderRGB)
      {
	rShift = pfmt.RedShift-delta;
	rMask = pfmt.RedMask >> rShift;
      }
      else
      {
	rShift = delta;
	rMask = pfmt.RedMask << rShift;
      }
      delta = 8-pfmt.GreenBits;
      gShift = pfmt.GreenShift-delta;
      gMask = pfmt.GreenMask >> gShift;
      delta = 8-pfmt.BlueBits;
      if (OrderRGB)
      {
	bShift = delta;
	bMask = pfmt.BlueMask << bShift;
      }
      else
      {
	bShift = pfmt.BlueShift-delta;
	bMask = pfmt.BlueMask >> bShift;
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
      if (OrderRGB)
        *dest = ((a & aMask) << aShift) 
	  | ((r & rMask) << rShift) 
	  | ((g & gMask) << gShift) 
	  | ((b & bMask) >> bShift);
      else
        *dest = ((a & aMask) << aShift) 
	  | ((b & bMask) << bShift)
	  | ((g & gMask) << gShift) 
	  | ((r & rMask) >> rShift); 
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
      if (OrderRGB)
      {
	a = (px >> aShift) & aMask;
	r = (px >> rShift) & rMask;
	g = (px >> gShift) & gMask;
	b = (px << bShift) & bMask;
      }
      else
      {
	a = (px >> aShift) & aMask;
	b = (px >> bShift) & bMask;
	g = (px >> gShift) & gMask;
	r = (px << rShift) & rMask;
      }
    }
  };
} // namespace cspluginSoftshader

#endif // __CS_SOFTSHADER_SCAN_PIX_H__
