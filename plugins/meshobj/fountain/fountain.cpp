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
#include "csgeom/matrix3.h"
#include "csgeom/fastsqrt.h"
#include "plugins/meshobj/fountain/fountain.h"
#include "imater.h"
#include <math.h>
#include <stdlib.h>

IMPLEMENT_IBASE_EXT (csFountainMeshObject)
  IMPLEMENTS_EMBEDDED_INTERFACE (iFountainState)
IMPLEMENT_IBASE_EXT_END

IMPLEMENT_EMBEDDED_IBASE (csFountainMeshObject::FountainState)
  IMPLEMENTS_INTERFACE (iFountainState)
IMPLEMENT_EMBEDDED_IBASE_END

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

    float radius = 10.0; // guessed radius of the fountain
    float height = 10.0; // guessed height
    bbox.Set(origin - csVector3(-radius,0,-radius), 
      origin + csVector3(+radius, +height, +radius) );

    // create particles
    for (int i=0 ; i<number ; i++)
    {
      AppendRectSprite (drop_width, drop_height, mat, lighted_particles);
      GetParticle(i)->SetMixmode(MixMode);
      RestartParticle(i, (fall_time / float(number)) * float(number-i));
      bbox.AddBoundingVertexSmart( part_pos[i] );
    }
    time_left = 0.0;
    next_oldest = 0;
    SetupColor ();
    SetupMixMode ();
  }
}

csFountainMeshObject::csFountainMeshObject (iSystem* system)
	: csParticleSystem (system)
{
  CONSTRUCT_EMBEDDED_IBASE (scfiFountainState);
  initialized = false;
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

void csFountainMeshObject::Update (cs_time elapsed_time)
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

//----------------------------------------------------------------------

IMPLEMENT_IBASE (csFountainMeshObjectFactory)
  IMPLEMENTS_INTERFACE (iMeshObjectFactory)
IMPLEMENT_IBASE_END

csFountainMeshObjectFactory::csFountainMeshObjectFactory (iSystem* system)
{
  CONSTRUCT_IBASE (NULL);
  csFountainMeshObjectFactory::system = system;
}

csFountainMeshObjectFactory::~csFountainMeshObjectFactory ()
{
}

iMeshObject* csFountainMeshObjectFactory::NewInstance ()
{
  csFountainMeshObject* cm = new csFountainMeshObject (system);
  return QUERY_INTERFACE (cm, iMeshObject);
}

//----------------------------------------------------------------------

IMPLEMENT_IBASE (csFountainMeshObjectType)
  IMPLEMENTS_INTERFACE (iMeshObjectType)
  IMPLEMENTS_INTERFACE (iPlugIn)
IMPLEMENT_IBASE_END

IMPLEMENT_FACTORY (csFountainMeshObjectType)

EXPORT_CLASS_TABLE (fountain)
  EXPORT_CLASS (csFountainMeshObjectType, "crystalspace.meshobj.fountain",
    "Crystal Space Fountain Mesh Type")
EXPORT_CLASS_TABLE_END

csFountainMeshObjectType::csFountainMeshObjectType (iBase* pParent)
{
  CONSTRUCT_IBASE (pParent);
}

csFountainMeshObjectType::~csFountainMeshObjectType ()
{
}

bool csFountainMeshObjectType::Initialize (iSystem* system)
{
  csFountainMeshObjectType::system = system;
  return true;
}

iMeshObjectFactory* csFountainMeshObjectType::NewFactory ()
{
  csFountainMeshObjectFactory* cm = new csFountainMeshObjectFactory (system);
  return QUERY_INTERFACE (cm, iMeshObjectFactory);
}

