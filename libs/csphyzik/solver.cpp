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

#ifndef __NO_CRYSTALSPACE__
#include "cssysdef.h"
#endif
#include "csphyzik/phyzent.h"
#include "csphyzik/solver.h"

void ctSimpleDynamicsSolver::solve ( real /*t*/ )
{
  ctForce *fp;

  fp = de.forces.get_first ();
  while ( fp )
  {
    fp->apply_F ( de );
    fp = de.forces.get_next ();
  }
}
