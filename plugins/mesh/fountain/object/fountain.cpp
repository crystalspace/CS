/*
    Copyright (C) 2000 by Jorrit Tyberghein
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
#include "qsqrt.h"
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
    RemoveParticles ();
    initialized = true;
    delete[] part_pos;
    delete[] part_speed;
    delete[] part_age;

    part_pos = new csVector3[number];
    part_speed = new csVector3[number];
    part_age = new float[number];
    amt = number;

    float fradius = 10.0; // guessed radius of the fountain
    float height = 10.0; // guessed height
    bbox.Set(origin - csVector3(-fradius,0,-fradius),
      origin + csVector3(+fradius, +height, +fradius) );

    // Calculate the maximum radius.
    csVector3 size = bbox.Max () - bbox.Min ();
    float max_size = size.x;
    if (size.y > max_size) max_size = size.y;
    if (size.z > max_size) max_size = size.z;
    float a = max_size/2.;
    radius = qsqrt (a*a + a*a);

    // create particles
    for (int i=0 ; i<number ; i++)
    {
      AppendRectSprite (drop_width, drop_height, mat, lighted_particles);
      GetParticle(i)->SetMixMode(MixMode);
      RestartParticle(i, (fall_time / float(number)) * float(number-i));
      bbox.AddBoundingVertexSmart( part_pos[i] );
    }
    time_left = 0.0;
    next_oldest = 0;
    SetupColor ();
    SetupMixMode ();
  }
}

csFountainMeshObject::csFountainMeshObject (iObjectRegistry* object_reg,
  iMeshObjectFactory* factory) : csParticleSystem (object_reg, factory)
{
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiFountainState);
  part_pos = NULL;
  part_speed = NULL;
  part_age = NULL;
  accel.Set (0, -1, 0);
  origin.Set (0, 0, 0);
  fall_time = 1;
  speed = 1;
  opening = 1;
  azimuth = 1;
  elevation = 1;
  number = 50;
}

csFountainMeshObject::~csFountainMeshObject()
{
  delete[] part_pos;
  delete[] part_speed;
  delete[] part_age;
}


void csFountainMeshObject::RestartParticle (int index, float pre_move)
{
  csVector3 dest; // destination spot of particle (for speed at start)
  dest.Set(speed, 0.0f, 0.0f);
  // now make it shoot to a circle in the x direction
  float rotz_open = 2.0 * opening * (rand() / (1.0+RAND_MAX)) - opening;
  csZRotMatrix3 openrot(rotz_open);
  dest = openrot * dest;
  float rot_around = 2.0 * PI * (rand() / (1.0+RAND_MAX));
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
  part_pos[index] = origin;
  part_speed[index] = dest;

  // pre move a bit (in a perfect arc)
  part_speed[index] += accel * pre_move;
  part_pos[index] += part_speed[index] * pre_move;
  part_age[index] = pre_move;

  GetParticle(index)->SetPosition(part_pos[index]);
}


int csFountainMeshObject::FindOldest ()
{
  int ret = next_oldest;
  next_oldest = (next_oldest + 1 ) % amt;
  return ret;
}

void csFountainMeshObject::Update (csTicks elapsed_time)
{
  SetupObject ();
  csParticleSystem::Update (elapsed_time);
  float delta_t = elapsed_time / 1000.0f; // in seconds
  // move particles;
  int i;
  for (i=0 ; i < particles.Length() ; i++)
  {
    part_speed[i] += accel * delta_t;
    part_pos[i] += part_speed[i] * delta_t;
    GetParticle(i)->SetPosition (part_pos[i]);
    part_age[i] += delta_t;
  }

  // restart a number of particles
  float intersperse = fall_time / (float)amt;
  float todo_time = delta_t + time_left;
  while (todo_time > intersperse)
  {
    RestartParticle (FindOldest (), todo_time);
    todo_time -= intersperse;
  }
  time_left = todo_time;
}

void csFountainMeshObject::HardTransform (const csReversibleTransform& t)
{
  origin = t.This2Other (origin);
  initialized = false;
  shapenr++;
}

//----------------------------------------------------------------------

SCF_IMPLEMENT_IBASE (csFountainMeshObjectFactory)
  SCF_IMPLEMENTS_INTERFACE (iMeshObjectFactory)
SCF_IMPLEMENT_IBASE_END

csFountainMeshObjectFactory::csFountainMeshObjectFactory (iBase *p,
	iObjectRegistry* s)
{
  SCF_CONSTRUCT_IBASE (p);
  object_reg = s;
}

csFountainMeshObjectFactory::~csFountainMeshObjectFactory ()
{
}

iMeshObject* csFountainMeshObjectFactory::NewInstance ()
{
  csFountainMeshObject* cm =
    new csFountainMeshObject (object_reg, (iMeshObjectFactory*)this);
  iMeshObject* im = SCF_QUERY_INTERFACE (cm, iMeshObject);
  im->DecRef ();
  return im;
}

//----------------------------------------------------------------------

SCF_IMPLEMENT_IBASE (csFountainMeshObjectType)
  SCF_IMPLEMENTS_INTERFACE (iMeshObjectType)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iPlugin)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csFountainMeshObjectType::eiPlugin)
  SCF_IMPLEMENTS_INTERFACE (iPlugin)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_FACTORY (csFountainMeshObjectType)

SCF_EXPORT_CLASS_TABLE (fountain)
  SCF_EXPORT_CLASS (csFountainMeshObjectType,
    "crystalspace.mesh.object.fountain",
    "Crystal Space Fountain Mesh Type")
SCF_EXPORT_CLASS_TABLE_END

csFountainMeshObjectType::csFountainMeshObjectType (iBase* pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiPlugin);
}

csFountainMeshObjectType::~csFountainMeshObjectType ()
{
}

iMeshObjectFactory* csFountainMeshObjectType::NewFactory ()
{
  csFountainMeshObjectFactory* cm =
    new csFountainMeshObjectFactory (this, object_reg);
  iMeshObjectFactory* ifact = SCF_QUERY_INTERFACE (cm, iMeshObjectFactory);
  ifact->DecRef ();
  return ifact;
}
