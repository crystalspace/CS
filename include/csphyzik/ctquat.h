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

#ifndef CTQUATERNION_H
#define CTQUATERNION_H

#include "csgeom/quaterni.h"
#include "csphyzik/ctvector.h"
#include "csphyzik/ctmatrix.h"

class ctQuaternion : public csQuaternion {
 public:
  ctQuaternion(real r, real x, real y, real z) : csQuaternion(r, x, y, z) {}
  ctQuaternion(ctVector3 x) : csQuaternion(csVector3(x[0], x[1], x[2])) {}
  ctQuaternion() {}
  ~ctQuaternion() {}

  ctMatrix3 to_matrix();
  void from_matrix(ctMatrix3& M);
};

#endif // CTQUATERNION_H

