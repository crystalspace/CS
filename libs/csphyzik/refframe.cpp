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
#include "csphyzik/refframe.h"

ctReferenceFrame::ctReferenceFrame 
  ( coord px, coord py, coord pz, ctangle ppitch, ctangle proll, ctangle pyaw, 
    ctReferenceFrame *ref ) 
  : offset ( px, py, pz )
{
  //!me what the hell is this.  Who writes this stuff?
  (void) ref;
  (void) pyaw;
  (void) proll;
  (void) ppitch;
  (void) px;
  (void) py;
  (void) pz;
  reference_count = 0;
  is_universe_frame = false;
//	parent_frame = ref ? ref : &universe();
}

ctReferenceFrame& ctReferenceFrame::universe ()
{ 
  static ctReferenceFrame basis;
  static bool initialized = false;
  if (!initialized) 
  {
    initialized = true;
    basis.is_universe_frame = true; 
  }
  return basis;
} 

ctDeltaReferenceFrame::ctDeltaReferenceFrame () 
  : v(0), w(0)
{
	reference_count = 0; is_universe_frame = false;
}

ctDeltaReferenceFrame& ctDeltaReferenceFrame::universe ()
{
  static ctDeltaReferenceFrame basis;
  static bool initialized = false;
  if (!initialized) 
  {
    initialized = true;
    basis.is_universe_frame = true;
  }
  return basis;
} 

// calculate tranform from world coords to this coords by going from
// root ( world ) and combining all transfroms up to this frame.
//void ctReferenceFrame::calc_T_world()
//{
//	is_T_world_calced = true;

/*
	// if this transform is valid, don't bother re-calcing it
	if( !is_T_world_calced ){  
		// this is world coords, don't need to calc world-to-world
		if( !this->is_universe() ){
			if( parent_frame ){ // not NULL
				// don't calc world coords ( I matrix ) with this.
				if( parent_frame->is_universe() ){  
					T_world = T_parent;
				}else{
					// recurse 'till root ( world/universe coords )
					parent_frame->calc_T_world();
					// combine tranforms to get world to this transform
					// (parent to this)*(world to parent)*v
					T_world = (T_parent)*(parent_frame->T_world);
				}
			}
		}		
		is_T_world_calced = true;
	}*/
//}

/*
inline void ctReferenceFrame::set_state( real **state_array )
{
ctVector3 = o;
ctMatrix3 = M;

	o = get_org_world();
	M = get_R();

	**state_array = o.x;
	*state_array++;
	**state_array = o.y;
	*state_array++;
	**state_array = o.z;
	*state_array++;

	for( int i = 0; i < 3; i++ ){
		for( int j = 0; j < 3; j++ ){
			**state_array = M[i][j];
			*state_array++;
		}
	}
}

inline void ctReferenceFrame::get_state( real **state_array )
{
ctVector3 = o;
ctMatrix3 = M;

	o = get_org_world();
	M = get_R();

	o.x = **state_array;
	*state_array++;
	o.y = **state_array;
	*state_array++;
	o.z = **state_array;
	*state_array++;
	
	for( int i = 0; i < 3; i++ ){
		for( int j = 0; j < 3; j++ ){
			M[i][j] = **state_array;
			*state_array++;
		}
	}
}
*/
