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

#include "sysdef.h"
#include "csengine/sysitf.h"
#include "csengine/physics.h"
#include "csengine/world.h"

//---------------------------------------------------------------------------

Physics::Physics (PhysicalObjectInt* obj) :
	linearForces (0,0,0),
	maxLinearForces (2.0, 2.0, 2.0),
	linearDampening (2.0, 0.1, 2.0), // vertical dampening is less for gravitation
	linearSpeed (0.0, 0.0, 0.0),
	rotationalForces (0,0,0),
	gravity (6.0),
	moveSpeed (6.0)
{
  csWorld::isys->GetTime (lastTime);
  lastFootStep = lastTime; // sji - init the timer
  object = obj;
}

Physics::~Physics ()
{
}

/*--------------------------------------------------------------------
	Do the physics calcs
  --------------------------------------------------------------------*/
void Physics::spendTime(void)
{
  // the forces are per second forces so get the fraction of a second to apply
  time_t tm;
  csWorld::isys->GetTime (tm);
  float           timeSlice = float(tm - lastTime) / 1000.0; 

  // our frame rate should be limited to less than CLOCKS_PER_SEC
  // I would reccommend CLOCKS_PER_SEC / 2, or 30fps
  if (timeSlice == 0.0) // not enough time has gone by, things will now screw up
    timeSlice = 1.0 / 60.0;

  csWorld::isys->GetTime (lastTime);

  // calc our new speed based on all forces
  linearSpeed += linearForces;
  linearForces -= linearForces;   // forces are used up now
        
  // calc our new position based on our speeds
  csVector3         new_position;
  new_position.x = timeSlice * linearSpeed.x;
  new_position.y = 0; // ### do y last as a temp hack for gravity 
//timeSlice * linearSpeed.y;
  new_position.z = timeSlice * linearSpeed.z;
  //m_cam2world.transform(pos, new_position); // sji movement is not relative to the camera
  if (!object->move_relative (new_position))
  {
    // determine what we hit and slide off it
  }
  // do vertical velocity last
  // a hack to make gravity work until we can calc sliding off of surfaces
  new_position.x = 0;
  new_position.y = timeSlice * linearSpeed.y;
  new_position.y -= 1.0; // hack: our point of view is really above the ground
  new_position.z = 0;
  if (!object->can_move_relative (new_position))
    linearSpeed.y = 0; // for now, we may want to bounce
  else
  {
    new_position.y += 1.0; // hack: our point of view is really above the ground
    object->move_relative (new_position);
  }
                        
  // gravity - this force is always there
  linearForces.y -= gravity * timeSlice;
        
  // dampen our velocity
  // this must be done to by shortening the length of a 3d vector
  // the dampening would differ depending on what you are in
  // for air it would be light, water heavy, and on the ground it would be very heavy
  // ### for now I will use this, but this needs more 3d math
  linearSpeed.x *= (1.0 - linearDampening.x * timeSlice);
  linearSpeed.y *= (1.0 - linearDampening.y * timeSlice);
  linearSpeed.z *= (1.0 - linearDampening.z * timeSlice);
}

/*--------------------------------------------------------------------
        Limit the forces
--------------------------------------------------------------------*/
void Physics::limitForces(void)
{
  if (linearForces.x > maxLinearForces.x)
    linearForces.x = maxLinearForces.x;
  if (linearForces.x < -maxLinearForces.x)
    linearForces.x = -maxLinearForces.x;
  if (linearForces.y > maxLinearForces.y)
    linearForces.y = maxLinearForces.y;
  if (linearForces.y < -maxLinearForces.y)
    linearForces.y = -maxLinearForces.y;
  if (linearForces.z > maxLinearForces.z)
    linearForces.z = maxLinearForces.z;
  if (linearForces.z < -maxLinearForces.z)
    linearForces.z = -maxLinearForces.z;
}

/*--------------------------------------------------------------------
        Apply a force using our foot, which MUST be on the ground AND
        can only operate at a certain rate, like 2 steps / second
--------------------------------------------------------------------*/
void Physics::useFoot(float x, float y, float z)
{
  time_t tm;
  csWorld::isys->GetTime (tm);
  float           timeSlice = float(tm - lastFootStep) / 1000.0;
        
  // we need to check if we are on the ground here and the time
  // ok, we are allowed to take a step, apply the forces
  // in time slice is 0, then we are being called multiple times
  // per frame, which is normal and allowed
  if (object->on_ground() && (timeSlice == 0 || timeSlice > 0.3))
  {
    csWorld::isys->GetTime (lastFootStep);
    csVector3 force(x, y, z);
    csVector3 relativeForce;
    object->transform_force (force, relativeForce); // forces applied are relative to the camera
    linearForces += relativeForce;
  }
}

/*--------------------------------------------------------------------
        Apply a force to the camera
--------------------------------------------------------------------*/
void Physics::applyForce(float x, float y, float z)
{
  csVector3 force(x, y, z);
  csVector3 relativeForce;
  object->transform_force (force, relativeForce); // forces applied are relative to the camera
  linearForces += relativeForce;
}

/*--------------------------------------------------------------------
        Apply a onetime rotational force to the camera along an axis.
        These are in radians per second.
--------------------------------------------------------------------*/
void Physics::applyRotation(float x, float y, float z)
{
  csVector3 force(x, y, z);
  rotationalForces += force;
}

//---------------------------------------------------------------------------
