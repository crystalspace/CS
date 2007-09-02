/*
    Copyright (C) 2006 by Jorrit Tyberghein
              (C) 2006 by Frank Richter

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

#include "cssysdef.h"

#include "csgeom/tri.h"
#include "csgfx/trianglestream.h"
#include "cstool/rbuflock.h"
#include "igeom/clip2d.h"

#include "vertextransform.h"

CS_PLUGIN_NAMESPACE_BEGIN(Soft3D)
{
  VertexTransform::VertexTransform() : do_near_plane (false)
  {
    bufferPtrs.SetLength (activeBufferCount);
  }

  inline void VertexLTNToVector3 (csVector3& out, const float* data)
  {
    out.Set (data[0], data[1], data[2]);
  }

  void VertexTransform::TransformVertices (
    const csReversibleTransform& object2world, 
    const csReversibleTransform& world2cam, 
    iRenderBuffer* inIndices, csRenderMeshType meshType,
    const VerticesLTN& inBuffers, 
    size_t inIndexStart, size_t inIndexEnd,
    csTriangle*& outTriangles, size_t& outTrianglesCount,
    VerticesLTN& outBuffers, size_t& rangeStart, size_t& rangeEnd)
  {
    const float* inData = inBuffers.GetData ();
    size_t inStride = inBuffers.GetStride ();
    const size_t posOffs = 
      inBuffers.GetOffset (CS_SOFT3D_VA_BUFINDEX (POSITION));
    CS_ALLOC_STACK_ARRAY(float, tr_vert, inStride);

    const csMatrix3& w2c_m = world2cam.GetO2T();
    const csMatrix3& o2w_m = object2world.GetO2T();
    const csVector3& w2c_t = world2cam.GetO2TTranslation();
    const csVector3& o2w_t = object2world.GetO2TTranslation();
    csReversibleTransform object2camera (
      o2w_m * w2c_m,
      w2c_t + w2c_m.GetTranspose()*o2w_t);

    if (!object2camera.IsIdentity())
    {
      size_t i;
      for (i = 0; i < rangeStart; i++)
      {
        outBuffers.AddVertex (inData);
        inData += inStride;
      }
      const size_t maxVert = csMin (rangeEnd, inBuffers.GetVertexCount());
      // Make sure we don't process too many vertices;
      for (; i <= maxVert; i++)
      {
        const csVector3 v (inData[posOffs+0], inData[posOffs+1], 
          inData[posOffs+2]);
        const csVector3 tr_v = object2camera.This2Other (v);
        memcpy (tr_vert, inData, inStride * sizeof (float));
        tr_vert[posOffs+0] = tr_v.x;
        tr_vert[posOffs+1] = tr_v.y;
        tr_vert[posOffs+2] = tr_v.z;
        outBuffers.AddVertex (tr_vert);
        inData += inStride;
      }
    }
    else
    {
      const size_t maxVert = csMin (rangeEnd, inBuffers.GetVertexCount());
      size_t i;
      for (i = 0; i < maxVert; i++)
      {
        outBuffers.AddVertex (inData);
        inData += inStride;
      }
    }

    csRenderBufferLock<uint8> indices (inIndices, CS_BUF_LOCK_READ);

    size_t stride = inIndices->GetElementDistance();
    uint8* tri = indices + inIndexStart*stride;
    const uint8* triEnd = indices + inIndexEnd*stride;

    CS::TriangleIndicesStream<int> triangles;
    triangles.BeginTriangulate (tri, triEnd, stride, 
      inIndices->GetComponentType(), meshType);
    trisArray.Empty();

    if (do_near_plane)
    {
      inData = outBuffers.GetData ();
      while (triangles.HasNextTri())
      {
        csTriangle tri;
        triangles.NextTriangle (tri.a, tri.b, tri.c);
        const size_t Tri[3] = {tri.a, tri.b, tri.c};

        csVector3 inVert[3];
        VertexLTNToVector3 (inVert[0], inData + tri.a * inStride + posOffs);
        VertexLTNToVector3 (inVert[1], inData + tri.b * inStride + posOffs);
        VertexLTNToVector3 (inVert[2], inData + tri.c * inStride + posOffs);
        size_t outCount = 4;
        csVertexStatus outStatus[4];
        uint clipResult = near_plane.ClipPolygon (inVert, 3, 0, 
          outCount, outStatus);
        switch (clipResult)
        {
          case CS_CLIP_INSIDE:
            trisArray.Push (tri);
            break;
          case CS_CLIP_CLIPPED:
            {
              int newIndices[4];
              for (size_t i = 0; i < outCount; i++)
              {
                switch (outStatus[i].Type)
                {
                  case CS_VERTEX_ORIGINAL:
                    newIndices[i] = int (Tri[outStatus[i].Vertex]);
                    break;
                  case CS_VERTEX_ONEDGE:
                    {
	              const size_t outVt = outStatus[i].Vertex;
	              const size_t vt = Tri[outVt];
	              const size_t vt2 = Tri[(outVt >= 2) ? 0 : outVt + 1];
	              const float t = outStatus[i].Pos;
                      outBuffers.LerpTo (tr_vert, vt, vt2, t);
                      newIndices[i] = int (outBuffers.AddVertex (tr_vert));
                      inData = outBuffers.GetData ();
                      rangeEnd = newIndices[i];
                    }
                    break;
                  case CS_VERTEX_INSIDE:
                    CS_ASSERT(false);
                }
              }
              CS_ASSERT(outCount == 3 || outCount == 4);
              tri.a = newIndices[0];
              tri.b = newIndices[1];
              tri.c = newIndices[2];
              trisArray.Push (tri);
              if (outCount == 4)
              {
                tri.a = newIndices[0];
                tri.b = newIndices[2];
                tri.c = newIndices[3];
                trisArray.Push (tri);
              }
            }
            break;
          case CS_CLIP_OUTSIDE:
            /* nothing */
            break;
        }
      }
    }
    else
    {
      while (triangles.HasNextTri())
      {
        csTriangle tri;
        triangles.NextTriangle (tri.a, tri.b, tri.c);
        trisArray.Push (tri);
      }
    }
    outTriangles = trisArray.GetArray();
    outTrianglesCount = trisArray.GetSize();
  }

}
CS_PLUGIN_NAMESPACE_END(Soft3D)

