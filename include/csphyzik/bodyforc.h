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

#ifndef __CT_BODYFORC_H__
#define __CT_BODYFORC_H__

#include "csphyzik/force.h"
#include "csphyzik/linklist.h"

class ctPhysicalEntity;

// parent class for all N-body forces 
// generally a body added as NULL indicates the world.
class ctNBodyForce : public ctForce
{
public:
  ctLinkList<ctPhysicalEntity> body_vector;

  ctNBodyForce ();
  ctNBodyForce ( ctReferenceFrame &rf );
  virtual ~ctNBodyForce ();

  // add another body that this force works on
  virtual void add_body ( ctPhysicalEntity *bod )
  { body_vector.add_link( bod ); }
};

#endif
