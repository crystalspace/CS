/*
    Copyright (C) 1998 by Jorrit Tyberghein
  
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

#ifndef PHYSICS_H
#define PHYSICS_H

#include "csgeom/math3d.h"

class csSector;
class Polygon3D;
class Vertex;

/**
 * Interface from which a class should inherit (using multiple
 * inheritance if needed) so that it can be used together with
 * the Physics class.
 */
class PhysicalObjectInt
{
public:
  /**
   * Move the object a relative amount. Return true if
   * succesful. Return false if we hit something (even then
   * the object should move as far as possible).
   */
  virtual bool move_relative (csVector3& v) = 0;

  /**
   * Check if the object can move a relative amount
   * without hitting something. Return false if there
   * is a hit.
   */
  virtual bool can_move_relative (csVector3& v) = 0;

  /**
   * Test if we are on the ground.
   */
  virtual bool on_ground (void) = 0;

  /**
   * Transform a force vector in the direction that the object
   * is facing.
   */
  virtual void transform_force (csVector3& force, csVector3& relativeForce) = 0;
};

/**
 * The physics class. This class operates on a PhysicalObjectInt.
 * For every PhysicalObjectInt there is a corresponding Physics
 * class.
 */
class Physics
{
private:
  csVector3 linearForces;
  csVector3 maxLinearForces;
  csVector3 linearDampening;
  csVector3 linearSpeed;
  csVector3 rotationalForces;
  float   gravity;
  float	  moveSpeed;
  time_t  lastTime;
  time_t  lastFootStep;

  /// The object for which this physics class works.
  PhysicalObjectInt* object;

public:
  ///
  Physics (PhysicalObjectInt* obj);
  ///
  virtual ~Physics ();

  ///
  void applyForce (float x, float y, float z);
  ///
  void applyRotation (float x, float y, float z);
  ///
  void spendTime (void);
  ///
  void limitForces (void);
  ///
  void useFoot (float x, float y, float z);
  ///
  bool onGround (void);
};

#endif /*PHYSICS_H*/
