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

#ifndef ctPointObj_H
#define ctPointObj_H

#include "ctphyzik/ctvector.h"
#include "ctphyzik/phyztype.h"

// A ctPointObj is the parent class of anything which acts like a point,
// including point-masses and points (connectors) on larger objects.  Its
// basic function is to retrieve its position and velocity, and allow
// point forces and impulses to be applied.
//
class ctPointObj {
 public:
  virtual ctVector3 pos();
  virtual ctVector3 vel();  // The derivative of pos() wrt time

  virtual void apply_force(ctVector3 force);
};

#endif
