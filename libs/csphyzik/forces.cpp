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
#include "csphyzik/forces.h"
#include "csphyzik/phyzent.h"
#include "csphyzik/refframe.h"
#include "csphyzik/debug.h"

ctGravityF::ctGravityF ( real pg, ctVector3 pd )
{
  magnitude = pg, direction = pd; 
}

ctGravityF::ctGravityF ( ctReferenceFrame &rf, real pg, ctVector3 pd ) 
  : ctForce( rf )
{
  magnitude = pg, direction = pd; 
}

ctVector3 ctGravityF::apply_F ( ctDynamicEntity &pe )
{
  // F = mg
  ctVector3 f = direction * magnitude * pe.get_m ();
  pe.sum_force ( f );
  return f;
}

ctAirResistanceF::ctAirResistanceF ( real pk )
{
  magnitude = -pk;
}

ctVector3 ctAirResistanceF::apply_F ( ctDynamicEntity &pe )
{
  ctVector3 lf = (pe.get_v ()) * magnitude;
  pe.sum_force ( lf );
//    ctVector3 af = (pe.get_angular_v ()) * magnitude/5;
//    pe.sum_torque ( af );
  return lf;

}

ctAppliedF::ctAppliedF ( ctVector3 dir, real pm )
{ 
  magnitude = pm; 
  direction = dir.Unit(); 
}

ctVector3 ctAppliedF::apply_F ( ctDynamicEntity &pe )
{ 
  pe.sum_force ( direction * magnitude ); 
  return direction*magnitude; 
}


ctTorqueF::ctTorqueF( ctVector3 dir, real pm )
{ 
  magnitude = pm; 
  direction = dir.Unit(); 
}

ctVector3 ctTorqueF::apply_F( ctDynamicEntity &pe )
{ 
  pe.sum_torque ( direction*magnitude ); 
  return direction*magnitude; 
}


ctVector3 ctSpringF::apply_F( ctDynamicEntity &pe )
{
  ctVector3 f;
  ctVector3 d;
  ctVector3 a1;
  ctVector3 a2;

  if ( body_vector.get_size() == 2 && 
       attachment_point_vector.get_size() == 2 )
  {
    ctPhysicalEntity *b1 = body_vector.get_first();
    ctPhysicalEntity *b2 = body_vector.get_next();
    
    ctVector3 *orig_a1 = attachment_point_vector.get_first();
    ctVector3 *orig_a2 = attachment_point_vector.get_next();

    if ( b1 && orig_a1 && orig_a2 )
    {
      ctReferenceFrame *r1 = b1->get_RF();
      ctReferenceFrame *r2 = ( b2 == NULL ) ? 
	&(ctReferenceFrame::universe()) : b2->get_RF();

      if ( r1 && r2 )
      {
	r1->this_to_world( a1, *orig_a1 );
	r2->this_to_world( a2, *orig_a2 );
				
	if ( &pe == b1 ) 
	  d = a2 - a1;
	else if( &pe == b2 )
	  d = a1 - a2;
	else
	{
	  log( "ctSpringF: body applied not part of coupling\n" );
	  return ctVector3(0,0,0);
	}
				
	//!me maybe don't need to normalize and find unit vector
	//!me I think that d.Norm() and later mult d by disp divides out to 1
	real disp = d.Norm() - rest_length;
	if ( d*d > MIN_REAL ) // avoid divide by zero
	  d = d.Unit();
	else
	  d = ctVector3(1, 0,0 );  

	// f = -kx
	f = d*disp*magnitude;
	
	if ( &pe == b1 )
	{ 
	  b1->sum_force(f);
	  d = a1 - r1->get_world_offset ();
	  b1->sum_torque ( d % f );
	}
	else if ( &pe == b2 && b2 != NULL )
	{
	  b2->sum_force (f);
	  d = a2 - r2->get_world_offset ();
	  b2->sum_torque ( d % f );
	}
	return f;			
      }
    }
  }
  return f;
}


//!me untested
// 1/r^2 force.  Such as gravity.
// right now an object passing too close to the discontinuity will get accelerated
// way fast and energy will NOT be conserved.  This could be fixed using R-K 
// method with adaptive step-sizing or some happy horse-shit like that.
ctVector3 ctGravityWell::apply_F ( ctDynamicEntity &moon )
{
  double g_force;
  ctVector3 r_vec;
  ctVector3 total_f ( 0.0,0.0,0.0 );
  ctVector3 planet_x;
  ctVector3 moon_x;
  double r_len2;
  ctPhysicalEntity *planet;

  planet = body_vector.get_first ();
  while ( planet != NULL )
  {
    if ( planet != &moon )
    {
      planet_x = planet->get_pos ();
      moon_x = moon.get_pos ();

      r_vec = moon_x - planet_x;

      r_len2 = r_vec * r_vec;

      r_vec.Normalize ();
      if ( r_len2 < MIN_REAL )
	r_len2 = MIN_REAL; //!me maybe have a max accel limit instead?

      g_force = ( magnitude * (( ctDynamicEntity * )planet)->get_m() ) * moon.get_m () / r_len2;
      r_vec *= g_force;
      moon.sum_force ( r_vec ); 
      total_f += r_vec;
    }
    planet = body_vector.get_next ();
  }	
  return total_f;
}
