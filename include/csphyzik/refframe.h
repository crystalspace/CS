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

#ifndef ctReferenceFrame_H
#define ctReferenceFrame_H

#include <stdio.h>
#include "csphyzik/ctvector.h"
#include "csphyzik/ctmatrix.h"
#include "csphyzik/math3d.h"

//!me add support for nested frames of reference.
//!me This class should take care of it's ODE state interface... maybe...

// frame of reference.  position and orientation data are stored here.
// uses refrence counting scheme.
class ctReferenceFrame
{
public:
  ctReferenceFrame ( coord px = 0, coord py = 0, coord pz = 0,
		     ctangle ppitch = 0, ctangle proll = 0, ctangle pyaw = 0,
		     ctReferenceFrame *ref = NULL );

  ~ctReferenceFrame() {}

  static ctReferenceFrame& universe ();

  bool is_universe ()
  { return is_universe_frame; }

  bool not_universe ()
  { return !is_universe_frame; }

  static void add_ref ( ctReferenceFrame &rf )
  { rf.reference_count++; }

  static void remove_ref ( ctReferenceFrame &rf )
  {
    if ( --rf.reference_count <= 0 )
      if ( rf.not_universe() )
	delete &rf;
  }

  const ctVector3 &get_offset()
  { return offset; }

  const ctVector3 &get_world_offset()
  { return offset; }

  void set_offset( const ctVector3 &v )
  { offset = v; }

  void set_world_offset( ctVector3 &v )
  { offset = v; }

  /**
   * get/set rotation matrix ( transform from child to parent frame )
   * or rotation matrix in parent coords
   */
  const ctMatrix3 &get_R ()
  { return fTg; }

  const ctMatrix3 &get_this_to_parent ()
  { return fTg; }

  void set_R ( const ctMatrix3 &M )
  { fTg = M;  gTf = fTg.get_transpose(); }

  void set_this_to_parent ( const ctMatrix3 &M )
  { fTg = M; gTf = fTg.get_transpose(); }

  /// get/set tranform matrix ( transfrom from parent to child frame )
  const ctMatrix3 &get_T()
  { return gTf; }

  const ctMatrix3 &get_parent_to_this ()
  { return gTf; }

  void set_T ( const ctMatrix3 &M )
  { gTf = M; fTg = gTf.get_transpose(); }

  void set_parent_to_this ( const ctMatrix3 &M )
  { gTf = M; fTg = gTf.get_transpose(); }

  //!me no hiearchy yet really
  const ctMatrix3 &get_this_to_world()
  { return fTg; }

  void this_to_world ( ctVector3 &v )
  {
    v = get_this_to_world()*v + get_world_offset();
//		if( !T_world_calced() )
//			calc_T_world();
//		return T_world.This2Other( v );
  }

  void this_to_world ( ctVector3 &v, ctVector3 &src )
  {
    v = get_this_to_world()*src + get_world_offset();
//		if( !T_world_calced() )
//			calc_T_world();
//		return T_world.This2Other( v );
  }

protected:
  /// transform from parent frame to this one
  ctMatrix3 gTf;
  /// transfrom from this frame to parent frame
  ctMatrix3 fTg;
  /// offset from parent frame to this one in parent's frame
  ctVector3 offset;

  int reference_count;

  /// true if this is the top-most reference frame
  bool is_universe_frame;
//	bool is_T_world_calced; // true if T_world is currently valid
//	ctReferenceFrame *parent_frame;
};

class ctDeltaReferenceFrame
{
public:

  static ctDeltaReferenceFrame& universe ();

  bool is_universe ()
  { return is_universe_frame; }

  bool not_universe ()
  { return !is_universe_frame; }

  static void add_ref ( ctDeltaReferenceFrame &rf )
  {  rf.reference_count++; }

  static void remove_ref ( ctDeltaReferenceFrame &rf )
   {
     if ( --rf.reference_count <= 0 )
       if ( rf.not_universe() )
	 delete &rf;
   }

  ctDeltaReferenceFrame ();
  ctVector3 v;
  ctVector3 w;

protected:
  int reference_count;
  bool is_universe_frame; // true if this is the top-most reference frame
};

#endif
