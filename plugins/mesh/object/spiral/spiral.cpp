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
#include "csgeom/transfrm.h"
#include "spiral.h"
#include "imater.h"
#include "qsqrt.h"
#include <math.h>
#include <stdlib.h>

IMPLEMENT_IBASE_EXT (csSpiralMeshObject)
  IMPLEMENTS_EMBEDDED_INTERFACE (iSpiralState)
IMPLEMENT_IBASE_EXT_END

IMPLEMENT_EMBEDDED_IBASE (csSpiralMeshObject::SpiralState)
  IMPLEMENTS_INTERFACE (iSpiralState)
IMPLEMENT_EMBEDDED_IBASE_END

void csSpiralMeshObject::SetupObject ()
{
  if (!initialized)
  {
    RemoveParticles ();
    initialized = true;

    time_before_new_particle = 0;
    last_reuse = 0;
    float radius = 10.0; // guessed radius of the spiral;
    float height = 10.0; // guessed height
    bbox.Set(source - csVector3(-radius,0,-radius), 
      source + csVector3(+radius, +height, +radius) );

    // Calculate the maximum radius.
    csVector3 size = bbox.Max () - bbox.Min ();
    float max_size = size.x;
    if (size.y > max_size) max_size = size.y;
    if (size.z > max_size) max_size = size.z;
    float a = max_size/2.;
    radius = qsqrt (a*a + a*a);

    SetupColor ();
    SetupMixMode ();
  }
}

csSpiralMeshObject::csSpiralMeshObject (iSystem* system)
	: csNewtonianParticleSystem (system)
{
  CONSTRUCT_EMBEDDED_IBASE (scfiSpiralState);
  initialized = false;
  max = 50;
  source.Set (0, 0, 0);
  time_before_new_particle = 0;
  last_reuse = 0;
}

csSpiralMeshObject::~csSpiralMeshObject()
{
}


void csSpiralMeshObject::Update (cs_time elapsed_time)
{
  SetupObject ();
  int i;
  // Update the acceleration vectors first.
  for (i=0 ; i < particles.Length () ; i++)
  {
    // Take a 2D vector between 'source' and 'part_speed' as seen from above
    // and rotate it 90 degrees. This gives angle_vec which will be the
    // acceleration.
    csVector2 angle_vec (part_speed[i].z, -part_speed[i].x);
    float n = angle_vec.Norm ();
    if (ABS (n) > SMALL_EPSILON)
      angle_vec /= n;
    float delta_t = elapsed_time / 1000.0; // in seconds
    angle_vec *= delta_t * 2.;
    SetSpeed (i, part_speed[i]+csVector3 (angle_vec.x, 0, angle_vec.y));
  }

  time_before_new_particle -= elapsed_time;
  while (time_before_new_particle < 0)
  {
    time_before_new_particle += 15;	// @@@ PARAMETER
    int num = GetNumParticles ();
    int part_idx;
    if (num >= max)
    {
      part_idx = last_reuse;
      last_reuse = (last_reuse+1)%max;
    }
    else
    {
      AppendRegularSprite (3, .02, mat, false);	// @@@ PARAMETER
      part_idx = GetNumParticles ()-1;
      //GetParticle (part_idx)->MoveToSector (this_sector);
    }
    iParticle* part = GetParticle (part_idx);
    part->SetPosition (source);
    csVector3 dir;
    dir = GetRandomDirection (csVector3 (.01, .01, .01), csVector3 (.1, .3, .1));

    SetSpeed (part_idx, dir);
    SetAccel (part_idx, csVector3 (0));
    part->MovePosition( -(float)time_before_new_particle / 1000.0 * dir);
  }
  csNewtonianParticleSystem::Update (elapsed_time);
}

void csSpiralMeshObject::HardTransform (const csReversibleTransform& t)
{
  source = t.This2Other (source);
  initialized = false;
}

//----------------------------------------------------------------------

IMPLEMENT_IBASE (csSpiralMeshObjectFactory)
  IMPLEMENTS_INTERFACE (iMeshObjectFactory)
IMPLEMENT_IBASE_END

csSpiralMeshObjectFactory::csSpiralMeshObjectFactory (iSystem* system)
{
  CONSTRUCT_IBASE (NULL);
  csSpiralMeshObjectFactory::system = system;
}

csSpiralMeshObjectFactory::~csSpiralMeshObjectFactory ()
{
}

iMeshObject* csSpiralMeshObjectFactory::NewInstance ()
{
  csSpiralMeshObject* cm = new csSpiralMeshObject (system);
  return QUERY_INTERFACE (cm, iMeshObject);
}

//----------------------------------------------------------------------

IMPLEMENT_IBASE (csSpiralMeshObjectType)
  IMPLEMENTS_INTERFACE (iMeshObjectType)
  IMPLEMENTS_INTERFACE (iPlugIn)
IMPLEMENT_IBASE_END

IMPLEMENT_FACTORY (csSpiralMeshObjectType)

EXPORT_CLASS_TABLE (spiral)
  EXPORT_CLASS (csSpiralMeshObjectType, "crystalspace.mesh.object.spiral",
    "Crystal Space Spiral Mesh Type")
EXPORT_CLASS_TABLE_END

csSpiralMeshObjectType::csSpiralMeshObjectType (iBase* pParent)
{
  CONSTRUCT_IBASE (pParent);
}

csSpiralMeshObjectType::~csSpiralMeshObjectType ()
{
}

bool csSpiralMeshObjectType::Initialize (iSystem* system)
{
  csSpiralMeshObjectType::system = system;
  return true;
}

iMeshObjectFactory* csSpiralMeshObjectType::NewFactory ()
{
  csSpiralMeshObjectFactory* cm = new csSpiralMeshObjectFactory (system);
  return QUERY_INTERFACE (cm, iMeshObjectFactory);
}

