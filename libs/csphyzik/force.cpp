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
#include "csphyzik/force.h"
#include "csphyzik/phyzent.h"
#include "csphyzik/bodyforc.h"
#include "csphyzik/refframe.h"

ctForce::ctForce() :
  direction(0), origin(0), RF( ctReferenceFrame::universe() )
{
  RF.add_ref( RF );
  magnitude = 1.0;
}

ctForce::ctForce ( ctReferenceFrame &ref ) :
  direction(0), origin(0), RF( ref )
{
  RF.add_ref( RF );
}

ctForce::~ctForce ()
{
  RF.remove_ref( RF );
}


ctNBodyForce::ctNBodyForce ()
{

}

ctNBodyForce::ctNBodyForce ( ctReferenceFrame &ref ) 
  : ctForce( ref )
{
}

ctNBodyForce::~ctNBodyForce ()
{
  ctPhysicalEntity *pe;
	
  pe = body_vector.get_first();
  while ( pe )
  {
    body_vector.delete_link( pe );
    pe = body_vector.get_next();
  }

/*	for( int i = 0; i < body_vector.get_size(); i++ ){
		pe = body_vector[i];
		if( pe ){
			pe->remove_force( this );
		}
	}*/
}
