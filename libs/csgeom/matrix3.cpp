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
#include "csgeom/matrix3.h"
#include "csgeom/quaternion.h"
#include "csutil/csstring.h"

//---------------------------------------------------------------------------
csMatrix3::csMatrix3 (float x,float y, float z, float angle)
{
  float c = cos(angle);
  float s = sin(angle);
  float t = 1.0 - c;
  m11 = c + x * x * t;
  m22 = c + y * y * t;
  m33 = c + z * z * t;

  double tmp1 = x * y * t;
  double tmp2 = z * s;
  m21 = tmp1 + tmp2;
  m12 = tmp1 - tmp2;

  tmp1 = x * z * t;
  tmp2 = y * s;
  m31 = tmp1 - tmp2;
  m13 = tmp1 + tmp2;
  tmp1 = y * z * t;
  tmp2 = x * s;
  m32 = tmp1 + tmp2;
  m23 = tmp1 - tmp2;
}

csString csMatrix3::Description () const
{
  return csString().Format ("(%s), (%s), (%s)",
    Row1().Description().GetData(), 
    Row2().Description().GetData(), 
    Row3().Description().GetData());
}

void csMatrix3::Set (const csQuaternion &quat)
{
  *this = quat.GetMatrix ();
}


//---------------------------------------------------------------------------

csXRotMatrix3::csXRotMatrix3 (float angle)
{
  m11 = 1;
  m12 = 0;
  m13 = 0;
  m21 = 0;
  m22 = (float)cos (angle);
  m23 = (float) -sin (angle);
  m31 = 0;
  m32 = -m23;
  m33 = m22;
}

csYRotMatrix3::csYRotMatrix3 (float angle)
{
  m11 = (float)cos (angle);
  m12 = 0;
  m13 = (float) -sin (angle);
  m21 = 0;
  m22 = 1;
  m23 = 0;
  m31 = -m13;
  m32 = 0;
  m33 = m11;
}

csZRotMatrix3::csZRotMatrix3 (float angle)
{
  m11 = (float)cos (angle);
  m12 = (float) -sin (angle);
  m13 = 0;
  m21 = -m12;
  m22 = m11;
  m23 = 0;
  m31 = 0;
  m32 = 0;
  m33 = 1;
}
