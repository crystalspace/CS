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

#ifndef __CS_SOFT3D_CLIP_ZNEAR_H__
#define __CS_SOFT3D_CLIP_ZNEAR_H__

#include "clipper.h"

namespace cspluginSoft3d
{

class ClipMeatZNear
{
  int width2;
  int height2;
  float aspect;

  CS_FORCEINLINE void Project (csVector4& v, const float com_iz)
  {
    const float clipPos = SMALL_Z*10;
    const float com_zv = 1.0f / clipPos;
    v.Set (v.x * com_iz + width2, v.y * com_iz + height2, com_zv, 1.0f);
  }
public:
  void Init (int w2, int h2, float a)
  {
    width2 = w2;
    height2 = h2;
    aspect = a;
  }

  size_t DoClip (const csTriangle& tri, const csVector3* inPersp,
    VertexOutputBase& voutPersp, const VertexBuffer* inBuffers, 
    const size_t* inStrides, BuffersMask buffersMask, 
    VertexOutputBase* vout)
  {
    const float clipPos = SMALL_Z*10;
    const float com_zv = 1.0f / clipPos;
    const float com_iz = aspect * com_zv;
    csVector4 v;

    const VertexBuffer& bPos = inBuffers[CS_VATTR_BUFINDEX(POSITION)];
    const size_t bPosStride = inStrides[CS_VATTR_BUFINDEX(POSITION)];
    VertexOutputBase& voutPos = vout[CS_VATTR_BUFINDEX(POSITION)];
    const csVector3& va = *(csVector3*)(bPos.data + tri.a * bPosStride);
    const csVector3& vb = *(csVector3*)(bPos.data + tri.b * bPosStride);
    const csVector3& vc = *(csVector3*)(bPos.data + tri.c * bPosStride);
    int cnt_vis = int (va.z >= SMALL_Z) +
                  int (vb.z >= SMALL_Z) +
                  int (vc.z >= SMALL_Z);
    if (cnt_vis == 0)
    {
      //=====
      // Easy case: the triangle is completely not visible.
      //=====
      return 0;
    }
    else if (cnt_vis == 3/* || lazyclip*/)
    {
      //=====
      // Another easy case: all vertices are visible or we are using
      // lazy clipping in which case we draw the triangle completely.
      //=====
      voutPersp.Write ((float*)&inPersp[tri.a]);
      voutPersp.Write ((float*)&inPersp[tri.b]);
      voutPersp.Write ((float*)&inPersp[tri.c]);
      int n = 0;
      while (buffersMask != 0)
      {
        if (buffersMask & 1)
        {
	  vout[n].Copy (tri.a);
	  vout[n].Copy (tri.b);
	  vout[n].Copy (tri.c);
        }
        buffersMask >>= 1;
        n++;
      }
      return 3;
    }
    else if (cnt_vis == 1)
    {
      //=====
      // A reasonably complex case: one vertex is visible. We need
      // to clip to the Z-plane but fortunatelly this will result in
      // another triangle.
      //=====

      if (va.z >= SMALL_Z)
      {
        // Calculate intersection between a-b and Z=clipPos.
        // p = a + r * (b-a) (parametric line equation between a and b).
        const float r1 = (clipPos-va.z)/(vb.z-va.z);
        // Calculate intersection between a-c and Z=clipPos.
        const float r2 = (clipPos-va.z)/(vc.z-va.z);

	voutPersp.Write ((float*)&inPersp[tri.a]);

	voutPos.LerpTo ((float*)&v, tri.b, tri.a, r1);
	Project (v, com_iz);
	voutPersp.Write ((float*)&v);

	voutPos.LerpTo ((float*)&v, tri.c, tri.a, r2);
	Project (v, com_iz);
	voutPersp.Write ((float*)&v);

        int n = 0;
        while (buffersMask != 0)
        {
          if (buffersMask & 1)
          {
	    // Point a is visible.
	    vout[n].Copy (tri.a);
	    vout[n].Lerp (tri.b, tri.a, r1);
	    vout[n].Lerp (tri.c, tri.a, r2);
          }
          buffersMask >>= 1;
          n++;
        }
      }
      else if (vb.z >= SMALL_Z)
      {
        // Calculate intersection between b-a and Z=clipPos.
        const float r1 = (clipPos-vb.z)/(va.z-vb.z);
        // Calculate intersection between b-c and Z=clipPos.
        const float r2 = (clipPos-vb.z)/(vc.z-vb.z);

	voutPos.LerpTo ((float*)&v, tri.a, tri.b, r1);
	Project (v, com_iz);
	voutPersp.Write ((float*)&v);

	voutPersp.Write ((float*)&inPersp[tri.b]);

	voutPos.LerpTo ((float*)&v, tri.c, tri.b, r2);
	Project (v, com_iz);
	voutPersp.Write ((float*)&v);

        int n = 0;
        while (buffersMask != 0)
        {
          if (buffersMask & 1)
          {
	    vout[n].Lerp (tri.a, tri.b, r1);
	    // Point b is visible
	    vout[n].Copy (tri.b);
	    vout[n].Lerp (tri.c, tri.b, r2);
          }
          buffersMask >>= 1;
          n++;
        }
      }
      else
      {
        // Calculate intersection between c-a and Z=clipPos.
        const float r1 = (clipPos-vc.z)/(va.z-vc.z);
        // Calculate intersection between c-b and Z=clipPos.
        const float r2 = (clipPos-vc.z)/(vb.z-vc.z);

	voutPos.LerpTo ((float*)&v, tri.a, tri.c, r1);
	Project (v, com_iz);
	voutPersp.Write ((float*)&v);

	voutPos.LerpTo ((float*)&v, tri.b, tri.c, r2);
	Project (v, com_iz);
	voutPersp.Write ((float*)&v);

	voutPersp.Write ((float*)&inPersp[tri.c]);

        int n = 0;
        while (buffersMask != 0)
        {
          if (buffersMask & 1)
          {
	    vout[n].Lerp (tri.a, tri.c, r1);
	    vout[n].Lerp (tri.b, tri.c, r2);
	    // Point c is visible
	    vout[n].Copy (tri.c);
          }
          buffersMask >>= 1;
          n++;
        }
      }
      return 3;
    }
    else
    {
      //=====
      // The most complicated case: two vertices are visible. In this
      // case clipping to the Z-plane does not result in a triangle.
      // So we have to triangulate.
      // We will triangulate to triangles a,b,c, and a,c,d.
      //=====

      if (va.z < SMALL_Z)
      {
	// Calculate intersection between a-b and Z=clipPos.
	// p = a + r * (b-a) (parametric line equation between a and b).
	const float r1 = (clipPos-va.z)/(vb.z-va.z);
        // Calculate intersection between a-c and Z=.
        const float r2 = (clipPos-va.z)/(vc.z-va.z);

	voutPos.LerpTo ((float*)&v, tri.a, tri.b, r1);
	Project (v, com_iz);
	voutPersp.Write ((float*)&v);

	voutPersp.Write ((float*)&inPersp[tri.b]);
	voutPersp.Write ((float*)&inPersp[tri.c]);

	voutPos.LerpTo ((float*)&v, tri.a, tri.c, r2);
	Project (v, com_iz);
	voutPersp.Write ((float*)&v);

        int n = 0;
        while (buffersMask != 0)
        {
          if (buffersMask & 1)
          {
	    vout[n].Lerp (tri.a, tri.b, r1);
	    // Point a is not visible.
	    vout[n].Copy (tri.b);
	    vout[n].Copy (tri.c);
	    vout[n].Lerp (tri.a, tri.c, r2);
          }
          buffersMask >>= 1;
          n++;
        }
      }
      else if (vb.z < SMALL_Z)
      {
        // Calculate intersection between b-a and Z=clipPos.
        const float r1 = (clipPos-vb.z)/(va.z-vb.z);
        // Calculate intersection between b-c and Z=clipPos.
        const float r2 = (clipPos-vb.z)/(vc.z-vb.z);

	voutPersp.Write ((float*)&inPersp[tri.a]);

	voutPos.LerpTo ((float*)&v, tri.b, tri.a, r1);
	Project (v, com_iz);
	voutPersp.Write ((float*)&v);

	voutPos.LerpTo ((float*)&v, tri.b, tri.c, r2);
	Project (v, com_iz);
	voutPersp.Write ((float*)&v);

	voutPersp.Write ((float*)&inPersp[tri.c]);

        int n = 0;
        while (buffersMask != 0)
        {
          if (buffersMask & 1)
          {
	    // Point b is not visible.
	    vout[n].Copy (tri.a);
            
	    vout[n].Lerp (tri.b, tri.a, r1);
	    vout[n].Lerp (tri.b, tri.c, r2);
            
	    vout[n].Copy (tri.c);
          }
          buffersMask >>= 1;
          n++;
        }
      }
      else
      {
        // Calculate intersection between c-a and Z=clipPos.
        const float r1 = (clipPos-vc.z)/(va.z-vc.z);
        // Calculate intersection between c-b and Z=clipPos.
        const float r2 = (clipPos-vc.z)/(vb.z-vc.z);

	voutPersp.Write ((float*)&inPersp[tri.a]);
	voutPersp.Write ((float*)&inPersp[tri.b]);

	voutPos.LerpTo ((float*)&v, tri.c, tri.a, r1);
	Project (v, com_iz);
	voutPersp.Write ((float*)&v);

	voutPos.LerpTo ((float*)&v, tri.c, tri.b, r2);
	Project (v, com_iz);
	voutPersp.Write ((float*)&v);

        int n = 0;
        while (buffersMask != 0)
        {
          if (buffersMask & 1)
          {
	    // Point c is not visible.
	    vout[n].Copy (tri.a);
	    vout[n].Copy (tri.b);
            
	    vout[n].Lerp (tri.c, tri.a, r1);
	    vout[n].Lerp (tri.c, tri.b, r2);
          }
          buffersMask >>= 1;
          n++;
        }
      }
      return 4;
    }
    CS_ASSERT(false);
    return 0;
  }
};

} // namespace cspluginSoft3d

#endif // __CS_SOFT3D_CLIP_ZNEAR_H__
