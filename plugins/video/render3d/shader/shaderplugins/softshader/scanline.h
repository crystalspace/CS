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

#include "csplugincommon/softshader/types.h"

#include "scan_z.h"
#include "scanline_base.h"

namespace cspluginSoftshader
{
  using namespace CrystalSpace::SoftShader;

  struct Color_None
  {
    static const size_t compCount = 0;

    Color_None (ScanlineRendererBase* /*This*/) {}

    CS_FORCEINLINE
    void Apply (const ScanlineComp* /*color*/, Pixel& /*col*/) {}
  };

  struct Color_Multiply
  {
    static const size_t compCount = 4;

    const int cshift;
    const int ashift;

    Color_Multiply (ScanlineRendererBase* This) : 
      cshift (This->colorShift), ashift (This->alphaShift) {}

    CS_FORCEINLINE
    static uint8 ClampAndShift (int32 x, const int shift)
    {
      return (x & 0x80000000) ? 0 : 
	(((x >> shift) & 0x7fffff00) ? 0xff : (x >> shift));
    }

    CS_FORCEINLINE
    void Apply (const ScanlineComp* color, 
      Pixel& col) 
    {
      col.r = ClampAndShift (col.r * color[0].c.GetFixed(), cshift);
      col.g = ClampAndShift (col.g * color[1].c.GetFixed(), cshift);
      col.b = ClampAndShift (col.b * color[2].c.GetFixed(), cshift);
      col.a = ClampAndShift (col.a * color[3].c.GetFixed(), ashift);
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
    void GetColor (const ScanlineComp* tc, Pixel& col)
    {
      int u = (int)(tc[0].c);
      int32 v = tc[1].c.GetFixed();
      uint32 texel = bitmap [((v >> v_shift_r) & and_h) + (u & and_w)];
      col.FromUI32 (texel);
    }
  };

  struct Source_Flat
  {
    static const size_t compCount = 0;

    const Pixel flat_col;

    Source_Flat (ScanlineRendererBase* This) :
      flat_col(This->flat_col) { }

    CS_FORCEINLINE
    void GetColor (const ScanlineComp* /*tc*/, Pixel& col)
    {
      col = flat_col;
    }
  };

  class ScanlineRenderer : public ScanlineRendererBase
  {
  public:
    csVector4 dnTC;

  private:
    template <typename Source, typename Color, 
      typename Zmode, int needColors>
    struct ScanlineImpl
    {
      static void Scan (iScanlineRenderer* _This,
	InterpolateEdgePersp& L, InterpolateEdgePersp& R, 
	int ipolStep, int ipolShift,
	uint32* dest, uint len, uint32 *zbuff)
      {
	const size_t offsetColor = 0;
	const size_t offsetTC = offsetColor + Color::compCount;
	const size_t myIpolFloatNum = offsetTC + Source::compCount;

	InterpolateScanlinePersp<myIpolFloatNum> ipol;
	ipol.Setup (L, R, 1.0f / len, ipolStep, ipolShift);
	ScanlineRenderer* This = (ScanlineRenderer*)_This;

	Source colSrc (This);
	Color col (This);

	uint32* destend = dest + len;
	Zmode Z (ipol, zbuff);

	while (dest < destend)
	{
	  if (Z.Test())
	  {
	    if (needColors)
	    {
	      Pixel px;
	      colSrc.GetColor (ipol.GetFloat (offsetTC), px);
	      col.Apply (ipol.GetFloat (offsetColor), px);

	      px.a = (px.a >> 1) | 0x80;
	      *dest = px.ToUI32();
	    }
	    else
	    {
	      const Pixel px (0, 0, 0, 0x80);
	      *dest = px.ToUI32();
	    }
	    
	    Z.Update();
	  }
	  else
	    *dest = 0;
	  dest++;
	  ipol.Advance();
	  Z.Advance();
	} /* endwhile */
      }
    };
    template<typename Source, typename Color, typename Zmode>
    static iScanlineRenderer::ScanlineProc GetScanlineProcSCC (bool needColors)
    {
      if (needColors)
	return ScanlineImpl<Source, Color, Zmode, 1>::Scan;
      else
	return ScanlineImpl<Source, Color, Zmode, 0>::Scan;
    }
    template<typename Source, typename Color>
    static iScanlineRenderer::ScanlineProc GetScanlineProcSC (csZBufMode zmode,
							      bool needColors)
    {
      switch (zmode)
      {
	case CS_ZBUF_NONE:
  	  return GetScanlineProcSCC<Source, Color, ZBufMode_ZNone> (needColors);
	case CS_ZBUF_FILL:
	  return GetScanlineProcSCC<Source, Color, ZBufMode_ZFill> (needColors);
	case CS_ZBUF_TEST:
	  return GetScanlineProcSCC<Source, Color, ZBufMode_ZTest> (needColors);
	case CS_ZBUF_USE:
	  return GetScanlineProcSCC<Source, Color, ZBufMode_ZUse> (needColors);
	case CS_ZBUF_EQUAL:
	  return GetScanlineProcSCC<Source, Color, ZBufMode_ZEqual> (needColors);
	case CS_ZBUF_INVERT:
	  return GetScanlineProcSCC<Source, Color, ZBufMode_ZInvert> (needColors);
	default:
	  return 0;
      }
    }
    template<typename Source>
    static iScanlineRenderer::ScanlineProc GetScanlineProcS (bool colorized, 
							     csZBufMode zmode,
							     bool needColors)
    {
      if (colorized)
	return GetScanlineProcSC<Source, Color_Multiply> (zmode, needColors);
      else
	return GetScanlineProcSC<Source, Color_None> (zmode, needColors);
    }
    static iScanlineRenderer::ScanlineProc GetScanlineProc (bool flat,
							    bool colorized, 
							    csZBufMode zmode,
							    bool needColors)
    {
      if (flat)
	return GetScanlineProcS<Source_Flat> (colorized, zmode, needColors);
      else
	return GetScanlineProcS<Source_Texture> (colorized, zmode, needColors);
    }
  public:
    bool Init (SoftwareTexture** textures,
      const csRenderMeshModes& modes,
      bool needColors,
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

      renderInfo.proc = GetScanlineProc (doFlat, doColor, modes.z_buf_mode,
	needColors);

      return renderInfo.proc != 0;
    }
  };
} // namespace cspluginSoftshader

#endif // __CS_SOFTSHADER_SCANLINE_H__
