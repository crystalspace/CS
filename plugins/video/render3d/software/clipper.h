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

#ifndef __CS_SOFT3D_CLIPPER_H__
#define __CS_SOFT3D_CLIPPER_H__

class VertexOutputBase
{
protected:
  uint8* in;
  float* out;
  size_t inStride;
public:
  VertexOutputBase () {}
  VertexOutputBase (uint8* in, size_t inStride, float* out) :
    in(in), out(out), inStride(inStride) {}
  
  virtual void Copy (size_t idx) {}
  virtual void LerpTo (float* dst, size_t idx1, size_t idx2, float p) {}
  virtual void Lerp (size_t idx1, size_t idx2, float p) {}
  virtual void Write (float* what) {}
};

template<int Ni, int No>
class VertexOutput : public VertexOutputBase
{
  void Lerp (float*& dst, size_t idx1, size_t idx2, float p) 
  {
    const uint8* I1 = (uint8*)in + idx1*inStride;
    const uint8* I2 = (uint8*)in + idx2*inStride;
    for (int i = 0; i < No; i++)
    {
      if (i < Ni)
      {
        const float f1 = *((float*)I1);
        const float f2 = *((float*)I2);
        *dst = f1 + p * (f2-f1);
        I1 += sizeof(float);
        I2 += sizeof(float);
      }
      else
        *dst = (i == 3) ? 1.0f : 0.0;
      dst++;
    }
  }
public:
  VertexOutput (uint8* in, size_t inStride, float* out) :
    VertexOutputBase (in, inStride, out) {}

  virtual void Copy (size_t idx)
  {
    const uint8* I = in + idx*inStride;
    for (int i = 0; i < No; i++)
    {
      if (i < Ni)
      {
        *out = *((float*)I);
        I += sizeof(float);
      }
      else
        *out = (i == 3) ? 1.0f : 0.0;
      out++;
    }
  }
  virtual void LerpTo (float* dst, size_t idx1, size_t idx2, float p) 
  {
    Lerp (dst, idx1, idx2, p);
  }
  virtual void Lerp (size_t idx1, size_t idx2, float p) 
  {
    Lerp (out, idx1, idx2, p);
  }
  virtual void Write (float* what)
  {
    memcpy (out, what, No * sizeof (float));
    out += No;
  }
};

struct ClipBuffer
{
  uint8* source;
  size_t sourceComp;
  size_t sourceStride;
  float* dest;
  size_t destComp;
};

const int ClipMaxBuffers = 16;
typedef uint ClipBuffersMask;

template<class Meat>
class BuffersClipper
{
  VertexOutputBase vout[ClipMaxBuffers];
  
  template<int Ni, int No>
  void SetupVOut2 (int i, const ClipBuffer& clipBuf)
  {
    new (&vout[i]) VertexOutput<Ni, No> (clipBuf.source, clipBuf.sourceStride,
      clipBuf.dest);
  }
  template<int Ni>
  void SetupVOut1 (int i, const ClipBuffer& clipBuf)
  {
    switch (clipBuf.destComp)
    {
      case 1:
        SetupVOut2<Ni, 1> (i, clipBuf);
        break;
      case 2:
        SetupVOut2<Ni, 2> (i, clipBuf);
        break;
      case 3:
        SetupVOut2<Ni, 3> (i, clipBuf);
        break;
      case 4:
        SetupVOut2<Ni, 4> (i, clipBuf);
        break;
      default:
        CS_ASSERT_MSG("Unsupported component count", false);
    }
  }
  void SetupVOut (int i, const ClipBuffer& clipBuf)
  {
    switch (clipBuf.sourceComp)
    {
      case 1:
        SetupVOut1<1> (i, clipBuf);
        break;
      case 2:
        SetupVOut1<2> (i, clipBuf);
        break;
      case 3:
        SetupVOut1<3> (i, clipBuf);
        break;
      case 4:
        SetupVOut1<4> (i, clipBuf);
        break;
      default:
        CS_ASSERT_MSG("Unsupported component count", false);
    }
  }
public:
  size_t DoClip (Meat& meat, const csTriangle& tri, const ClipBuffer* buffers, 
    ClipBuffersMask buffersMask)
  {
    for (int i = 0; i < ClipMaxBuffers; i++)
    {
      if (!(buffersMask & (1 << i))) continue;
      SetupVOut (i, buffers[i]);
    }
    return meat.DoClip (tri, buffers, buffersMask, vout);
  }
};

#define VATTR_CLIPINDEX(x)                                               \
  (CS_VATTRIB_ ## x - (CS_VATTRIB_ ## x >=  CS_VATTRIB_GENERIC_FIRST ?        \
  CS_VATTRIB_GENERIC_FIRST : CS_VATTRIB_SPECIFIC_FIRST))

#endif // __CS_SOFT3D_CLIPPER_H__
