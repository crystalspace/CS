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

CS_PLUGIN_NAMESPACE_BEGIN(Soft3D)
{

//#define VOUT_DEBUG

class VertexOutputPersp
{
protected:
  csVector3* out;
public:
  const csVector3* in;
  csVector3* startOut;

  void Reset() { out = startOut; }
  inline void SanityCheck()
  {
#ifdef VOUT_DEBUG
    CS_ASSERT_MSG("Reset() not called", out != 0);
#endif
  }
  void Lerp (csVector3*& dst, size_t idx1, size_t idx2, float p) 
  {
    const csVector3* I1 = in + idx1;
    const csVector3* I2 = in + idx2;
    for (int i = 0; i < 3; i++)
    {
      const float f1 = (*I1)[i];
      const float f2 = (*I2)[i];
      (*dst)[i] = f1 + p * (f2-f1);
    }
    dst++;
  }
  void Lerp3 (csVector3*& dst, size_t idx1, size_t idx2, float p1,
	      size_t idx3, size_t idx4, float p2,
	      float p)
  {
    csVector3 v1;
    csVector3 v2;
    csVector3* d1 = &v1;
    csVector3* d2 = &v2;
    Lerp (d1, idx1, idx2, p1);
    Lerp (d2, idx3, idx4, p2);
    for (int i = 0; i < 3; i++)
    {
      const float f1 = v1[i];
      const float f2 = v2[i];
      (*dst)[i] = f1 + p * (f2-f1);
    }
    dst++;
  }
public:
  VertexOutputPersp () : 
#ifdef CS_DEBUG
    out(0)
#endif
    {}

  void Copy (size_t idx)
  {
    SanityCheck();
    *out++ = in[idx];
  }
  void LerpTo (csVector3* dst, size_t idx1, size_t idx2, float p) 
  {
    SanityCheck();
    Lerp (dst, idx1, idx2, p);
  }
  void Lerp (size_t idx1, size_t idx2, float p) 
  {
    SanityCheck();
    Lerp (out, idx1, idx2, p);
  }
  void Lerp3 (size_t idx1, size_t idx2, float p1,
              size_t idx3, size_t idx4, float p2,
	      float p)
  {
    SanityCheck();
    Lerp3 (out, idx1, idx2, p1, idx3, idx4, p2, p);
  }
  void Lerp3To (csVector3* dst, size_t idx1, size_t idx2, float p1,
		size_t idx3, size_t idx4, float p2,
		float p)
  {
    SanityCheck();
    Lerp3 (dst, idx1, idx2, p1, idx3, idx4, p2, p);
  }
  void Write (const csVector3& what)
  {
    SanityCheck();
    *out++ = what;
  }
};

template<class Meat>
class BuffersClipper
{
  VertexOutputPersp voutPersp;
  const csVector3* inPersp;
  Meat& meat;
  const VerticesLTN* inBuffers;
  VerticesLTN* outBuffers;
  
public:
  BuffersClipper (Meat& meat) :  meat(meat) { }
  void Init (const csVector3* inPersp, csVector3* outPersp,
    const VerticesLTN* inBuffers, VerticesLTN* outBuffers)
  {
    this->inPersp = inPersp;
    this->inBuffers = inBuffers;
    this->outBuffers = outBuffers;

    voutPersp.in = inPersp;
    voutPersp.startOut = outPersp;
  }

  size_t DoClip (const csTriangle& tri)
  {
    voutPersp.Reset();
    return meat.DoClip (tri, inPersp, voutPersp, inBuffers, outBuffers);
  }
};

}
CS_PLUGIN_NAMESPACE_END(Soft3D)

#endif // __CS_SOFT3D_CLIPPER_H__
