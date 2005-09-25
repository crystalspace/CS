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

  struct InterpolateEdge
  {
    // The X coordinates and its per-scanline delta
    float x, dxdy;
    // The Z coordinates and its per-scanline delta
    float z, dzdy;
    // Buffer values and per-scanline delta
    csVector4 c[maxBuffers];
    csVector4 dcdy[maxBuffers];

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
	dzdy = (vfv.z - vsv.z) * inv_dy;

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

	z = Lerp (vsv.z, vfv.z, Factor);

	for (size_t b = 0; b < maxBuffers; b++)
	{
	  if (!(buffersMask & (1 << b))) continue;

	  const csVector4 csv (((csVector4*)buffers[b].data)[sv]);
	  const csVector4 cfv (((csVector4*)buffers[b].data)[fv]);

	  c[b] = Lerp (csv, cfv, Factor);
	  dcdy[b] = (cfv - csv) * inv_dy;
	}
      } /* endif */
    }
    void Advance (BuffersMask buffersMask)
    {
      x += dxdy;
      z += dzdy;

      for (size_t b = 0; b < maxBuffers; b++)
      {
	if (!(buffersMask & (1 << b))) continue;
	c[b] += dcdy[b];
      }
    }
  };
  struct InterpolateScanline
  {
    csVector4 c[maxBuffers];
    csVector4 dcdx[maxBuffers];

    void Setup (const InterpolateEdge& L, const InterpolateEdge& R, 
      BuffersMask buffersMask, float inv_l)
    {
      for (size_t b = 0; b < maxBuffers; b++)
      {
	if (!(buffersMask & (1 << b))) continue;
	const csVector4 cL = L.c[b];
	const csVector4 cR = R.c[b];
	c[b] = cL;
	dcdx[b].Set (
	  (cR.x - cL.x) * inv_l,
	  (cR.y - cL.y) * inv_l,
	  (cR.z - cL.z) * inv_l,
	  (cR.w - cL.w) * inv_l);
      }
    }
    void Advance (BuffersMask buffersMask)
    {
      for (size_t b = 0; b < maxBuffers; b++)
      {
	if (!(buffersMask & (1 << b))) continue;
	c[b] += dcdx[b];
      }
    }
  };
    
} // namespace cspluginSoft3d

#define VATTR_BUFINDEX(x)                                               \
  (CS_VATTRIB_ ## x - (CS_VATTRIB_ ## x >=  CS_VATTRIB_GENERIC_FIRST ?  \
  CS_VATTRIB_GENERIC_FIRST : CS_VATTRIB_SPECIFIC_FIRST))

#endif // __CS_SOFT3D_TYPES_H__
