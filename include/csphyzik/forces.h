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

#ifndef CT_THEFORCES_H
#define CT_THEFORCES_H

#include "csphyzik/force.h"

class ctDynamicEntity;

// magnitude = g.  g = 9.81 for earth gravity
class ctGravityF : public ctForce
{
public:
	
	ctGravityF( real pg = 9.81*M_PER_WORLDUNIT, ctVector3 pd = ctVector3( 0.0, -1.0, 0.0 ) );
	ctGravityF( ctReferenceFrame &rf, real pg = 9.81*M_PER_WORLDUNIT, ctVector3 pd = ctVector3( 0.0, -1.0, 0.0 ) );
	virtual ctVector3 apply_F( ctDynamicEntity &pe );

};

// f = -kv
//!me really simplified right now..... should take surface area into account
//!me would be cool to simulate falling paper and leaves
class ctAirResistanceF : public ctForce
{
public:
	
	ctAirResistanceF( real pk = DEFAULT_AIR_RESISTANCE );
	virtual ctVector3 apply_F( ctDynamicEntity &pe );

};

// just an applied torque
class ctTorqueF : public ctForce
{
public:
	ctTorqueF( ctVector3 dir, real pm );
	virtual ctVector3 apply_F( ctDynamicEntity &pe );

};

class ctAppliedF : public ctForce
{
public:
	ctAppliedF( ctVector3 dir, real pm );
	virtual ctVector3 apply_F( ctDynamicEntity &pe );


};

// spring attached to two bodies or inertial reference frame.
class ctSpringF : public ctNBodyForce
{
public:
	ctSpringF( ctPhysicalEntity *b1, ctVector3 p1, ctPhysicalEntity *b2, ctVector3 p2 )
	{
		body_vector.add_link( b1 );
		attachment_point_vector.add_link( new ctVector3(p1) );
		body_vector.add_link( b2 );
		attachment_point_vector.add_link( new ctVector3(p2) );
		rest_length = 1;
	}
	
	~ctSpringF(){ 
		attachment_point_vector.delete_link( attachment_point_vector.get_first() );
		attachment_point_vector.delete_link( attachment_point_vector.get_first() );
	}

	void set_rest_length( real len ){ rest_length = len; }

	virtual ctVector3 apply_F( ctDynamicEntity &pe );

	//!me expand to include 3+ bodies on one spring
	virtual void add_body( ctPhysicalEntity *bod ){ if( body_vector.get_size() < 2 ) body_vector.add_link( bod ); }

protected:
	ctLinkList_ctVector3 attachment_point_vector;
	real rest_length;
	
};


// 1/r^2 force. ( G * M1 * m2 ) / ( r^2 ) 
// a gravity well.  gravitational attraction between bodies.
// add bodies to this force that will exert force on bodies that have this force
// included in their force list.
//!me right now an object passing too close to the discontinuity will get accelerated
// way to fast and energy will NOT be conserved.  This could be fixed using R-K 
// method with adaptive step-sizing or some happy horse-shit like that.
class ctGravityWell : public ctNBodyForce
{
public:
	ctGravityWell( ctPhysicalEntity *BIG_mass )
	{
		magnitude = PHYZ_CONSTANT_G; 
		body_vector.add_link( BIG_mass );
	}

	//!me clean up
	virtual ~ctGravityWell(){ 
	}

	virtual ctVector3 apply_F( ctDynamicEntity &pe );

protected:
	
};

#endif
