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

#ifndef __CS_SOFT3D_CLIP_ICLIPPER_H__
#define __CS_SOFT3D_CLIP_ICLIPPER_H__

#include "igeom/clip2d.h"

#include "clipper.h"

//#define CLIP_DEBUG

namespace cspluginSoft3d
{

class ClipMeatiClipper
{
  iClipper2D* clipper;
  size_t maxClipVertices;

  CS_FORCEINLINE size_t CopyTri (const csTriangle& tri, 
    BuffersMask buffersMask, VertexOutputBase& voutPersp, 
    VertexOutputBase* vout)
  {
    voutPersp.Copy (tri.a);
    voutPersp.Copy (tri.b);
    voutPersp.Copy (tri.c);
    int n = 0;
    while (buffersMask != 0)
    {
      vout[n].Copy (tri.a);
      vout[n].Copy (tri.b);
      vout[n].Copy (tri.c);
      buffersMask >>= 1;
      n++;
    }
    return 3;
  }
public:
  void Init (iClipper2D* clipper, size_t maxClipVertices)
  {
    this->clipper = clipper;
    this->maxClipVertices = maxClipVertices;
  }

  size_t DoClip (const csTriangle& tri, const csVector3* inPersp,
    VertexOutputBase& voutPersp, const VertexBuffer* inBuffers, 
    const size_t* inStrides, BuffersMask buffersMask, 
    VertexOutputBase* vout)
  {
    if (!clipper)
      return CopyTri (tri, buffersMask, voutPersp, vout);

    const csVector3 v[3] = 
    {
      inPersp[tri.a],
      inPersp[tri.b],
      inPersp[tri.c],
    };
    const int Tri[3] = {tri.a, tri.b, tri.c};

    csVector2 inpoly[3];
    for (int j = 0; j < 3; j++)
      inpoly[j].Set (v[j].x, v[j].y);
    CS_ALLOC_STACK_ARRAY(csVector2, outPoly, maxClipVertices);
    CS_ALLOC_STACK_ARRAY(csVertexStatus, outStatus, maxClipVertices);
    size_t outNum;

    uint clip_result = clipper->Clip (inpoly, 3, outPoly, outNum, outStatus);
    CS_ASSERT(outNum <= maxClipVertices);

    if (clip_result == CS_CLIP_OUTSIDE) return 0;
    if (clip_result == CS_CLIP_INSIDE)
      return CopyTri (tri, buffersMask, voutPersp, vout);

    for (size_t i = 0 ; i < outNum; i++)
    {
      switch (outStatus[i].Type)
      {
	case CS_VERTEX_ORIGINAL:
	  {
	    const size_t vt = Tri[outStatus[i].Vertex];
	    csVector3 vn (outPoly[i].x, outPoly[i].y,
	      v[outStatus[i].Vertex].z);
	    voutPersp.Write ((float*)&vn);
	    for (size_t n = 0; n < maxBuffers; n++)
	    {
	      if (buffersMask & (1 << n))
	      {
		vout[n].Copy (vt);
	      }
	    }
	  }
	  break;
	case CS_VERTEX_ONEDGE:
	  {
	    const size_t outVt = outStatus[i].Vertex;
	    const size_t vt = Tri[outVt];
	    const size_t vt2 = Tri[(outVt >= 2) ? 0 : outVt + 1];
	    const float t = outStatus[i].Pos;

	    csVector3 vn;
	    voutPersp.LerpTo ((float*)&vn, vt, vt2, t);

#ifdef CLIP_DEBUG
	    if ((ABS(vn.x - outPoly[i].x) > EPSILON)
	      || (ABS(vn.y - outPoly[i].y) > EPSILON))
	      CS_DEBUG_BREAK;
#endif

	    vn.x = outPoly[i].x;
	    vn.y = outPoly[i].y;
	    voutPersp.Write ((float*)&vn);

	    for (size_t n = 0; n < maxBuffers; n++)
	    {
	      if (buffersMask & (1 << n))
	      {
		vout[n].Lerp (vt, vt2, t);
	      }
	    }
	    break;
	  }
	case CS_VERTEX_INSIDE:
	  {
	    float x = outPoly[i].x;
	    float y = outPoly[i].y;
	    size_t edge1 [2], edge2 [2];
	    // Determine edges from which to interpolate the vertex data
	    if ((y >= v[0].y && y <= v[1].y) ||
	      (y <= v[0].y && y >= v[1].y))
	    {
	      edge1[0] = 0;
	      edge1[1] = 1;
	      if ((y >= v[1].y && y <= v[2].y) ||
		(y <= v[1].y && y >= v[2].y))
	      {
		edge2[0] = 1;
		edge2[1] = 2;
	      }
	      else
	      {
		edge2[0] = 0;
		edge2[1] = 2;
	      }
	    }
	    else
	    {
	      edge1[0] = 1;
	      edge1[1] = 2;
	      edge2[0] = 0;
	      edge2[1] = 2;
	    }
	    const csVector3& A = v[edge1 [0]];
	    const csVector3& B = v[edge1 [1]];
	    const csVector3& C = v[edge2 [0]];
	    const csVector3& D = v[edge2 [1]];
	    // Coefficients
            /* dy1/dy2 check to prevent a gcc optimize mode crash
             * however something is _seriously_ off here [res] */ 
            const float dy1 = (B.y - A.y);
	    const float t1 = (dy1 > EPSILON) ? (y - A.y) / dy1 : 1.0f;
            const float dy2 = (B.y - A.y);
	    const float t2 = (dy2 > EPSILON) ? (y - C.y) / dy2 : 1.0f;
	    const float x1 = A.x + t1 * (B.x - A.x);
	    const float x2 = C.x + t2 * (D.x - C.x);
	    const float dx = (x2 - x1);
	    const float t = dx ? ((x - x1) / dx) : 0.0f;

	    const int vt11 = Tri[edge1[0]];
	    const int vt12 = Tri[edge1[1]];
	    const int vt21 = Tri[edge2[0]];
	    const int vt22 = Tri[edge2[1]];

	    csVector3 vn;
	    voutPersp.Lerp3To ((float*)&vn, vt11, vt12, t1,
	      vt21, vt22, t2, t);
	    vn.x = x;
	    vn.y = y;
	    voutPersp.Write ((float*)&vn);

	    for (size_t n = 0; n < maxBuffers; n++)
	    {
	      if (buffersMask & (1 << n))
	      {
		vout[n].Lerp3 (vt11, vt12, t1,
		  vt21, vt22, t2, t);
	      }
	    }
	  }
	  break;
	default:
	  CS_ASSERT(false);
      }
    }

    return outNum;
  }
};

} // namespace cspluginSoft3d

#endif // __CS_SOFT3D_CLIP_ICLIPPER_H__
