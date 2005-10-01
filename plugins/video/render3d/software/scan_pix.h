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

namespace cspluginSoft3d
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
  };
} // namespace cspluginSoft3d

#endif // __CS_SOFT3D_SCAN_PIX_H__
