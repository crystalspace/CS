/*
    Dynamics/Kinematics modeling and simulation library.
    Copyright (C) 1999 by Michael Alexander Ewert

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

#ifndef CT_MATH_UTIL_H
#define CT_MATH_UTIL_H

#include "csphyzik/phyztype.h"
#include "csphyzik/ctvector.h"
#include "csphyzik/ctmatrix.h"
#include <stdarg.h>

#ifndef PI
#define PI 3.1412L
#endif
#define degree_to_rad( A ) ( 2.0*A*PI/360.0 )

/// calc rotation matrix from a rotation of an angle around a vector
void R_from_vector_and_angle ( ctVector3 pvec, real pangle, ctMatrix3 &pR );

/// return angle between two vectors
real angle_diff ( ctVector3 v1, ctVector3 v2 );

/// takes real's as args
/// return max
real mu_max ( int pnum, ... );
real sign ( real pval );

#endif
