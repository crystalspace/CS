/*
    Copyright (C) 1998,1999,2000 by Jorrit Tyberghein
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
#include <float.h>
#include "csqint.h"
#include "csqsqrt.h"
#include "csgeom/vector4.h"

//---------------------------------------------------------------------------

csString csVector4::Description() const
{
  csString s;
  s.Format("%g,%g,%g,%g", x, y, z, w);
  return s;
}

float csVector4::Norm () const
{
  return csQsqrt (x * x + y * y + z * z + w * w);
}

void csVector4::Normalize ()
{
  float sqlen = x * x + y * y + z * z + w * w;
  if (sqlen < SMALL_EPSILON) return ;

  float invlen = csQisqrt (sqlen);
  *this *= invlen;
}

csVector4::csVector4 (const csDVector4 &v)
{
  x = (float)v.x;
  y = (float)v.y;
  z = (float)v.z;
  w = (float)v.w;
}

double csDVector4::Norm () const
{
  return sqrt (x * x + y * y + z * z + w * w);
}

double csDVector4::SquaredNorm () const
{
  return x * x + y * y + z * z + w * w;
}

void csDVector4::Normalize ()
{
  double len;
  len = this->Norm ();
  if (len > SMALL_EPSILON) *this /= len;
}

csDVector4::csDVector4 (const csVector4 &v)
{
  x = v.x;
  y = v.y;
  z = v.z;
  w = v.w;
}

//---------------------------------------------------------------------------
