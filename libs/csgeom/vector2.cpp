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
#include "csqsqrt.h"
#include "csgeom/vector2.h"
#include "csutil/csstring.h"

csString csVector2::Description() const
{
  csString s;
  s.Format("%g,%g", x, y);
  return s;
}

float csVector2::Norm (const csVector2 &v)
{
  return csQsqrt (v * v);
}

float csVector2::Norm () const
{
  return csQsqrt (x * x + y * y);
}

void csVector2::Rotate (float angle)
{
  float s = (float)sin (angle);
  float c = (float)cos (angle);
  float nx = x * c + y * s;
  y = -x * s + y * c;
  x = nx;
}

csVector2 operator+ (const csVector2 &v1, const csVector2 &v2)
{
  return csVector2 (v1.x + v2.x, v1.y + v2.y);
}

csVector2 operator- (const csVector2 &v1, const csVector2 &v2)
{
  return csVector2 (v1.x - v2.x, v1.y - v2.y);
}

float operator * (const csVector2 &v1, const csVector2 &v2)
{
  return v1.x * v2.x + v1.y * v2.y;
}

csVector2 operator * (const csVector2 &v, float f)
{
  return csVector2 (v.x * f, v.y * f);
}

csVector2 operator * (float f, const csVector2 &v)
{
  return csVector2 (v.x * f, v.y * f);
}

csVector2 operator/ (const csVector2 &v, float f)
{
  f = 1.0f / f;
  return csVector2 (v.x * f, v.y * f);
}

bool operator== (const csVector2 &v1, const csVector2 &v2)
{
  return (v1.x == v2.x) && (v1.y == v2.y);
}

bool operator!= (const csVector2 &v1, const csVector2 &v2)
{
  return (v1.x != v2.x) || (v1.y != v2.y);
}
