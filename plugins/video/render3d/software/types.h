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

#ifndef __CS_SOFT3D_TYPES_H__
#define __CS_SOFT3D_TYPES_H__

#include "csgeom/fixed.h"

namespace cspluginSoft3d
{
  struct VertexBuffer
  {
    uint8* data;
    size_t comp;
  };
  
  const size_t maxBuffers = 16;
  typedef uint BuffersMask;

  template <typename T>
  static inline T Lerp (const T& a, const T& b, float f)
  { return a + (b-a) * f; }

  struct InterpolateEdgePersp
  {
    // The X coordinates and its per-scanline delta
    float x, dxdy;
    // The inverse Z coordinates and its per-scanline delta
    float Iz, dIzdy;
    // Inverse buffer values and per-scanline delta
    csVector4 Ic[maxBuffers];
    csVector4 dIcdy[maxBuffers];
    // Un-inverted buffer values
    csVector4 c[maxBuffers];

    void Setup (const csVector3* vertices, const VertexBuffer* buffers, 
      const size_t bufNum, size_t sv, size_t fv, int sy)
    {
      const csVector3 vsv (vertices[sv]);
      const csVector3 vfv (vertices[fv]);

      float dy = vsv.y - vfv.y;
      if (dy)
      {
	float inv_dy = 1 / dy;
	x = vsv.x;
	dxdy = (vfv.x - x) * inv_dy;
	const float Isz = vsv.z; // Z coord already inverted here
	const float Ifz = vfv.z;
	dIzdy = (Ifz - Isz) * inv_dy;

	// horizontal pixel correction
	float deltaX = dxdy *
	  (vsv.y - (float (sy) - 0.5));
	x += deltaX;

	// apply sub-pixel accuracy factor
	float Factor;
	if (vfv.x != vsv.x)
	  Factor = deltaX / (vfv.x - vsv.x);
	else
	  Factor = 0;

	Iz = Lerp (Isz, Ifz, Factor);
	const float z = 1.0f/Iz;

	for (size_t b = 0; b < bufNum; b++)
	{
	  // Z coord already inverted here
	  const csVector4 Icsv (((csVector4*)buffers[b].data)[sv]);
	  const csVector4 Icfv (((csVector4*)buffers[b].data)[fv]);

	  Ic[b] = Lerp (Icsv, Icfv, Factor);
	  c[b] = Ic[b] * z;
	  dIcdy[b] = (Icfv - Icsv) * inv_dy;
	}
      } /* endif */
    }
    void Advance (const size_t bufNum)
    {
      Iz += dIzdy;
      const float z = 1.0f/Iz;

      for (size_t b = 0; b < bufNum; b++)
      {
	Ic[b] += dIcdy[b];
	c[b] = Ic[b] * z;
      }
      x += dxdy;
    }
  };
  struct InterpolateScanlinePersp
  {
    int InterpolStep;
    int InterpolShift;
    csFixed24 Iz, dIzdx;
    float Iz_f, dIzdx_f;
    int ipx;
    struct BufData
    {
      csVector4 Ic;
      csVector4 dIcdx;
      CS_ALIGNED_MEMBER(csVector4T<csFixed16> c, 16);
      CS_ALIGNED_MEMBER(csVector4T<csFixed16> dcdx, 16);
    } bufData[maxBuffers];

    void Setup (const InterpolateEdgePersp& L, const InterpolateEdgePersp& R,
      const size_t bufNum, float inv_l, int ipolStep, int ipolShift)
    {
      InterpolStep = ipolStep;
      InterpolShift = ipolShift;

      const float ipf = (float)InterpolStep;
      ipx = InterpolStep;
      float fact = inv_l * ipf;
      Iz = Iz_f = L.Iz;
      dIzdx = dIzdx_f = (R.Iz - L.Iz) * inv_l;
      dIzdx_f *= ipf;
      for (size_t b = 0; b < bufNum; b++)
      {
	const csVector4 cL = L.c[b];
	const csVector4 IcL = L.Ic[b];
	const csVector4 IcR = R.Ic[b];
	bufData[b].c = cL;
	float z2 = 1.0f/(Iz_f + dIzdx_f);
	bufData[b].dIcdx.Set (
	  (IcR.x - IcL.x) * fact,
	  (IcR.y - IcL.y) * fact,
	  (IcR.z - IcL.z) * fact,
	  (IcR.w - IcL.w) * fact);
	bufData[b].Ic = IcL + bufData[b].dIcdx;
	bufData[b].dcdx.Set (
	  (bufData[b].Ic.x*z2 - bufData[b].c.x) >> InterpolShift,
	  (bufData[b].Ic.y*z2 - bufData[b].c.y) >> InterpolShift,
	  (bufData[b].Ic.z*z2 - bufData[b].c.z) >> InterpolShift,
	  (bufData[b].Ic.w*z2 - bufData[b].c.w) >> InterpolShift);
      }
    }
    void Advance (const size_t bufNum)
    {
      if (--ipx > 0)
      {
	Iz += dIzdx;
	for (size_t b = 0; b < bufNum; b++)
	{
	  bufData[b].c += bufData[b].dcdx;
	}
      }
      else
      {
	const float z = 1.0f/Iz_f;
	Iz_f += dIzdx_f;
	Iz = Iz_f;
	const float z2 = 1.0f / Iz_f;
	ipx = InterpolStep;
	for (size_t b = 0; b < bufNum; b++)
	{
	  bufData[b].c = bufData[b].Ic * z;
	  bufData[b].Ic += bufData[b].dIcdx;
	  bufData[b].dcdx.Set (
	    (bufData[b].Ic.x*z2 - bufData[b].c.x) >> InterpolShift,
	    (bufData[b].Ic.y*z2 - bufData[b].c.y) >> InterpolShift,
	    (bufData[b].Ic.z*z2 - bufData[b].c.z) >> InterpolShift,
	    (bufData[b].Ic.w*z2 - bufData[b].c.w) >> InterpolShift);
	}
      }
    }
  };
    
} // namespace cspluginSoft3d

#define VATTR_BUFINDEX(x)                                               \
  (CS_VATTRIB_ ## x - (CS_VATTRIB_ ## x >=  CS_VATTRIB_GENERIC_FIRST ?  \
  CS_VATTRIB_GENERIC_FIRST : CS_VATTRIB_SPECIFIC_FIRST))

#endif // __CS_SOFT3D_TYPES_H__
