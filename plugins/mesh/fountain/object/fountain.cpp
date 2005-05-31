/*
    Copyright (C) 2000-2004 by Jorrit Tyberghein
    (C) W.C.A. Wijngaards, 2000

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
#include "csgeom/transfrm.h"
#include "csgeom/matrix3.h"
#include "fountain.h"
#include "ivideo/material.h"
#include "iengine/material.h"
#include "csqsqrt.h"
#include <math.h>
#include <stdlib.h>

CS_IMPLEMENT_PLUGIN

SCF_IMPLEMENT_IBASE_EXT (csFountainMeshObject)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iFountainState)
SCF_IMPLEMENT_IBASE_EXT_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csFountainMeshObject::FountainState)
  SCF_IMPLEMENTS_INTERFACE (iFountainState)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

void csFountainMeshObject::SetupObject ()
{
  if (!initialized)
  {
    csNewParticleSystem::SetupObject ();
    delete[] part_speed;
    delete[] part_age;

    part_speed = new csVector3[ParticleCount];
    part_age = new float[ParticleCount];

    time_left = 0.0;
    next_oldest = 0;
    int i;
    for (i = 0 ; i < ParticleCount ; i++)
    {
      RestartParticle (i, (fall_time / float (ParticleCount))
      	* float (ParticleCount-i));
    }
    UpdateBounds ();
  }
}

csFountainMeshObject::csFountainMeshObject (iEngine* engine,
	iMeshObjectFactory* factory)
	: csNewParticleSystem (engine, factory, CS_PARTICLE_SCALE)
{
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiFountainState);
  part_speed = 0;
  part_age = 0;
  accel.Set (0, -1, 0);
  origin.Set (0, 0, 0);
  fall_time = 1;
  speed = 1;
  opening = 1;
  azimuth = 1;
  elevation = 1;
  SetCount (50);
  SetDropSize (0.1f, 0.1f);
  rnd.Initialize ();
}

csFountainMeshObject::~csFountainMeshObject()
{
  delete[] part_speed;
  delete[] part_age;
  SCF_DESTRUCT_EMBEDDED_IBASE (scfiFountainState);
}


void csFountainMeshObject::RestartParticle (int index, float pre_move)
{
  csVector3 dest; // destination spot of particle (for speed at start)
  dest.Set(speed, 0.0f, 0.0f);
  // now make it shoot to a circle in the x direction
  float rotz_open = 2.0 * opening * (rnd.Get ()) - opening;
  csZRotMatrix3 openrot(rotz_open);
  dest = openrot * dest;
  float rot_around = rnd.GetAngle ();
  csXRotMatrix3 xaround(rot_around);
  dest = xaround * dest;
  // now dest point to somewhere in a circular cur of a sphere around the
  // x axis.

  // direct the fountain to the users dirction
  csZRotMatrix3 elev(elevation);
  dest = elev * dest;
  csYRotMatrix3 compassdir(azimuth);
  dest = compassdir * dest;

  // now dest points to the exit speed of the spout if that spout was
  // at 0,0,0.
  PositionArray[index] = origin;
  part_speed[index] = dest;

  // pre move a bit (in a perfect arc)
  part_speed[index] += accel * pre_move;
  PositionArray[index] += part_speed[index] * pre_move;
  part_age[index] = pre_move;
}


int csFountainMeshObject::FindOldest ()
{
  int ret = next_oldest;
  next_oldest = (next_oldest + 1 ) % ParticleCount;
  return ret;
}

void csFountainMeshObject::Update (csTicks elapsed_time)
{
  csNewParticleSystem::Update (elapsed_time);
  float delta_t = elapsed_time / 1000.0f; // in seconds
  // move particles;
  bool bounds_modified = false;
  int i;
  for (i=0 ; i < ParticleCount ; i++)
  {
    part_speed[i] += accel * delta_t;
    PositionArray[i] += part_speed[i] * delta_t;
    part_age[i] += delta_t;
    if (Bounds.AddBoundingVertexSmartTest (PositionArray[i]))
      bounds_modified = true;
  }

  // restart a number of particles
  float intersperse = fall_time / (float)ParticleCount;
  float todo_time = delta_t + time_left;
  while (todo_time > intersperse)
  {
    RestartParticle (FindOldest (), todo_time);
    todo_time -= intersperse;
  }
  time_left = todo_time;

  if (bounds_modified)
  {
    scfiObjectModel.ShapeChanged ();
  }
}

void csFountainMeshObject::HardTransform (const csReversibleTransform& t)
{
  origin = t.This2Other (origin);
  initialized = false;
  scfiObjectModel.ShapeChanged ();
}

//----------------------------------------------------------------------

CS_DECLARE_SIMPLE_MESH_FACTORY (csFountainFactory, csFountainMeshObject);
CS_DECLARE_SIMPLE_MESH_PLUGIN (csFountainMeshObjectType, csFountainFactory);

SCF_IMPLEMENT_FACTORY (csFountainMeshObjectType)

//----------------------------------------------------------------------

