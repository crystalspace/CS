/*
    Dynamics/Kinematics modeling and simulation library.
    Copyright (C) 1999 by Michael Alexander Ewert and Noah Gibbs

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
#include "csphyzik/ctvector.h"
#include "csphyzik/ctmatrix.h"
#include "csphyzik/ctquat.h"
#include "csgeom/quaterni.h"
#include "qsqrt.h"

ctMatrix3 ctQuaternion::to_matrix ()
{
  ctMatrix3 M;

  M[0][0] = 1.0 - 2.0 * y * y - 2.0 * z * z;
  M[0][1] = 2.0 * x * y - 2.0 * r * z;
  M[0][2] = 2.0 * x * z + 2.0 * r * y;

  M[1][0] = 2.0 * x * y + 2.0 * r * z;
  M[1][1] = 1.0 - 2.0 * x * x - 2.0 * z * z;
  M[1][2] = 2.0 * y * z - 2.0 * r * x;

  M[2][0] = 2.0 * x * z - 2.0 * r * y;
  M[2][1] = 2.0 * y * z + 2.0 * r * x;
  M[2][2] = 1.0 - 2.0 * x * x - 2.0 * y * y;

  return M;
}

void ctQuaternion::from_matrix(ctMatrix3& M)
{
  real         tr, s;

  tr = M[0][0] + M[1][1] + M[2][2];
  if (tr >= 0.0)
  {
    s = qsqrt(tr + 1.0);
    r = 0.5 * s;
    x = (M[2][1] - M[1][2]) * s;
    y = (M[0][2] - M[2][0]) * s;
    z = (M[1][0] - M[0][1]) * s;
  }
  else
  {
    int i = 0;

    if (M[1][1] > M[0][0]) i = 1;
    if (M[2][2] > M[i][i]) i = 2;

    switch (i)
    {
      case 0:
	s = qsqrt((M[0][0] - (M[1][1] + M[2][2])) + 1.0);
	x = 0.5 * s;
	s = 0.5 / s;
	y = (M[0][1] + M[1][0]) * s;
	z = (M[2][0] + M[0][2]) * s;
	r = (M[2][1] - M[1][2]) * s;
	break;
      case 1:
	s = qsqrt((M[1][1] - (M[0][0] + M[2][2])) + 1.0);
	y = 0.5 * s;
	s = 0.5 / s;
	z = (M[1][2] + M[2][1]) * s;
	x = (M[0][1] + M[1][0]) * s;
	r = (M[0][2] - M[2][0]) * s;
	break;
      case 2:
	s = qsqrt((M[2][2] - (M[1][1] + M[0][0])) + 1.0);
	z = 0.5 * s;
	s = 0.5 / s;
	x = (M[2][0] + M[0][2]) * s;
	y = (M[1][2] + M[2][1]) * s;
	r = (M[1][0] - M[0][1]) * s;
	break;
    }
  }
}
