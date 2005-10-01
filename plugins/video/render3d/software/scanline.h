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
      ScanlineRenderInfo& renderInfo) = 0;
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
    template <typename Zmode>
    struct ScanlineImpl
    {
      static void Scan (ScanlineRendererBase* _This,
	InterpolateEdgePersp& L, InterpolateEdgePersp& R, 
	int ipolStep, int ipolShift,
	void* dest, uint len, uint32 *zbuff)
      {
	const size_t myIpolFloatNum = 2;
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
	    int u = (int)(ipol.floats[0].c);
	    int32 v = ipol.floats[1].c.GetFixed();
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
	  ipol.Advance();
	  Z.Advance();
	} /* endwhile */
      }
    };
  public:
    ScanlineRenderer (const csPixelFormat& pfmt) : pix (pfmt) 
    {}

    bool Init (csSoftwareTexture** textures,
      const csRenderMeshModes& modes,
      ScanlineRenderInfo& renderInfo)
    {
      renderInfo.renderer = this;
      renderInfo.desiredBuffers = CS_BUFFERFLAG(TEXCOORD);
      csSoftwareTexture* tex = textures[0];
      if (tex == 0) return false;      // @@@ Use flat color instead
      bitmap = tex->bitmap;
      v_shift_r = tex->get_w_shift();
      and_w = tex->get_w_mask();
      and_h = tex->get_h_mask() << v_shift_r;
      v_shift_r = 16 - v_shift_r;

      dnTC.Set (tex->get_width(), tex->get_height(), 0.0f, 0.0f);
      renderInfo.denormFactors = &dnTC;
      renderInfo.denormBuffers = CS_BUFFERFLAG(TEXCOORD);
      static const size_t myBufferComps[] = {2};
      renderInfo.bufferComps = myBufferComps;

      switch (modes.z_buf_mode)
      {
	case CS_ZBUF_NONE:
	  renderInfo.proc = ScanlineImpl<ZBufMode_ZNone>::Scan;
	  break;
	case CS_ZBUF_FILL:
	case CS_ZBUF_FILLONLY:
	  renderInfo.proc = ScanlineImpl<ZBufMode_ZFill>::Scan;
	  break;
	case CS_ZBUF_TEST:
	  renderInfo.proc = ScanlineImpl<ZBufMode_ZTest>::Scan;
	  break;
	case CS_ZBUF_USE:
	  renderInfo.proc = ScanlineImpl<ZBufMode_ZUse>::Scan;
	  break;
	case CS_ZBUF_EQUAL:
	  renderInfo.proc = ScanlineImpl<ZBufMode_ZEqual>::Scan;
	  break;
	case CS_ZBUF_INVERT:
	  renderInfo.proc = ScanlineImpl<ZBufMode_ZInvert>::Scan;
	  break;
	default:
	  return false;
      }
      return true;
    }
  };
} // namespace cspluginSoft3d

#endif // __CS_SOFT3D_SCANLINE_H__
