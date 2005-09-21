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

namespace cspluginSoft3d
{

//#define VOUT_DEBUG

class VertexOutputBase
{
protected:
  uint8* in;
  float* out;
  float* startOut;
  size_t inStride;
public:
  VertexOutputBase () {}
  VertexOutputBase (uint8* in, size_t inStride, float* out) :
    in(in), 
#ifdef CS_DEBUG
    out(0),
#endif
    startOut(out), inStride(inStride) {}

  void Reset() { out = startOut; }
  virtual void Copy (size_t idx) {}
  virtual void LerpTo (float* dst, size_t idx1, size_t idx2, float p) {}
  virtual void Lerp (size_t idx1, size_t idx2, float p) {}
  virtual void Lerp3 (size_t idx1, size_t idx2, float p1,
    size_t idx3, size_t idx4, float p2,
    float p) {}
  virtual void Lerp3To (float* dst, size_t idx1, size_t idx2, float p1,
    size_t idx3, size_t idx4, float p2,
    float p) {}
  virtual void Write (float* what) {}
};

template<int Ni, int No>
class VertexOutput : public VertexOutputBase
{
  inline void SanityCheck()
  {
#ifdef VOUT_DEBUG
    CS_ASSERT_MSG("Reset() not called", out != 0);
#endif
  }
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
  virtual void Lerp3 (float*& dst, size_t idx1, size_t idx2, float p1,
		      size_t idx3, size_t idx4, float p2,
		      float p)
  {
    float v1[4];
    float v2[4];
    float* d1 = v1;
    float* d2 = v2;
    Lerp (d1, idx1, idx2, p1);
    Lerp (d2, idx3, idx4, p2);
    for (int i = 0; i < No; i++)
    {
      if (i < Ni)
      {
        const float f1 = v1[i];
        const float f2 = v2[i];
        *dst = f1 + p * (f2-f1);
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
    SanityCheck();
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
    SanityCheck();
    Lerp (dst, idx1, idx2, p);
  }
  virtual void Lerp (size_t idx1, size_t idx2, float p) 
  {
    SanityCheck();
    Lerp (out, idx1, idx2, p);
  }
  virtual void Lerp3 (size_t idx1, size_t idx2, float p1,
		      size_t idx3, size_t idx4, float p2,
		      float p)
  {
    SanityCheck();
    Lerp3 (out, idx1, idx2, p1, idx3, idx4, p2, p);
  }
  virtual void Lerp3To (float* dst, size_t idx1, size_t idx2, float p1,
			size_t idx3, size_t idx4, float p2,
			float p)
  {
    SanityCheck();
    Lerp3 (dst, idx1, idx2, p1, idx3, idx4, p2, p);
  }
  virtual void Write (float* what)
  {
    SanityCheck();
    memcpy (out, what, No * sizeof (float));
    out += No;
  }
};

// @@@ Move elsewhere?
struct VertexBuffer
{
  uint8* data;
  size_t comp;
};

const size_t clipMaxBuffers = 16;
typedef uint ClipBuffersMask;

template<class Meat>
class BuffersClipper
{
  VertexOutputBase vout[clipMaxBuffers];
  VertexOutputBase voutPersp;
  const csVector3* inPersp;
  const VertexBuffer* inBuffers;
  const size_t* inStrides;
  const VertexBuffer* outBuffers;
  Meat& meat;
  ClipBuffersMask buffersMask;
  
  template<int Ni, int No>
  void SetupVOut2 (size_t i, const VertexBuffer& inBuffer, 
    const size_t inStride, const VertexBuffer& outBuffer)
  {
    new (&vout[i]) VertexOutput<Ni, No> (inBuffer.data, inStride,
      (float*)outBuffer.data);
  }
  template<int Ni>
  void SetupVOut1 (size_t i, const VertexBuffer& inBuffer, 
    const size_t inStride, const VertexBuffer& outBuffer)
  {
    switch (inBuffer.comp)
    {
      case 1:
        SetupVOut2<Ni, 1> (i, inBuffer, inStride, outBuffer);
        break;
      case 2:
        SetupVOut2<Ni, 2> (i, inBuffer, inStride, outBuffer);
        break;
      case 3:
        SetupVOut2<Ni, 3> (i, inBuffer, inStride, outBuffer);
        break;
      case 4:
        SetupVOut2<Ni, 4> (i, inBuffer, inStride, outBuffer);
        break;
      default:
        CS_ASSERT_MSG("Unsupported component count", false);
    }
  }
  void SetupVOut (size_t i, const VertexBuffer& inBuffer, 
    const size_t inStride, const VertexBuffer& outBuffer)
  {
    switch (inBuffer.comp)
    {
      case 1:
        SetupVOut1<1> (i, inBuffer, inStride, outBuffer);
        break;
      case 2:
        SetupVOut1<2> (i, inBuffer, inStride, outBuffer);
        break;
      case 3:
        SetupVOut1<3> (i, inBuffer, inStride, outBuffer);
        break;
      case 4:
        SetupVOut1<4> (i, inBuffer, inStride, outBuffer);
        break;
      default:
        CS_ASSERT_MSG("Unsupported component count", false);
    }
  }
public:
  BuffersClipper (Meat& meat) :  meat(meat) { }
  void Init (const csVector3* inPersp, csVector3* outPersp,
    const VertexBuffer* inBuffers, const size_t* inStrides, 
    const VertexBuffer* outBuffers, ClipBuffersMask buffersMask)
  {
    this->inPersp = inPersp;
    this->inBuffers = inBuffers;
    this->inStrides = inStrides;
    this->outBuffers = outBuffers;
    this->buffersMask = buffersMask;
    for (size_t i = 0; i < clipMaxBuffers; i++)
    {
      if (!(buffersMask & (1 << i))) continue;
      SetupVOut (i, inBuffers[i], inStrides[i], outBuffers[i]);
    }
    new (&voutPersp) VertexOutput<3, 3> ((uint8*)inPersp, sizeof (csVector3),
      (float*)outPersp);
  }

  size_t DoClip (const csTriangle& tri)
  {
    for (size_t i = 0; i < clipMaxBuffers; i++)
    {
      if (!(buffersMask & (1 << i))) continue;
      vout[i].Reset();
    }
    voutPersp.Reset();
    return meat.DoClip (tri, inPersp, voutPersp, inBuffers, inStrides, 
      buffersMask, vout);
  }
};

} // namespace cspluginSoft3d

#define VATTR_BUFINDEX(x)                                               \
  (CS_VATTRIB_ ## x - (CS_VATTRIB_ ## x >=  CS_VATTRIB_GENERIC_FIRST ?        \
  CS_VATTRIB_GENERIC_FIRST : CS_VATTRIB_SPECIFIC_FIRST))

#endif // __CS_SOFT3D_CLIPPER_H__
