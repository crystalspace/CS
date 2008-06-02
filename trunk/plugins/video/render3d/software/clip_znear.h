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

#include "csgeom/tri.h"
#include "clipper.h"

CS_PLUGIN_NAMESPACE_BEGIN(Soft3D)
{

#define Z_NEAR	    0.1f

class ClipMeatZNear
{
  int width2;
  int height2;
  float aspect;

  CS_FORCEINLINE void Project (csVector3& v, const float com_iz)
  {
    const float clipPos = Z_NEAR;
    const float com_zv = 1.0f / clipPos;
    v.Set (v.x * com_iz + width2, v.y * com_iz + height2, com_zv);
  }
public:
  void Init (int w2, int h2, float a)
  {
    width2 = w2;
    height2 = h2;
    aspect = a;
  }

  size_t DoClip (const csTriangle& tri, const csVector3* inPersp,
    VertexOutputPersp& voutPersp, 
    const VerticesLTN* inBuffers, VerticesLTN* outBuffers)
  {
    const float clipPos = Z_NEAR;
    const float com_zv = 1.0f / clipPos;
    const float com_iz = aspect * com_zv;
    csVector3 v;

    const float* inData = inBuffers->GetData();
    const size_t posOffs = 
      inBuffers->GetOffset (CS_SOFT3D_VA_BUFINDEX(POSITION));
    const float* bPos = inData + posOffs;
    const size_t stride = inBuffers->GetStride();
    const csVector3& va = *(csVector3*)(bPos + tri.a * stride);
    const csVector3& vb = *(csVector3*)(bPos + tri.b * stride);
    const csVector3& vc = *(csVector3*)(bPos + tri.c * stride);
    int cnt_vis = int (va.z >= Z_NEAR) +
                  int (vb.z >= Z_NEAR) +
                  int (vc.z >= Z_NEAR);
    if (cnt_vis == 0)
    {
      //=====
      // Easy case: the triangle is completely not visible.
      //=====
      return 0;
    }
    else if (cnt_vis == 3)
    {
      //=====
      // Another easy case: all vertices are visible or we are using
      // lazy clipping in which case we draw the triangle completely.
      //=====
      voutPersp.Write (inPersp[tri.a]);
      voutPersp.Write (inPersp[tri.b]);
      voutPersp.Write (inPersp[tri.c]);
      outBuffers->AddVertex (inData + tri.a * stride);
      outBuffers->AddVertex (inData + tri.b * stride);
      outBuffers->AddVertex (inData + tri.c * stride);
      return 3;
    }
    else if (cnt_vis == 1)
    {
      //=====
      // A reasonably complex case: one vertex is visible. We need
      // to clip to the Z-plane but fortunatelly this will result in
      // another triangle.
      //=====

      if (va.z >= Z_NEAR)
      {
        // Calculate intersection between a-b and Z=clipPos.
        // p = a + r * (b-a) (parametric line equation between a and b).
        const float r1 = (clipPos-va.z)/(vb.z-va.z);
        // Calculate intersection between a-c and Z=clipPos.
        const float r2 = (clipPos-va.z)/(vc.z-va.z);

	voutPersp.Write (inPersp[tri.a]);
        outBuffers->AddVertex (inData + tri.a * stride);

        CS_ALLOC_STACK_ARRAY(float, v2, inBuffers->GetStride());

        inBuffers->LerpTo (v2, tri.a, tri.b, r1);
        v.Set (v2[posOffs + 0], v2[posOffs + 1], v2[posOffs + 2]);
	Project (v, com_iz);
	voutPersp.Write (v);
        outBuffers->AddVertex (v2);

        inBuffers->LerpTo (v2, tri.a, tri.c, r2);
        v.Set (v2[posOffs + 0], v2[posOffs + 1], v2[posOffs + 2]);
	Project (v, com_iz);
	voutPersp.Write (v);
        outBuffers->AddVertex (v2);
      }
      else if (vb.z >= Z_NEAR)
      {
        // Calculate intersection between b-a and Z=clipPos.
        const float r1 = (clipPos-vb.z)/(va.z-vb.z);
        // Calculate intersection between b-c and Z=clipPos.
        const float r2 = (clipPos-vb.z)/(vc.z-vb.z);

        CS_ALLOC_STACK_ARRAY(float, v2, inBuffers->GetStride());

        inBuffers->LerpTo (v2, tri.b, tri.a, r1);
        v.Set (v2[posOffs + 0], v2[posOffs + 1], v2[posOffs + 2]);
	Project (v, com_iz);
	voutPersp.Write (v);
        outBuffers->AddVertex (v2);

	voutPersp.Write (inPersp[tri.b]);
        outBuffers->AddVertex (inData + tri.b * stride);

        inBuffers->LerpTo (v2, tri.b, tri.c, r2);
        v.Set (v2[posOffs + 0], v2[posOffs + 1], v2[posOffs + 2]);
	Project (v, com_iz);
	voutPersp.Write (v);
        outBuffers->AddVertex (v2);
      }
      else
      {
        // Calculate intersection between c-a and Z=clipPos.
        const float r1 = (clipPos-vc.z)/(va.z-vc.z);
        // Calculate intersection between c-b and Z=clipPos.
        const float r2 = (clipPos-vc.z)/(vb.z-vc.z);

        CS_ALLOC_STACK_ARRAY(float, v2, inBuffers->GetStride());

        inBuffers->LerpTo (v2, tri.c, tri.a, r1);
        v.Set (v2[posOffs + 0], v2[posOffs + 1], v2[posOffs + 2]);
	Project (v, com_iz);
	voutPersp.Write (v);
        outBuffers->AddVertex (v2);

        inBuffers->LerpTo (v2, tri.c, tri.b, r2);
        v.Set (v2[posOffs + 0], v2[posOffs + 1], v2[posOffs + 2]);
	Project (v, com_iz);
	voutPersp.Write (v);
        outBuffers->AddVertex (v2);

	voutPersp.Write (inPersp[tri.c]);
        outBuffers->AddVertex (inData + tri.c * stride);
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

      if (va.z < Z_NEAR)
      {
	// Calculate intersection between a-b and Z=clipPos.
	// p = a + r * (b-a) (parametric line equation between a and b).
	const float r1 = (clipPos-va.z)/(vb.z-va.z);
        // Calculate intersection between a-c and Z=.
        const float r2 = (clipPos-va.z)/(vc.z-va.z);

        CS_ALLOC_STACK_ARRAY(float, v2, inBuffers->GetStride());

        inBuffers->LerpTo (v2, tri.a, tri.b, r1);
        v.Set (v2[posOffs + 0], v2[posOffs + 1], v2[posOffs + 2]);
	Project (v, com_iz);
	voutPersp.Write (v);
        outBuffers->AddVertex (v2);

	voutPersp.Write (inPersp[tri.b]);
        outBuffers->AddVertex (inData + tri.b * stride);
	voutPersp.Write (inPersp[tri.c]);
        outBuffers->AddVertex (inData + tri.c * stride);

        inBuffers->LerpTo (v2, tri.a, tri.c, r2);
        v.Set (v2[posOffs + 0], v2[posOffs + 1], v2[posOffs + 2]);
	Project (v, com_iz);
	voutPersp.Write (v);
        outBuffers->AddVertex (v2);
      }
      else if (vb.z < Z_NEAR)
      {
        // Calculate intersection between b-a and Z=clipPos.
        const float r1 = (clipPos-vb.z)/(va.z-vb.z);
        // Calculate intersection between b-c and Z=clipPos.
        const float r2 = (clipPos-vb.z)/(vc.z-vb.z);

        CS_ALLOC_STACK_ARRAY(float, v2, inBuffers->GetStride());

	voutPersp.Write (inPersp[tri.a]);
        outBuffers->AddVertex (inData + tri.a * stride);

        inBuffers->LerpTo (v2, tri.b, tri.a, r1);
        v.Set (v2[posOffs + 0], v2[posOffs + 1], v2[posOffs + 2]);
	Project (v, com_iz);
	voutPersp.Write (v);
        outBuffers->AddVertex (v2);

        inBuffers->LerpTo (v2, tri.b, tri.c, r2);
        v.Set (v2[posOffs + 0], v2[posOffs + 1], v2[posOffs + 2]);
	Project (v, com_iz);
	voutPersp.Write (v);
        outBuffers->AddVertex (v2);

	voutPersp.Write (inPersp[tri.c]);
        outBuffers->AddVertex (inData + tri.c * stride);
      }
      else
      {
        // Calculate intersection between c-b and Z=clipPos.
        const float r1 = (clipPos-vc.z)/(vb.z-vc.z);
        // Calculate intersection between c-a and Z=clipPos.
        const float r2 = (clipPos-vc.z)/(va.z-vc.z);

        CS_ALLOC_STACK_ARRAY(float, v2, inBuffers->GetStride());

	voutPersp.Write (inPersp[tri.a]);
        outBuffers->AddVertex (inData + tri.a * stride);
	voutPersp.Write (inPersp[tri.b]);
        outBuffers->AddVertex (inData + tri.b * stride);

        inBuffers->LerpTo (v2, tri.c, tri.b, r1);
        v.Set (v2[posOffs + 0], v2[posOffs + 1], v2[posOffs + 2]);
	Project (v, com_iz);
	voutPersp.Write (v);
        outBuffers->AddVertex (v2);

        inBuffers->LerpTo (v2, tri.c, tri.a, r2);
        v.Set (v2[posOffs + 0], v2[posOffs + 1], v2[posOffs + 2]);
	Project (v, com_iz);
	voutPersp.Write (v);
        outBuffers->AddVertex (v2);
      }
      return 4;
    }
    CS_ASSERT(false);
    return 0;
  }
};

}
CS_PLUGIN_NAMESPACE_END(Soft3D)

#endif // __CS_SOFT3D_CLIP_ZNEAR_H__
