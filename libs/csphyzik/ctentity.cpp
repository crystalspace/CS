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

#include "cssysdef.h"
#include "csphyzik/entity.h"
#include "csphyzik/solver.h"
#include "csphyzik/phyzent.h"
#include "csphyzik/debug.h"

ctEntity::ctEntity ()
{
  state_offset = -1;
  solver = NULL;
  flags = 0;
}

ctEntity::~ctEntity ()
{
  if ( solver )
    delete solver;
}

// pass control to solver
void ctEntity::solve ( real t )
{
  if ( solver )
    solver->solve ( t );
}

void ctEntity::apply_given_F ( ctForce& /*frc*/ )
{
  // notin
}
