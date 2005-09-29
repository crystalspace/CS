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
      BuffersMask buffersMask, size_t sv, size_t fv, int sy)
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

	for (size_t b = 0; b < maxBuffers; b++)
	{
	  if (!(buffersMask & (1 << b))) continue;

	  // Z coord already inverted here
	  const csVector4 Icsv (((csVector4*)buffers[b].data)[sv]);
	  const csVector4 Icfv (((csVector4*)buffers[b].data)[fv]);

	  Ic[b] = Lerp (Icsv, Icfv, Factor);
	  c[b] = Ic[b] * z;
	  dIcdy[b] = (Icfv - Icsv) * inv_dy;
	}
      } /* endif */
    }
    void Advance (BuffersMask buffersMask)
    {
      Iz += dIzdy;
      const float z = 1.0f/Iz;

      for (size_t b = 0; b < maxBuffers; b++)
      {
	if (!(buffersMask & (1 << b))) continue;
	Ic[b] += dIcdy[b];
	c[b] = Ic[b] * z;
      }
      x += dxdy;
    }
  };
  struct InterpolateScanlinePersp
  {
    csVector4 c[maxBuffers];
    csVector4 Ic[maxBuffers];
    csVector4 dIcdx[maxBuffers];
    float Iz, dIzdx;
    
    /// Interpolation step for semi-perspective correct texture mapping
    //int InterpolStep;
    /// Interpolation step (shift-value) for semi-perspective correct texture mapping
    //int InterpolShift;

    void Setup (const InterpolateEdgePersp& L, const InterpolateEdgePersp& R,
      BuffersMask buffersMask, float inv_l)
    {
      //InterpolStep = 16;
      //InterpolShift = 4;
      //inv_l = 1.0f / (R.x - L.x);
      Iz = L.Iz;
      dIzdx = (R.Iz - L.Iz) * inv_l;
      for (size_t b = 0; b < maxBuffers; b++)
      {
	if (!(buffersMask & (1 << b))) continue;
	const csVector4 cL = L.c[b];
	const csVector4 IcL = L.Ic[b];
	const csVector4 IcR = R.Ic[b];
	c[b] = cL;
	Ic[b] = IcL;
	dIcdx[b].Set (
	  (IcR.x - IcL.x) * inv_l,
	  (IcR.y - IcL.y) * inv_l,
	  (IcR.z - IcL.z) * inv_l,
	  (IcR.w - IcL.w) * inv_l);
      }
    }
    void Advance (BuffersMask buffersMask)
    {
      Iz += dIzdx;
      const float z = 1.0f/Iz;

      for (size_t b = 0; b < maxBuffers; b++)
      {
	if (!(buffersMask & (1 << b))) continue;
	Ic[b] += dIcdx[b];
	c[b] = Ic[b] * z;
      }
    }
  };
    
} // namespace cspluginSoft3d

#define VATTR_BUFINDEX(x)                                               \
  (CS_VATTRIB_ ## x - (CS_VATTRIB_ ## x >=  CS_VATTRIB_GENERIC_FIRST ?  \
  CS_VATTRIB_GENERIC_FIRST : CS_VATTRIB_SPECIFIC_FIRST))

#endif // __CS_SOFT3D_TYPES_H__
