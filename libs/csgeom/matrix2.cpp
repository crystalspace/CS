/*
    Copyright (C) 1998,2000 by Jorrit Tyberghein
    Largely rewritten by Ivan Avramovic <ivan@avramovic.com>

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
#include <math.h>
#include "csgeom/matrix2.h"

//---------------------------------------------------------------------------
csMatrix2::csMatrix2 ()
{
  m11 = m22 = 1;
  m12 = m21 = 0;
}

csMatrix2::csMatrix2 (float m11, float m12, float m21, float m22)
{
  csMatrix2::m11 = m11;
  csMatrix2::m12 = m12;
  csMatrix2::m21 = m21;
  csMatrix2::m22 = m22;
}

csMatrix2 &csMatrix2::operator+= (const csMatrix2 &m)
{
  m11 += m.m11;
  m12 += m.m12;
  m21 += m.m21;
  m22 += m.m22;
  return *this;
}

csMatrix2 &csMatrix2::operator-= (const csMatrix2 &m)
{
  m11 -= m.m11;
  m12 -= m.m12;
  m21 -= m.m21;
  m22 -= m.m22;
  return *this;
}

csMatrix2 &csMatrix2::operator*= (const csMatrix2 &m)
{
  csMatrix2 r (*this);
  m11 = r.m11 * m.m11 + r.m12 * m.m21;
  m12 = r.m11 * m.m12 + r.m12 * m.m22;
  m21 = r.m21 * m.m11 + r.m22 * m.m21;
  m22 = r.m21 * m.m12 + r.m22 * m.m22;
  return *this;
}

csMatrix2 &csMatrix2::operator*= (float s)
{
  m11 *= s;
  m12 *= s;
  m21 *= s;
  m22 *= s;
  return *this;
}

csMatrix2 &csMatrix2::operator/= (float s)
{
  s=1.0f/s;
  m11 *= s;
  m12 *= s;
  m21 *= s;
  m22 *= s;
  return *this;
}

void csMatrix2::Identity ()
{
  m11 = m22 = 1;
  m12 = m21 = 0;
}

void csMatrix2::Transpose ()
{
  float swap = m12;
  m12 = m21;
  m21 = swap;
}

csMatrix2 csMatrix2::GetTranspose () const
{
  return csMatrix2 (m11, m21, m12, m22);
}

float csMatrix2::Determinant () const
{
  return m11 * m22 - m12 * m21;
}

csMatrix2 operator+ (const csMatrix2 &m1, const csMatrix2 &m2)
{
  return csMatrix2 (
      m1.m11 + m2.m11,
      m1.m12 + m2.m12,
      m1.m21 + m2.m21,
      m1.m22 + m2.m22);
}

csMatrix2 operator- (const csMatrix2 &m1, const csMatrix2 &m2)
{
  return csMatrix2 (
      m1.m11 - m2.m11,
      m1.m12 - m2.m12,
      m1.m21 - m2.m21,
      m1.m22 - m2.m22);
}

csMatrix2 operator * (const csMatrix2 &m1, const csMatrix2 &m2)
{
  return csMatrix2 (
      m1.m11 * m2.m11 + m1.m12 * m2.m21,
      m1.m11 * m2.m12 + m1.m12 * m2.m22,
      m1.m21 * m2.m11 + m1.m22 * m2.m21,
      m1.m21 * m2.m12 + m1.m22 * m2.m22);
}

csMatrix2 operator * (const csMatrix2 &m, float f)
{
  return csMatrix2 (m.m11 * f, m.m12 * f, m.m21 * f, m.m22 * f);
}

csMatrix2 operator * (float f, const csMatrix2 &m)
{
  return csMatrix2 (m.m11 * f, m.m12 * f, m.m21 * f, m.m22 * f);
}

csMatrix2 operator/ (const csMatrix2 &m, float f)
{
  float inv_f = 1 / f;
  return csMatrix2 (
      m.m11 * inv_f,
      m.m12 * inv_f,
      m.m21 * inv_f,
      m.m22 * inv_f);
}

//---------------------------------------------------------------------------
