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

#ifndef __CS_SOFTSHADER_SCANLINE_H__
#define __CS_SOFTSHADER_SCANLINE_H__

#include "ivideo/rendermesh.h"

#include "csgeom/math.h"
#include "csplugincommon/softshader/scanline.h"
#include "csplugincommon/softshader/types.h"
#include "csutil/scf_implementation.h"
#include "scan_pix.h"
#include "scan_z.h"

namespace cspluginSoftshader
{
  using namespace CrystalSpace::SoftShader;

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

  class ScanlineRendererBase : 
    public scfImplementation1<ScanlineRendererBase, iScanlineRenderer>
  {
  public:
    uint8 flat_r, flat_g, flat_b, flat_a;
    uint32* bitmap;
    int v_shift_r;
    int and_w;
    int and_h;

    ScanlineRendererBase() : scfImplementationType (this),
      flat_r(255), flat_g(255), flat_b(255), flat_a(255) {}
    virtual ~ScanlineRendererBase() {}

    void SetFlatColor (const csVector4& v)
    {
      flat_r = csClamp ((int)(v.x * 255.99f), 255, 0);
      flat_g = csClamp ((int)(v.y * 255.99f), 255, 0);
      flat_b = csClamp ((int)(v.z * 255.99f), 255, 0);
      flat_a = csClamp ((int)(v.w * 255.99f), 255, 0);
    }
  };
  
  struct Source_Texture
  {
    static const size_t compCount = 2;

    const uint32* bitmap;
    const int v_shift_r;
    const int and_w;
    const int and_h;

    Source_Texture (ScanlineRendererBase* This) :
      bitmap(This->bitmap),
      v_shift_r(This->v_shift_r),
      and_w(This->and_w),
      and_h(This->and_h)
    {
    }

    CS_FORCEINLINE
    void GetColor (const ScanlineComp* tc, 
      uint8& r, uint8& g, uint8& b, uint8& a)
    {
      int u = (int)(tc[0].c);
      int32 v = tc[1].c.GetFixed();
      uint32 texel = bitmap [((v >> v_shift_r) & and_h) + (u & and_w)];
      r = texel & 0xff;
      g = (texel >> 8) & 0xff;
      b = (texel >> 16) & 0xff;
      a = (texel >> 24);
    }
  };

  struct Source_Flat
  {
    static const size_t compCount = 0;

    const uint8 flat_r;
    const uint8 flat_g;
    const uint8 flat_b;
    const uint8 flat_a;

    Source_Flat (ScanlineRendererBase* This) :
      flat_r(This->flat_r),
      flat_g(This->flat_g),
      flat_b(This->flat_b),
      flat_a(This->flat_a)
    {
    }

    CS_FORCEINLINE
    void GetColor (const ScanlineComp* /*tc*/, 
      uint8& r, uint8& g, uint8& b, uint8& a)
    {
      r = flat_r;
      g = flat_g;
      b = flat_b;
      a = flat_a;
    }
  };

  template<typename Pix>
  class ScanlineRenderer : public ScanlineRendererBase
  {
  public:
    Pix pix;
    csVector4 dnTC;

  private:
    template <typename Source, typename Color, typename Zmode>
    struct ScanlineImpl
    {
      static void Scan (iScanlineRenderer* _This,
	InterpolateEdgePersp& L, InterpolateEdgePersp& R, 
	int ipolStep, int ipolShift,
	void* dest, uint len, uint32 *zbuff)
      {
	const size_t offsetColor = 0;
	const size_t offsetTC = offsetColor + Color::compCount;
	const size_t myIpolFloatNum = offsetTC + Source::compCount;

	InterpolateScanlinePersp<myIpolFloatNum> ipol;
	ipol.Setup (L, R, 1.0f / len, ipolStep, ipolShift);
	ScanlineRenderer<Pix>* This = (ScanlineRenderer<Pix>*)_This;

	Pix& pix = This->pix;
	Source src (This);

	typename_qualifier Pix::PixType* _dest = 
	  (typename_qualifier Pix::PixType*)dest;
	typename_qualifier Pix::PixType* _destend = _dest + len;
	Zmode Z (ipol, zbuff);

	while (_dest < _destend)
	{
	  if (Z.Test())
	  {
	    uint8 r, g, b, a;
	    src.GetColor (ipol.GetFloat (offsetTC), r, g, b, a);
	    Color::Apply (ipol.GetFloat (offsetColor), r, g, b, a);
	    pix.WritePix (_dest, r, g, b, a);
	    Z.Update();
	  }
	  _dest++;
	  ipol.Advance();
	  Z.Advance();
	} /* endwhile */
      }
    };
    template<typename Source, typename Color>
    static iScanlineRenderer::ScanlineProc GetScanlineProcSC (csZBufMode zmode)
    {
      switch (zmode)
      {
	case CS_ZBUF_NONE:
	  return ScanlineImpl<Source, Color, ZBufMode_ZNone>::Scan;
	case CS_ZBUF_FILL:
	  return ScanlineImpl<Source, Color, ZBufMode_ZFill>::Scan;
	case CS_ZBUF_TEST:
	  return ScanlineImpl<Source, Color, ZBufMode_ZTest>::Scan;
	case CS_ZBUF_USE:
	  return ScanlineImpl<Source, Color, ZBufMode_ZUse>::Scan;
	case CS_ZBUF_EQUAL:
	  return ScanlineImpl<Source, Color, ZBufMode_ZEqual>::Scan;
	case CS_ZBUF_INVERT:
	  return ScanlineImpl<Source, Color, ZBufMode_ZInvert>::Scan;
	default:
	  return 0;
      }
    }
    template<typename Source>
    static iScanlineRenderer::ScanlineProc GetScanlineProcS (bool colorized, 
							     csZBufMode zmode)
    {
      if (colorized)
	return GetScanlineProcSC<Source, Color_Multiply> (zmode);
      else
	return GetScanlineProcSC<Source, Color_None> (zmode);
    }
    static iScanlineRenderer::ScanlineProc GetScanlineProc (bool flat,
							    bool colorized, 
							    csZBufMode zmode)
    {
      if (flat)
	return GetScanlineProcS<Source_Flat> (colorized, zmode);
      else
	return GetScanlineProcS<Source_Texture> (colorized, zmode);
    }
  public:
    ScanlineRenderer (const csPixelFormat& pfmt) : pix (pfmt) 
    {}

    bool Init (SoftwareTexture** textures,
      const csRenderMeshModes& modes,
      BuffersMask availableBuffers,
      iScanlineRenderer::RenderInfo& renderInfo)
    {
      renderInfo.desiredBuffers = 0;
      renderInfo.denormBuffers = 0;

      SoftwareTexture* tex = textures[0];
      bool doFlat = false;
      if (tex != 0)
      {
	bitmap = tex->bitmap;
        v_shift_r = tex->shf_w;
        and_w = tex->and_w;
        and_h = tex->and_h << v_shift_r;
        v_shift_r = 16 - v_shift_r;

	renderInfo.desiredBuffers |= CS_SOFT3D_BUFFERFLAG(TEXCOORD);
	renderInfo.denormFactors = &dnTC;
	renderInfo.denormBuffers |= CS_SOFT3D_BUFFERFLAG(TEXCOORD);

	dnTC.Set (tex->w, tex->h, 0.0f, 0.0f);
      }
      else
	doFlat = true;

      bool doColor = availableBuffers & CS_SOFT3D_BUFFERFLAG(COLOR);

      renderInfo.renderer = this;
      static const size_t myBufferComps[] = {4, 2};
      if (doColor)
      {
	renderInfo.desiredBuffers |= CS_SOFT3D_BUFFERFLAG(COLOR);
	renderInfo.bufferComps = myBufferComps;
      }
      else
	renderInfo.bufferComps = &myBufferComps[1];

      renderInfo.proc = GetScanlineProc (doFlat, doColor, modes.z_buf_mode);

      return renderInfo.proc != 0;
    }
  };
} // namespace cspluginSoftshader

#endif // __CS_SOFTSHADER_SCANLINE_H__
