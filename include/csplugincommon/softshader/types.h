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

#ifndef __CS_CSPLUGINCOMMON_SOFTSHADER_TYPES_H__
#define __CS_CSPLUGINCOMMON_SOFTSHADER_TYPES_H__

/**\file
 * Software renderer common types
 */

#include "csgeom/fixed.h"

/**\addtogroup plugincommon
 * @{ */

namespace CrystalSpace
{
  namespace SoftShader
  {
    /// Maximum number of vertex buffers supported
    const size_t maxBuffers = 16;
    /// Type for a bit mask of buffers
    typedef uint BuffersMask;
  
    /// Helper function to linearly interpolate
    template <typename T>
    static inline T Lerp (const T& a, const T& b, float f)
    { return a + (b-a) * f; }
  
    /// Interpolator for a polygon edge
    struct InterpolateEdgePersp
    {
      /// X coordinate
      float x;
      ///  X per-scanline delta
      float dxdy;
      /// The inverse Z coordinate
      float Iz;
      /// Inverse Z per-scanline delta
      float dIzdy;
      /// A single interpolated buffer value
      struct PerFloat
      {
	/// Buffer value div Z
	float Ic;
	/// Buffer value div Z per-scanline delta
	float dIcdy;
	/// Un-divided buffer value
	float c;
      } Floats[maxBuffers*4];
  
      /// Setup interpolator
      void Setup (const csVector3* vertices, const float* floats, 
	const size_t floatNum, size_t sv, size_t fv, int sy)
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
  
	    // Z coord already inverted here
	  const float* Icsv = floats + sv*floatNum;
	  const float* Icfv = floats + fv*floatNum;
	  for (size_t f = 0; f < floatNum; f++)
	  {
	    const float fs = *Icsv++; 
	    const float ff = *Icfv++; 
	    Floats[f].Ic = Lerp (fs, ff, Factor);
	    Floats[f].c = Floats[f].Ic * z;
	    Floats[f].dIcdy = (ff - fs) * inv_dy;
	  }
	} /* endif */
      }
      /// Advance a line down
      void Advance (const size_t floatNum)
      {
	Iz += dIzdy;
	const float z = 1.0f/Iz;
  
	for (size_t f = 0; f < floatNum; f++)
	{
	  Floats[f].Ic += Floats[f].dIcdy;
	  Floats[f].c = Floats[f].Ic * z;
	}
	x += dxdy;
      }
    };
    /// Interpolator over a scanline, common code
    struct InterpolateScanlinePerspCommon
    {
      /// 1/z and per-pixel delta
      csFixed24 Iz, dIzdx;
      /// 1/f - float for better accuracy, delta is for 1 interpolation span
      float Iz_f, dIzdx_f;
    };
  
    /// Undivided buffer values
    struct ScanlineComp
    {
      /// "Direct" component
      csFixed16 c;
      /// "Direct" component delta
      csFixed16 dcdx;
    };
    /// Buffer values div Z
    struct ScanlineCompDivZ
    {
      /// 1/component
      float Ic;
      /// 1/component delta
      float dIcdx;
    };
    /// Interpolator over a scanline
    template<int maxFloats>
    struct InterpolateScanlinePersp : public InterpolateScanlinePerspCommon
    {
      /// Buffer values
      ScanlineComp floats[maxFloats];
      /// Buffer values div Z
      ScanlineCompDivZ floats_f[maxFloats];
  
      /// Interpolation step
      int InterpolStep;
      /// Interpolation shift
      int InterpolShift;
      /// Remaining pixels for this interpolation span
      int ipx;
  
      /// Setup interpolation over a scanline, given left and right edge
      void Setup (const InterpolateEdgePersp& L, const InterpolateEdgePersp& R,
	float inv_l, int ipolStep, int ipolShift)
      {
	InterpolStep = ipolStep;
	InterpolShift = ipolShift;
  
	const float ipf = (float)InterpolStep;
	ipx = InterpolStep;
	float fact = inv_l * ipf;
	Iz = Iz_f = L.Iz;
	dIzdx = dIzdx_f = (R.Iz - L.Iz) * inv_l;
	dIzdx_f *= ipf;
	Iz_f += dIzdx_f;
	float z2 = 1.0f / Iz_f;
	for (size_t f = 0; f < maxFloats; f++)
	{
	  const float cL = L.Floats[f].c;
	  const float IcL = L.Floats[f].Ic;
	  const float IcR = R.Floats[f].Ic;
	  floats[f].c = cL;
	  floats_f[f].dIcdx = (IcR - IcL) * fact;
	  floats_f[f].Ic = IcL + floats_f[f].dIcdx;
	  floats[f].dcdx = (floats_f[f].Ic*z2 - floats[f].c) >> InterpolShift;
	}
      }
      /// Advance a pixel right
      void Advance ()
      {
	if (--ipx > 0)
	{
	  Iz += dIzdx;
	  for (size_t f = 0; f < maxFloats; f++)
	  {
	    floats[f].c += floats[f].dcdx;
	  }
	}
	else
	{
	  /* Note: Iz_f is "ahead" one interpolation span, ie when the
	   * deltas are set up for the next, it has the value from the
	   * beginning if that span. */
	  const float z = 1.0f/Iz_f;
	  Iz = Iz_f;
	  Iz_f += dIzdx_f;
	  const float z2 = 1.0f / Iz_f;
	  ipx = InterpolStep;
	  for (size_t f = 0; f < maxFloats; f++)
	  {
	    floats[f].c = floats_f[f].Ic * z;
	    floats_f[f].Ic += floats_f[f].dIcdx;
	    floats[f].dcdx = (floats_f[f].Ic*z2 - floats[f].c) >> InterpolShift;
	  }
	}
      }
    };

  } // namespace SoftShader
} // namespace CrystalSpace

/**
 * Helper macro to obtain a software renderer buffer index for a vertex 
 * attribute.
*/
#define CS_SOFT3D_VA_BUFINDEX(x)                                        \
  (CS_VATTRIB_ ## x - (CS_VATTRIB_ ## x >=  CS_VATTRIB_GENERIC_FIRST ?  \
  CS_VATTRIB_GENERIC_FIRST : CS_VATTRIB_SPECIFIC_FIRST))
/**
 * Helper macro to obtain a software renderer buffer mask flag for a vertex 
 * attribute.
*/
#define CS_SOFT3D_BUFFERFLAG(x)			(1 << CS_SOFT3D_VA_BUFINDEX(x))

/** @} */

#endif // __CS_CSPLUGINCOMMON_SOFTSHADER_TYPES_H__
