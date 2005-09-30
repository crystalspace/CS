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

#ifndef __CS_SOFT3D_SCANLINE_H__
#define __CS_SOFT3D_SCANLINE_H__

#include "ivideo/rendermesh.h"

#include "types.h"
#include "scan.h"

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

  class ScanlineRendererBase
  {
  public:
    typedef void (*ScanlineProc) (ScanlineRendererBase* _This,
      InterpolateEdgePersp& L, InterpolateEdgePersp& R, 
      int ipolStep, int ipolShift,
      void* dest, uint len, uint32 *zbuff);

    virtual ScanlineProc Init (csSoftwareTexture** textures,
      const csRenderMeshModes& modes,
      BuffersMask& desiredBuffers,
      csVector4*& denormFactors,
      BuffersMask& denormBuffers) = 0;
  };

  struct ZBufMode_ZNone
  {
    ZBufMode_ZNone (const InterpolateScanlinePersp& /*ipol*/, 
      uint32* /*zBuff*/) {}

    bool Test () { return true; }
    void Update () {}
    void Advance () {}
  };

  struct ZBufMode_ZFill
  {
    const uint32 Iz;
    uint32* zBuff;

    ZBufMode_ZFill (const InterpolateScanlinePersp& ipol, uint32* zBuff) : 
      Iz(ipol.Iz.GetFixed()), zBuff(zBuff) {}

    bool Test () { return true; }
    void Update () { *zBuff = Iz; }
    void Advance () { zBuff++; }
  };

  struct ZBufMode_ZTest
  {
    const uint32 Iz;
    uint32* zBuff;

    ZBufMode_ZTest (const InterpolateScanlinePersp& ipol, uint32* zBuff) : 
      Iz(ipol.Iz.GetFixed()), zBuff(zBuff) {}

    bool Test () { return Iz >= *zBuff; }
    void Update () { }
    void Advance () { zBuff++; }
  };

  struct ZBufMode_ZUse
  {
    const uint32 Iz;
    uint32* zBuff;

    ZBufMode_ZUse (const InterpolateScanlinePersp& ipol, uint32* zBuff) : 
      Iz(ipol.Iz.GetFixed()), zBuff(zBuff) {}

    bool Test () { return Iz >= *zBuff; }
    void Update () { *zBuff = Iz; }
    void Advance () { zBuff++; }
  };

  struct ZBufMode_ZEqual
  {
    const uint32 Iz;
    uint32* zBuff;

    ZBufMode_ZEqual (const InterpolateScanlinePersp& ipol, uint32* zBuff) : 
      Iz(ipol.Iz.GetFixed()), zBuff(zBuff) {}

    bool Test () { return Iz == *zBuff; }
    void Update () { }
    void Advance () { zBuff++; }
  };

  struct ZBufMode_ZInvert
  {
    const uint32 Iz;
    uint32* zBuff;

    ZBufMode_ZInvert (const InterpolateScanlinePersp& ipol, uint32* zBuff) : 
      Iz(ipol.Iz.GetFixed()), zBuff(zBuff) {}

    bool Test () { return Iz < *zBuff; }
    void Update () { }
    void Advance () { zBuff++; }
  };

  template<typename Pix>
  class ScanlineRenderer : public ScanlineRendererBase
  {
  public:
    Pix pix;
    uint32* bitmap;
    int v_shift_r;
    int and_w;
    int and_h;
    csVector4 dnTC;

  private:
    template <typename Zmode>
    struct ScanlineImpl
    {
      static void Scan (ScanlineRendererBase* _This,
	InterpolateEdgePersp& L, InterpolateEdgePersp& R, 
	int ipolStep, int ipolShift,
	void* dest, uint len, uint32 *zbuff)
      {
	InterpolateScanlinePersp ipol;
	ipol.Setup (L, R, 1, 1.0f / len, ipolStep, ipolShift);
	ScanlineRenderer<Pix>* This = (ScanlineRenderer<Pix>*)_This;
	Pix& pix = This->pix;
	const uint32* bitmap = This->bitmap;
	const int v_shift_r = This->v_shift_r;
	const int and_w = This->and_w;
	const int and_h = This->and_h;
	const csVector4T<csFixed16>& tc = ipol.bufData[0].c;

	typename_qualifier Pix::PixType* _dest = 
	  (typename_qualifier Pix::PixType*)dest;
	typename_qualifier Pix::PixType* _destend = _dest + len;
	Zmode Z (ipol, zbuff);

	while (_dest < _destend)
	{
	  if (Z.Test())
	  {
	    int u = (int)tc.x;
	    int32 v = tc.y.GetFixed();
	    uint32 texel = bitmap [((v >> v_shift_r) & and_h) + (u & and_w)];
	    {
	      uint8 r = texel & 0xff;
	      uint8 g = (texel >> 8) & 0xff;
	      uint8 b = (texel >> 16) & 0xff;
	      uint8 a = (texel >> 24);
	      pix.WritePix (_dest, r, g, b, a);
	      Z.Update();
	    }
	  }
	  _dest++;
	  ipol.Advance (1);
	  Z.Advance();
	} /* endwhile */
      }
    };
  public:
    ScanlineRenderer (const csPixelFormat& pfmt) : pix (pfmt) 
    {}

    ScanlineProc Init (csSoftwareTexture** textures,
      const csRenderMeshModes& modes,
      BuffersMask& desiredBuffers,
      csVector4*& denormFactors,
      BuffersMask& denormBuffers)
    {
      desiredBuffers = 1 << VATTR_BUFINDEX(TEXCOORD);
      csSoftwareTexture* tex = textures[0];
      if (tex == 0) return 0;      // @@@ Use flat color instead
      bitmap = tex->bitmap;
      v_shift_r = tex->get_w_shift();
      and_w = tex->get_w_mask();
      and_h = tex->get_h_mask() << v_shift_r;
      v_shift_r = 16 - v_shift_r;

      dnTC.Set (tex->get_width(), tex->get_height(), 0.0f, 0.0f);
      denormFactors = &dnTC;
      denormBuffers = 1 << VATTR_BUFINDEX(TEXCOORD);

      switch (modes.z_buf_mode)
      {
	case CS_ZBUF_NONE:
	  return ScanlineImpl<ZBufMode_ZNone>::Scan;
	case CS_ZBUF_FILL:
	case CS_ZBUF_FILLONLY:
	  return ScanlineImpl<ZBufMode_ZFill>::Scan;
	case CS_ZBUF_TEST:
	  return ScanlineImpl<ZBufMode_ZTest>::Scan;
	case CS_ZBUF_USE:
	  return ScanlineImpl<ZBufMode_ZUse>::Scan;
	case CS_ZBUF_EQUAL:
	  return ScanlineImpl<ZBufMode_ZEqual>::Scan;
	case CS_ZBUF_INVERT:
	  return ScanlineImpl<ZBufMode_ZInvert>::Scan;
	default:
	  return 0;
      }
    }
  };

  struct ScanlineRenderInfo
  {
    ScanlineRendererBase* renderer;
    ScanlineRendererBase::ScanlineProc proc;
    csVector4* denormFactors;
    BuffersMask denormBuffers;
    BuffersMask desiredBuffers;
  };
} // namespace cspluginSoft3d

#endif // __CS_SOFT3D_SCANLINE_H__
