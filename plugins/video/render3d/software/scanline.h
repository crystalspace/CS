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
#include "scan_pix.h"
#include "scan_z.h"

namespace cspluginSoft3d
{
  struct ScanlineRenderInfo;

  class ScanlineRendererBase
  {
  public:
    typedef void (*ScanlineProc) (ScanlineRendererBase* _This,
      InterpolateEdgePersp& L, InterpolateEdgePersp& R, 
      int ipolStep, int ipolShift,
      void* dest, uint len, uint32 *zbuff);

    virtual bool Init (csSoftwareTexture** textures,
      const csRenderMeshModes& modes,
      BuffersMask availableBuffers,
      ScanlineRenderInfo& renderInfo) = 0;
  };

  struct Color_None
  {
    static const size_t compCount = 0;

    CS_FORCEINLINE
    static void Apply (const ScanlineComp* /*color*/, 
      uint8& /*r*/, uint8& /*g*/, uint8& /*b*/, uint8& /*a*/) {}
  };

  struct Color_Multiply
  {
    static const size_t compCount = 4;

    CS_FORCEINLINE
    static uint8 ClampAndShift (int32 x, const int shift)
    {
      return (x & 0x80000000) ? 0 : 
	(((x >> shift) & 0x7fffff00) ? 0xff : (x >> shift));
    }

    CS_FORCEINLINE
    static void Apply (const ScanlineComp* color, 
      uint8& r, uint8& g, uint8& b, uint8& a) 
    {
      // @@@ FIXME: adjustable scale
      r = ClampAndShift (r * color[0].c.GetFixed(), 15);
      g = ClampAndShift (g * color[1].c.GetFixed(), 15);
      b = ClampAndShift (b * color[2].c.GetFixed(), 15);
      a = ClampAndShift (a * color[3].c.GetFixed(), 15);
    }
  };

  struct ScanlineRenderInfo
  {
    ScanlineRendererBase* renderer;
    ScanlineRendererBase::ScanlineProc proc;
    csVector4* denormFactors;
    BuffersMask denormBuffers;
    BuffersMask desiredBuffers;
    const size_t* bufferComps;
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
    template <typename Color, typename Zmode>
    struct ScanlineImpl
    {
      static void Scan (ScanlineRendererBase* _This,
	InterpolateEdgePersp& L, InterpolateEdgePersp& R, 
	int ipolStep, int ipolShift,
	void* dest, uint len, uint32 *zbuff)
      {
	const size_t offsetColor = 0;
	const size_t offsetTC = offsetColor + Color::compCount;
	const size_t myIpolFloatNum = offsetTC + 2;

	InterpolateScanlinePersp<myIpolFloatNum> ipol;
	ipol.Setup (L, R, 1.0f / len, ipolStep, ipolShift);
	ScanlineRenderer<Pix>* This = (ScanlineRenderer<Pix>*)_This;

	Pix& pix = This->pix;
	const uint32* bitmap = This->bitmap;
	const int v_shift_r = This->v_shift_r;
	const int and_w = This->and_w;
	const int and_h = This->and_h;

	typename_qualifier Pix::PixType* _dest = 
	  (typename_qualifier Pix::PixType*)dest;
	typename_qualifier Pix::PixType* _destend = _dest + len;
	Zmode Z (ipol, zbuff);

	while (_dest < _destend)
	{
	  if (Z.Test())
	  {
	    int u = (int)(ipol.floats[offsetTC+0].c);
	    int32 v = ipol.floats[offsetTC+1].c.GetFixed();
	    uint32 texel = bitmap [((v >> v_shift_r) & and_h) + (u & and_w)];
	    {
	      uint8 r = texel & 0xff;
	      uint8 g = (texel >> 8) & 0xff;
	      uint8 b = (texel >> 16) & 0xff;
	      uint8 a = (texel >> 24);
	      Color::Apply (&ipol.floats[offsetColor], r, g, b, a);
	      pix.WritePix (_dest, r, g, b, a);
	      Z.Update();
	    }
	  }
	  _dest++;
	  ipol.Advance();
	  Z.Advance();
	} /* endwhile */
      }
    };
    template<typename Color>
    static ScanlineProc GetScanlineProcC (csZBufMode zmode)
    {
      switch (zmode)
      {
	case CS_ZBUF_NONE:
	  return ScanlineImpl<Color, ZBufMode_ZNone>::Scan;
	case CS_ZBUF_FILL:
	  return ScanlineImpl<Color, ZBufMode_ZFill>::Scan;
	case CS_ZBUF_TEST:
	  return ScanlineImpl<Color, ZBufMode_ZTest>::Scan;
	case CS_ZBUF_USE:
	  return ScanlineImpl<Color, ZBufMode_ZUse>::Scan;
	case CS_ZBUF_EQUAL:
	  return ScanlineImpl<Color, ZBufMode_ZEqual>::Scan;
	case CS_ZBUF_INVERT:
	  return ScanlineImpl<Color, ZBufMode_ZInvert>::Scan;
	default:
	  return 0;
      }
    }
    static ScanlineProc GetScanlineProc (bool colorized, csZBufMode zmode)
    {
      if (colorized)
	return GetScanlineProcC<Color_Multiply> (zmode);
      else
	return GetScanlineProcC<Color_None> (zmode);
    }
  public:
    ScanlineRenderer (const csPixelFormat& pfmt) : pix (pfmt) 
    {}

    bool Init (csSoftwareTexture** textures,
      const csRenderMeshModes& modes,
      BuffersMask availableBuffers,
      ScanlineRenderInfo& renderInfo)
    {
      csSoftwareTexture* tex = textures[0];
      if (tex == 0) return false;      // @@@ Use flat color instead
      bitmap = tex->bitmap;
      v_shift_r = tex->get_w_shift();
      and_w = tex->get_w_mask();
      and_h = tex->get_h_mask() << v_shift_r;
      v_shift_r = 16 - v_shift_r;

      bool doColor = availableBuffers & CS_BUFFERFLAG(COLOR);

      renderInfo.renderer = this;
      renderInfo.desiredBuffers = CS_BUFFERFLAG(TEXCOORD);
      dnTC.Set (tex->get_width(), tex->get_height(), 0.0f, 0.0f);
      renderInfo.denormFactors = &dnTC;
      renderInfo.denormBuffers = CS_BUFFERFLAG(TEXCOORD);
      static const size_t myBufferComps[] = {4, 2};
      if (doColor)
      {
	renderInfo.desiredBuffers |= CS_BUFFERFLAG(COLOR);
	renderInfo.bufferComps = myBufferComps;
      }
      else
	renderInfo.bufferComps = &myBufferComps[1];

      renderInfo.proc = GetScanlineProc (doColor, modes.z_buf_mode);

      return renderInfo.proc != 0;
    }
  };
} // namespace cspluginSoft3d

#endif // __CS_SOFT3D_SCANLINE_H__
