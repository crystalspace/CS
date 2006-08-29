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

#ifndef __CS_VERTICES_LTN_H__
#define __CS_VERTICES_LTN_H__

#include "csgeom/math.h"
#include "csutil/dirtyaccessarray.h"
#include "cstool/rbuflock.h"

#include "types.h"

CS_PLUGIN_NAMESPACE_BEGIN(Soft3D)
{
  /**
   * Vertices, Linearized(means: all data for a vertex is stored sequentially
   * in memory), Type-Normalized (means: all data is stored as floats).
   */
  class VerticesLTN
  {
    /// Remaining vertex data
    csDirtyAccessArray<float> linearizedData;
    /// Number of elements to the next vertex data
    size_t stride;
    /// Number of floats per vertex
    size_t floatsPerVert;
    /// Number of components for a buffer
    size_t compCount[activeBufferCount];
    /// Offset of different vertex data types
    size_t offsets[activeBufferCount];

    template<typename T>
    void LinearizeOneBuffer (uint8* srcData, size_t srcStride, 
                             size_t vertexNum, float* dst,
                             size_t dstStride, size_t compCount,
                             size_t compAvail, const float* defComp)
    {
      for (size_t v = 0; v < vertexNum; v++)
      {
        T* src = (T*)srcData;
        const size_t compNum = csMin (compAvail, compCount);
        size_t c = 0;
        for (; c < compNum; c++)
        {
          dst[c] = float (src[c]);
        }
        for (; c < compCount; c++)
        {
          dst[c] = defComp[c];
        }
        dst += dstStride;
        srcData += srcStride;
      }
    }
    void LinearizeOneBuffer (iRenderBuffer* buffer, float* dst,
                             size_t stride);
    void LinearizeOneBuffer (iRenderBuffer* buffer, float* dst,
                             size_t stride, size_t desiredComps);
    void FillOneBuffer (float* dst, size_t stride, size_t desiredComps);
  public:
    VerticesLTN() { Clear(); }

    /// Completely clear.
    void Clear();
    /// Remove vertex data, but keep offsets etc.
    void RemoveVertices()
    { linearizedData.Empty(); }
    void Linearize (iRenderBuffer* buffers[]);
    void Linearize (iRenderBuffer* buffers[], const size_t compCountsPacked[],
      uint desiredBuffers);
    void Linearize (const VertexBuffer buffers[], uint buffersMask,
      size_t vertNum);
    void SetupEmpty (const size_t compCountsPacked[], uint buffersMask);
    void SetupEmpty (const VerticesLTN& other);
    void CopyFromMasked (const VerticesLTN& others, uint buffersMask);

    size_t GetVertexCount() const 
    { return linearizedData.GetSize() / stride; }
    const float* GetData() const { return linearizedData.GetArray(); }
    size_t GetStride() const { return stride; }
    size_t GetFloatsPerVertex() const { return floatsPerVert; }
    size_t GetCompCount (size_t buffer) const
    {
      CS_ASSERT(buffer < activeBufferCount);
      return compCount[buffer];
    }
    size_t GetOffset (size_t buffer) const
    {
      CS_ASSERT(buffer < activeBufferCount);
      return offsets[buffer];
    }

    size_t AddVertex (const float* data)
    {
      const size_t vc = GetVertexCount();
      size_t i;
      for (i = 0; i < floatsPerVert; i++)
      {
        linearizedData.Push (data[i]);
      }
      for (; i < stride; i++)
      {
        linearizedData.Push (0);
      }
      return vc;
    }

    void LerpTo (float* dst, size_t index1, size_t index2, float p) const;
    void Lerp3To (float* dst, size_t index1, size_t index2, float p1,
      size_t index3, size_t index4, float p2, float p) const;

    void Multiply (float* coeff);
  };
}
CS_PLUGIN_NAMESPACE_END(Soft3D)

#endif // __CS_VERTICES_LTN_H__
