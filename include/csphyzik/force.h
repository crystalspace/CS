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

#ifndef FORCE_H
#define FORCE_H

#include "csphyzik/phyztype.h"
#include "csphyzik/math3d.h"
#include "csphyzik/linklist.h"

class ctReferenceFrame;
class ctDynamicEntity;
class ctPhysicalEntity;


// parent class for all forces
class ctForce
{
public:

	// defaults with inertial reference frame 
	ctForce();
	// use supplied referece frame
	ctForce( ctReferenceFrame &rf );
	virtual ~ctForce();

	// apply this force to the given body.  Adds F and Torque components to pe
	// overload this and add new functionality for new forces
	virtual ctVector3 apply_F( ctDynamicEntity& /*pe*/ ){ return *( new ctVector3() );}

	// generic magnitude and direction to be used by all forces
	void set_magnitude( real m ){ magnitude = m; }
	void set_direction( ctVector3 d ){ direction = d; }

	real magnitude;
	ctVector3 direction;
	ctVector3 origin;
	ctReferenceFrame &RF;
	
};


// parent class for all N-body forces 
class ctNBodyForce : public ctForce
{
public:
	ctNBodyForce();
	ctNBodyForce( ctReferenceFrame &rf );
	virtual ~ctNBodyForce();

	// add another body that this force works on
	virtual void add_body( ctPhysicalEntity *bod ){ body_vector.add_link( bod ); }

	ctLinkList_ctPhysicalEntity body_vector;  

};

#endif
