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

#include "vertices_ltn.h"

CS_PLUGIN_NAMESPACE_BEGIN(Soft3D)
{
  void VerticesLTN::LinearizeOneBuffer (iRenderBuffer* buffer, float* dst,
                                        size_t stride)
  {
    LinearizeOneBuffer (buffer, dst, stride, buffer->GetComponentCount());
  }

  namespace
  {
    static const float defComps[] = {0, 0, 0, 1};
  }

  void VerticesLTN::LinearizeOneBuffer (iRenderBuffer* buffer, float* dst,
                                        size_t stride, size_t desiredComps)
  {
    size_t compCount = buffer->GetComponentCount();
    csRenderBufferComponentType compType = buffer->GetComponentType();
    size_t vertexNum = csMin (GetVertexCount(), buffer->GetElementCount());
    csRenderBufferLock<uint8> bufLock (buffer, CS_BUF_LOCK_READ);

    switch (compType)
    {
      default:
        CS_ASSERT(false);
      case CS_BUFCOMP_BYTE:
        LinearizeOneBuffer<char> (bufLock, 
          buffer->GetElementDistance(), vertexNum, dst, stride, desiredComps,
          compCount, defComps);
        break;
      case CS_BUFCOMP_UNSIGNED_BYTE:
        LinearizeOneBuffer<unsigned char> (bufLock, 
          buffer->GetElementDistance(), vertexNum, dst, stride, desiredComps,
          compCount, defComps);
        break;
      case CS_BUFCOMP_SHORT:
        LinearizeOneBuffer<short> (bufLock, 
          buffer->GetElementDistance(), vertexNum, dst, stride, desiredComps,
          compCount, defComps);
        break;
      case CS_BUFCOMP_UNSIGNED_SHORT:
        LinearizeOneBuffer<unsigned short> (bufLock, 
          buffer->GetElementDistance(), vertexNum, dst, stride, desiredComps,
          compCount, defComps);
        break;
      case CS_BUFCOMP_INT:
        LinearizeOneBuffer<int> (bufLock, 
          buffer->GetElementDistance(), vertexNum, dst, stride, desiredComps,
          compCount, defComps);
        break;
      case CS_BUFCOMP_UNSIGNED_INT:
        LinearizeOneBuffer<unsigned int> (bufLock, 
          buffer->GetElementDistance(), vertexNum, dst, stride, desiredComps,
          compCount, defComps);
        break;
      case CS_BUFCOMP_FLOAT:
        LinearizeOneBuffer<float> (bufLock, 
          buffer->GetElementDistance(), vertexNum, dst, stride, desiredComps,
          compCount, defComps);
        break;
      case CS_BUFCOMP_DOUBLE:
        LinearizeOneBuffer<double> (bufLock, 
          buffer->GetElementDistance(), vertexNum, dst, stride, desiredComps,
          compCount, defComps);
        break;
    }
  }

  void VerticesLTN::FillOneBuffer (float* dst, size_t stride, size_t desiredComps)
  {
    LinearizeOneBuffer<float> (0, 0, GetVertexCount(), dst, stride, 
      desiredComps, 0, defComps);
  }

  void VerticesLTN::Clear()
  {
    memset (compCount, 0, sizeof (compCount));
    memset (offsets, 0, sizeof (offsets));
    stride = 0;
    floatsPerVert = 0;
    RemoveVertices();
  }

  void VerticesLTN::Linearize (iRenderBuffer* buffers[])
  {
    const size_t vertexNum = buffers[VATTR_SPEC(POSITION)]->GetElementCount();

    size_t totalComponents = 0;
    for (size_t b = 0; b < activeBufferCount; b++)
    {
      if (buffers[b] == 0) continue;
      offsets[b] = totalComponents;
      compCount[b] = buffers[b]->GetComponentCount();
      totalComponents += compCount[b];
    }
    floatsPerVert = totalComponents;
    stride = totalComponents;
    linearizedData.SetSize (vertexNum * stride);

    for (size_t b = 0; b < activeBufferCount; b++)
    {
      if (buffers[b] == 0) 
      {
        compCount[b] = 0;
        continue;
      }
      {
        float* dst = linearizedData.GetArray() + offsets[b];
        LinearizeOneBuffer (buffers[b], dst, stride);
      }
    }
  }

  void VerticesLTN::Linearize (iRenderBuffer* buffers[], 
                               const size_t compCountsPacked[],
                               uint desiredBuffers)
  {
    const size_t vertexNum = buffers[VATTR_SPEC(POSITION)]->GetElementCount();
    const size_t* compPtr = compCountsPacked;

    size_t totalComponents = 0;
    for (size_t b = 0; b < activeBufferCount; b++)
    {
      if (!(desiredBuffers & (1 << b)))
      {
        compCount[b] = 0;
        continue;
      }
      offsets[b] = totalComponents;
      compCount[b] = *compPtr++;
      totalComponents += compCount[b];
    }
    floatsPerVert = totalComponents;
    stride = totalComponents;
    linearizedData.SetSize (vertexNum * stride);

    for (size_t b = 0; b < activeBufferCount; b++)
    {
      if (!(desiredBuffers & (1 << b))) continue;
      float* dst = linearizedData.GetArray() + offsets[b];
      if (buffers[b] == 0) 
      {
        FillOneBuffer (dst, stride, compCount[b]);
      }
      else
      {
        LinearizeOneBuffer (buffers[b], dst, stride, compCount[b]);
      }
    }
  }

  void VerticesLTN::Linearize (const VertexBuffer buffers[], uint buffersMask,
      size_t vertNum)
  {
    size_t totalComponents = 0;
    for (size_t b = 0; b < activeBufferCount; b++)
    {
      if (!(buffersMask & (1 << b))) 
      {
        compCount[b] = 0;
        continue;
      }
      offsets[b] = totalComponents;
      compCount[b] = buffers[b].comp;
      totalComponents += compCount[b];
    }
    floatsPerVert = totalComponents;
    stride = totalComponents;
    linearizedData.SetSize (vertNum * stride);

    for (size_t b = 0; b < activeBufferCount; b++)
    {
      if (!(buffersMask & (1 << b))) continue;
      float* dst = linearizedData.GetArray() + offsets[b];
      LinearizeOneBuffer<float> (buffers[b].data, 
        sizeof (float) * buffers[b].comp, vertNum, dst, stride, 
        buffers[b].comp, buffers[b].comp, 0);
    }
  }

  void VerticesLTN::SetupEmpty (const size_t compCountsPacked[], uint buffersMask)
  {
    const size_t* compPtr = compCountsPacked;
    size_t totalComponents = 0;
    for (size_t b = 0; b < activeBufferCount; b++)
    {
      if (!(buffersMask & (1 << b))) 
      {
        compCount[b] = 0;
        continue;
      }
      offsets[b] = totalComponents;
      compCount[b] = *compPtr++;
      totalComponents += compCount[b];
    }
    floatsPerVert = totalComponents;
    stride = totalComponents;
    linearizedData.SetSize (0);
  }

  void VerticesLTN::SetupEmpty (const VerticesLTN& other)
  {
    linearizedData.SetSize (0);
    stride = other.stride;
    floatsPerVert = other.floatsPerVert;
    memcpy (compCount, other.compCount, sizeof (compCount));
    memcpy (offsets, other.offsets, sizeof (offsets));
  }

  void VerticesLTN::CopyFromMasked (const VerticesLTN& others, uint buffersMask)
  {
    size_t totalComponents = 0;
    for (size_t b = 0; b < activeBufferCount; b++)
    {
      if (!(buffersMask & (1 << b))) 
      {
        compCount[b] = 0;
        continue;
      }
      if (others.compCount[b] == 0) continue;
      offsets[b] = totalComponents;
      compCount[b] = others.compCount[b];
      totalComponents += compCount[b];
    }
    floatsPerVert = totalComponents;
    stride = totalComponents;

    const size_t vertexNum = others.GetVertexCount();
    linearizedData.SetSize (vertexNum * stride);
    for (size_t b = 0; b < activeBufferCount; b++)
    {
      if (compCount[b] == 0) continue;

      float* data = linearizedData.GetArray() + offsets[b];
      const float* otherData = others.GetData() + others.GetOffset (b);
      const size_t otherStride = others.GetStride();
      const size_t vertexSize = compCount[b] * sizeof (float);
      for (size_t v = 0; v < vertexNum; v++)
      {
        memcpy (data, otherData, vertexSize);
        data += stride;
        otherData += otherStride;
      }
    }
  }

  static void DoLerp (size_t num, float* dst, const float* a, const float* b, 
                      float p)
  {
    for (size_t n = 0; n < num; n++)
    {
      const float f1 = *a++;
      const float f2 = *b++;
      *dst++ = f1 + p * (f2-f1);
    }
  }

  void VerticesLTN::LerpTo (float* dst, size_t index1, size_t index2, 
                            float p) const
  {
    const float* data = linearizedData.GetArray();
    const float* a = data + index1 * stride;
    const float* b = data + index2 * stride;
    DoLerp (floatsPerVert, dst, a, b, p);
  }

  void VerticesLTN::Lerp3To (float* dst, size_t index1, size_t index2, 
                             float p1, size_t index3, size_t index4, 
                             float p2, float p) const
  {
    CS_ALLOC_STACK_ARRAY(float, dst1, floatsPerVert);
    CS_ALLOC_STACK_ARRAY(float, dst2, floatsPerVert);
    const float* data = linearizedData.GetArray();
    const float* a = data + index1 * stride;
    const float* b = data + index2 * stride;
    DoLerp (floatsPerVert, dst1, a, b, p1);
    const float* c = data + index3 * stride;
    const float* d = data + index4 * stride;
    DoLerp (floatsPerVert, dst2, c, d, p2);
    DoLerp (floatsPerVert, dst, dst1, dst2, p);
  }

  void VerticesLTN::Multiply (float* coeff)
  {
    const size_t vertexNum = GetVertexCount();
    float* data = linearizedData.GetArray();
    for (size_t v = 0; v < vertexNum; v++)
    {
      for (size_t n = 0; n < floatsPerVert; n++)
      {
        data[n] *= coeff[n];
      }
      data += stride;
      coeff += stride;
    }
  }
}
CS_PLUGIN_NAMESPACE_END(Soft3D)
