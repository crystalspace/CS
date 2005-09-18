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

namespace cspluginSoft3d
{

class ClipMeatiClipper
{
  iClipper2D* clipper;
  size_t maxClipVertices;

  CS_FORCEINLINE size_t CopyTri (const csTriangle& tri, 
    ClipBuffersMask buffersMask, VertexOutputBase* vout)
  {
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

  size_t DoClip (const csTriangle& tri, const ClipBuffer* buffers, 
    ClipBuffersMask buffersMask, VertexOutputBase* vout)
  {
    if (!clipper)
      return CopyTri (tri, buffersMask, vout);

    const ClipBuffer& bPos = buffers[VATTR_CLIPINDEX(POSITION)];
    const csVector3 v[3] = 
    {
      *(csVector3*)(bPos.source + tri.a * bPos.sourceStride),
      *(csVector3*)(bPos.source + tri.b * bPos.sourceStride),
      *(csVector3*)(bPos.source + tri.c * bPos.sourceStride),
    };

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
      return CopyTri (tri, buffersMask, vout);

    for (size_t i = 0 ; i < outNum; i++)
    {
      switch (outStatus[i].Type)
      {
	case CS_VERTEX_ORIGINAL:
	  {
	    for (size_t n = 0; n < clipMaxBuffers; n++)
	    {
	      if (buffersMask & (1 << n))
	      {
		if (n == VATTR_CLIPINDEX(POSITION))
		{
		  csVector3 vn (outPoly[i].x, outPoly[i].y,
		    v[outStatus[i].Vertex].z);
		  vout[n].Write ((float*)&vn);
		}
		else
		  vout[n].Copy (outStatus[i].Vertex);
	      }
	    }
	  }
	  break;
	case CS_VERTEX_ONEDGE:
	  {
	    const size_t vt = outStatus[i].Vertex;
	    const size_t vt2 = (vt >= 2) ? 0 : vt + 1;
	    const float t = outStatus[i].Pos;
	    for (size_t n = 0; n < clipMaxBuffers; n++)
	    {
	      if (buffersMask & (1 << n))
	      {
		if (n == VATTR_CLIPINDEX(POSITION))
		{
		  csVector3 vn;
		  vout[n].LerpTo ((float*)&vn, vt, vt2, t);
		  vn.x = outPoly[i].x;
		  vn.y = outPoly[i].y;
		  vout[n].Write ((float*)&vn);
		}
		else
		  vout[n].Lerp (vt, vt2, t);
	      }
	    }
	  }
	  break;
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
	    const float t1 = (y - A.y) / (B.y - A.y);
	    const float t2 = (y - C.y) / (D.y - C.y);
	    const float x1 = A.x + t1 * (B.x - A.x);
	    const float x2 = C.x + t2 * (D.x - C.x);
	    const float t = (x - x1) / (x2 - x1);
	    for (size_t n = 0; n < clipMaxBuffers; n++)
	    {
	      if (buffersMask & (1 << n))
	      {
		if (n == VATTR_CLIPINDEX(POSITION))
		{
		  csVector3 vn;
		  vout[n].Lerp3To ((float*)&vn, edge1[0], edge1[1], t1,
		    edge2[0], edge2[1], t2, t);
		  vn.x = x;
		  vn.y = y;
		  vout[n].Write ((float*)&vn);
		}
		else
		  vout[n].Lerp3 (edge1[0], edge1[1], t1,
		    edge2[0], edge2[1], t2, t);
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
