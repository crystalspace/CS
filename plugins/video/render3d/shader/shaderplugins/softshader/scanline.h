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

#include "scan_blend.h"
#include "scan_pix.h"
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
    void Apply (const ScanlineComp* /*color*/, 
      uint8& /*r*/, uint8& /*g*/, uint8& /*b*/, uint8& /*a*/) {}
  };

  struct Color_Multiply
  {
    static const size_t compCount = 4;

    const int shift;

    Color_Multiply (ScanlineRendererBase* This) : shift(This->colorShift) {}

    CS_FORCEINLINE
    static uint8 ClampAndShift (int32 x, const int shift)
    {
      return (x & 0x80000000) ? 0 : 
	(((x >> shift) & 0x7fffff00) ? 0xff : (x >> shift));
    }

    CS_FORCEINLINE
    void Apply (const ScanlineComp* color, 
      uint8& r, uint8& g, uint8& b, uint8& a) 
    {
      r = ClampAndShift (r * color[0].c.GetFixed(), shift);
      g = ClampAndShift (g * color[1].c.GetFixed(), shift);
      b = ClampAndShift (b * color[2].c.GetFixed(), shift);
      a = ClampAndShift (a * color[3].c.GetFixed(), shift);
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
    template <typename Source, typename Color, 
      typename SrcBlend, typename DstBlend,
      typename Zmode>
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
	Source colSrc (This);
	Color col (This);

	typename_qualifier Pix::PixType* _dest = 
	  (typename_qualifier Pix::PixType*)dest;
	typename_qualifier Pix::PixType* _destend = _dest + len;
	Zmode Z (ipol, zbuff);

	while (_dest < _destend)
	{
	  if (Z.Test())
	  {
	    uint8 r, g, b, a;
	    colSrc.GetColor (ipol.GetFloat (offsetTC), r, g, b, a);
	    col.Apply (ipol.GetFloat (offsetColor), r, g, b, a);

	    Target_Src srcTarget (r, g, b, a);
	    typename_qualifier SrcBlend::TargetType src (srcTarget);
	    Target_Dst<Pix> dstTarget (src, pix, _dest);
	    typename_qualifier DstBlend::TargetType dst (dstTarget);

	    SrcBlend blendSrc (pix, src);
	    DstBlend blendDst (pix, dst);
	    blendSrc.Apply (_dest, r, g, b, a);
	    blendDst.Apply (_dest, r, g, b, a);
	    
	    Z.Update();
	  }
	  _dest++;
	  ipol.Advance();
	  Z.Advance();
	} /* endwhile */
      }
    };
    template<typename Source, typename Color, typename SrcBlend, 
      typename DstBlend>
    static iScanlineRenderer::ScanlineProc GetScanlineProcSCMM (csZBufMode zmode)
    {
      switch (zmode)
      {
	case CS_ZBUF_NONE:
	  return ScanlineImpl<Source, Color, SrcBlend, DstBlend, 
	    ZBufMode_ZNone>::Scan;
	case CS_ZBUF_FILL:
	  return ScanlineImpl<Source, Color, SrcBlend, DstBlend, 
	    ZBufMode_ZFill>::Scan;
	case CS_ZBUF_TEST:
	  return ScanlineImpl<Source, Color, SrcBlend, DstBlend, 
	    ZBufMode_ZTest>::Scan;
	case CS_ZBUF_USE:
	  return ScanlineImpl<Source, Color, SrcBlend, DstBlend, 
	    ZBufMode_ZUse>::Scan;
	case CS_ZBUF_EQUAL:
	  return ScanlineImpl<Source, Color, SrcBlend, DstBlend, 
	    ZBufMode_ZEqual>::Scan;
	case CS_ZBUF_INVERT:
	  return ScanlineImpl<Source, Color, SrcBlend, DstBlend, 
	    ZBufMode_ZInvert>::Scan;
	default:
	  return 0;
      }
    }
    template<typename Source, typename Color, typename SrcBlend>
    static iScanlineRenderer::ScanlineProc GetScanlineProcSCM (uint mixmode,
      csZBufMode zmode)
    {
      switch (CS_MIXMODE_BLENDOP_DST (mixmode))
      {
	default:
	case CS_MIXMODE_FACT_ZERO:
	  return GetScanlineProcSCMM<Source, Color, SrcBlend,
	    Blend_Zero<Pix, Target_Dst<Pix> > > (zmode);
	case CS_MIXMODE_FACT_ONE:
	  return GetScanlineProcSCMM<Source, Color, SrcBlend,
	    Blend_Zero<Pix, Target_Inv<Target_Dst<Pix> > > > (zmode);

	case CS_MIXMODE_FACT_SRCCOLOR:
	  return GetScanlineProcSCMM<Source, Color, SrcBlend,
	    Blend_SrcColor<Pix, Target_Dst<Pix> > > (zmode);
	case CS_MIXMODE_FACT_SRCCOLOR_INV:
	  return GetScanlineProcSCMM<Source, Color, SrcBlend,
	    Blend_SrcColor<Pix, Target_Inv<Target_Dst<Pix> > > > (zmode);

	case CS_MIXMODE_FACT_SRCALPHA:
	  return GetScanlineProcSCMM<Source, Color, SrcBlend,
	    Blend_SrcAlpha<Pix, Target_Dst<Pix> > > (zmode);
	case CS_MIXMODE_FACT_SRCALPHA_INV:
	  return GetScanlineProcSCMM<Source, Color, SrcBlend,
	    Blend_SrcAlpha<Pix, Target_Inv<Target_Dst<Pix> > > > (zmode);

	case CS_MIXMODE_FACT_DSTCOLOR:
	  return GetScanlineProcSCMM<Source, Color, SrcBlend,
	    Blend_DstColor<Pix, Target_Dst<Pix> > > (zmode);
	case CS_MIXMODE_FACT_DSTCOLOR_INV:
	  return GetScanlineProcSCMM<Source, Color, SrcBlend,
	    Blend_DstColor<Pix, Target_Inv<Target_Dst<Pix> > > > (zmode);

	case CS_MIXMODE_FACT_DSTALPHA:
	  return GetScanlineProcSCMM<Source, Color, SrcBlend,
	    Blend_DstAlpha<Pix, Target_Dst<Pix> > > (zmode);
	case CS_MIXMODE_FACT_DSTALPHA_INV:
	  return GetScanlineProcSCMM<Source, Color, SrcBlend,
	    Blend_DstAlpha<Pix, Target_Inv<Target_Dst<Pix> > > > (zmode);
      }
    }
    template<typename Source, typename Color>
    static iScanlineRenderer::ScanlineProc GetScanlineProcSC (uint mixmode,
      csZBufMode zmode)
    {
      switch (CS_MIXMODE_BLENDOP_SRC (mixmode))
      {
	default:
	case CS_MIXMODE_FACT_ZERO:
	  return GetScanlineProcSCM<Source, Color,
	    Blend_Zero<Pix, Target_Src> > (mixmode, zmode);
	case CS_MIXMODE_FACT_ONE:
	  return GetScanlineProcSCM<Source, Color,
	    Blend_Zero<Pix, Target_Inv<Target_Src> > > (mixmode, zmode);

	case CS_MIXMODE_FACT_SRCCOLOR:
	  return GetScanlineProcSCM<Source, Color,
	    Blend_SrcColor<Pix, Target_Src> > (mixmode, zmode);
	case CS_MIXMODE_FACT_SRCCOLOR_INV:
	  return GetScanlineProcSCM<Source, Color_Multiply,
	    Blend_SrcColor<Pix, Target_Inv<Target_Src> > > (mixmode, zmode);

	case CS_MIXMODE_FACT_SRCALPHA:
	  return GetScanlineProcSCM<Source, Color,
	    Blend_SrcAlpha<Pix, Target_Src> > (mixmode, zmode);
	case CS_MIXMODE_FACT_SRCALPHA_INV:
	  return GetScanlineProcSCM<Source, Color_Multiply,
	    Blend_SrcAlpha<Pix, Target_Inv<Target_Src> > > (mixmode, zmode);

	case CS_MIXMODE_FACT_DSTCOLOR:
	  return GetScanlineProcSCM<Source, Color,
	    Blend_DstColor<Pix, Target_Src> > (mixmode, zmode);
	case CS_MIXMODE_FACT_DSTCOLOR_INV:
	  return GetScanlineProcSCM<Source, Color,
	    Blend_DstColor<Pix, Target_Inv<Target_Src> > > (mixmode, zmode);

	case CS_MIXMODE_FACT_DSTALPHA:
	  return GetScanlineProcSCM<Source, Color,
	    Blend_DstAlpha<Pix, Target_Src> > (mixmode, zmode);
	case CS_MIXMODE_FACT_DSTALPHA_INV:
	  return GetScanlineProcSCM<Source, Color,
	    Blend_DstAlpha<Pix, Target_Inv<Target_Src> > > (mixmode, zmode);
      }
    }
    template<typename Source>
    static iScanlineRenderer::ScanlineProc GetScanlineProcS (bool colorized, 
							     uint mixmode, 
							     csZBufMode zmode)
    {
      if (colorized)
	return GetScanlineProcSC<Source, Color_Multiply> (mixmode, zmode);
      else
	return GetScanlineProcSC<Source, Color_None> (mixmode, zmode);
    }
    static iScanlineRenderer::ScanlineProc GetScanlineProc (bool flat,
							    bool colorized, 
							    uint mixmode,
							    csZBufMode zmode)
    {
      if (flat)
	return GetScanlineProcS<Source_Flat> (colorized, mixmode, zmode);
      else
	return GetScanlineProcS<Source_Texture> (colorized, mixmode, zmode);
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

      uint realMixMode;
      switch (modes.mixmode & CS_MIXMODE_TYPE_MASK)
      {
	case CS_MIXMODE_TYPE_BLENDOP:
	  realMixMode = modes.mixmode;
	  break;
	case CS_MIXMODE_TYPE_AUTO:
	default:
	  if (modes.mixmode & CS_FX_MASK_ALPHA)
	    // @@@ FIXME: respect alpha component
	    realMixMode = CS_FX_ALPHA;
	  else
	    realMixMode = CS_MIXMODE_BLEND(ONE, ZERO);
	  // @@@ FIXME: Determine from texture also
	  break;
      }

      renderInfo.proc = GetScanlineProc (doFlat, doColor, realMixMode, 
	modes.z_buf_mode);

      return renderInfo.proc != 0;
    }
  };
} // namespace cspluginSoftshader

#endif // __CS_SOFTSHADER_SCANLINE_H__
