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
#include "explo.h"
#include "imater.h"
#include "iengine.h"
#include "idlight.h"
#include "ilight.h"
#include "qsqrt.h"
#include <math.h>
#include <stdlib.h>

IMPLEMENT_IBASE_EXT (csExploMeshObject)
  IMPLEMENTS_EMBEDDED_INTERFACE (iExplosionState)
IMPLEMENT_IBASE_EXT_END

IMPLEMENT_EMBEDDED_IBASE (csExploMeshObject::ExplosionState)
  IMPLEMENTS_INTERFACE (iExplosionState)
IMPLEMENT_EMBEDDED_IBASE_END

void csExploMeshObject::SetupObject ()
{
  if (!initialized)
  {
    RemoveParticles ();
    initialized = true;
    int i;
    csVector3 pos;
    /// add particles
    bbox.StartBoundingBox (center);
    float sqmaxaccel = 0.0;
    float sqmaxspeed = 0.0;
    csVector3 bbox_radius (part_radius, part_radius, part_radius);
    bbox_radius *= 10.;
    radius = bbox_radius.x; // @@@ This is only an approximation.

    // The bounding box for the explosion particle system is not accurate.
    // For efficiency reasons we overestimate this bounding box and never
    // calculate it again.
    for (i=0 ; i < number ; i++)
    {
      AppendRegularSprite (nr_sides, part_radius, mat, lighted_particles);
      pos = center + GetRandomDirection() * spread_pos;
      GetParticle(i)->SetPosition (pos);
      part_speed[i] = push + spread_speed * GetRandomDirection();
      part_accel[i] = (pos - center) * spread_accel * GetRandomDirection();
      if (part_speed[i].SquaredNorm() > sqmaxspeed) 
        sqmaxspeed = part_speed[i].SquaredNorm();
      if (part_accel[i].SquaredNorm() > sqmaxaccel) 
        sqmaxaccel = part_accel[i].SquaredNorm();
      bbox.AddBoundingVertexSmart(pos+bbox_radius);
      bbox.AddBoundingVertexSmart(pos-bbox_radius);
    }
    startbox = bbox;
    radiusnow = 1.0;
    maxspeed = qsqrt (sqmaxspeed);
    maxaccel = qsqrt (sqmaxaccel);
    SetupColor ();
    SetupMixMode ();
  }
}

csExploMeshObject::csExploMeshObject (iSystem* system)
	: csNewtonianParticleSystem (system)
{
  CONSTRUCT_EMBEDDED_IBASE (scfiExplosionState);
  initialized = false;
  /// defaults
  has_light = false;
  light_sector = NULL;
  explight = NULL;
  ilight = NULL;
  scale_particles = false;
  push.Set (0, 0, 0);
  center.Set (0, 0, 0);
  number = 50;
}

csExploMeshObject::~csExploMeshObject()
{
  //@@@TODO: if(has_light) RemoveLight();
}


void csExploMeshObject::Update (cs_time elapsed_time)
{
  SetupObject ();
  csNewtonianParticleSystem::Update (elapsed_time);

  float delta_t = elapsed_time / 1000.0f;
  float addedradius = ( maxspeed + maxaccel * delta_t ) * delta_t;
  radiusnow += addedradius;

  // size of particles is exponentially reduced in fade time.
  if (scale_particles && self_destruct && time_to_live < fade_particles)
    ScaleBy (1.0 - (fade_particles - time_to_live)/((float)fade_particles));
  if (!has_light) return;
  csColor newcol;
  newcol.red =   1.0 - 0.3*sin(time_to_live/10. + center.x);
  newcol.green = 1.0 - 0.3*sin(time_to_live/15. + center.y);
  newcol.blue =  0.3 + 0.3*sin(time_to_live/10. + center.z);
  if (self_destruct && time_to_live < light_fade)
    newcol *= 1.0 - (light_fade - time_to_live)/((float)light_fade);
  ilight->SetColor (newcol);
}

void csExploMeshObject::AddLight (iEngine *engine, iSector *sec, cs_time fade)
{
  if (has_light) return;
  light_engine = engine;
  light_sector = sec;
  light_fade = fade;
  has_light = true;
  explight = engine->CreateDynLight (center, 5, csColor (1, 1, 0));
  ilight = QUERY_INTERFACE (explight, iLight);
  ilight->SetSector (light_sector);
  explight->Setup ();
}


void csExploMeshObject::RemoveLight ()
{
  if (!has_light) return;
  has_light = false;
  light_engine->RemoveDynLight (explight);
  explight->DecRef ();
  explight = NULL;
  ilight = NULL;
  light_sector = NULL;
  light_engine = NULL;
}

void csExploMeshObject::HardTransform (const csReversibleTransform& t)
{
  center = t.This2Other (center);
  initialized = false;
}

//----------------------------------------------------------------------

IMPLEMENT_IBASE (csExploMeshObjectFactory)
  IMPLEMENTS_INTERFACE (iMeshObjectFactory)
IMPLEMENT_IBASE_END

csExploMeshObjectFactory::csExploMeshObjectFactory (iSystem* system)
{
  CONSTRUCT_IBASE (NULL);
  csExploMeshObjectFactory::system = system;
}

csExploMeshObjectFactory::~csExploMeshObjectFactory ()
{
}

iMeshObject* csExploMeshObjectFactory::NewInstance ()
{
  csExploMeshObject* cm = new csExploMeshObject (system);
  return QUERY_INTERFACE (cm, iMeshObject);
}

//----------------------------------------------------------------------

IMPLEMENT_IBASE (csExploMeshObjectType)
  IMPLEMENTS_INTERFACE (iMeshObjectType)
  IMPLEMENTS_INTERFACE (iPlugIn)
IMPLEMENT_IBASE_END

IMPLEMENT_FACTORY (csExploMeshObjectType)

EXPORT_CLASS_TABLE (explo)
  EXPORT_CLASS (csExploMeshObjectType, "crystalspace.mesh.object.explosion",
    "Crystal Space Explosion Mesh Type")
EXPORT_CLASS_TABLE_END

csExploMeshObjectType::csExploMeshObjectType (iBase* pParent)
{
  CONSTRUCT_IBASE (pParent);
}

csExploMeshObjectType::~csExploMeshObjectType ()
{
}

bool csExploMeshObjectType::Initialize (iSystem* system)
{
  csExploMeshObjectType::system = system;
  return true;
}

iMeshObjectFactory* csExploMeshObjectType::NewFactory ()
{
  csExploMeshObjectFactory* cm = new csExploMeshObjectFactory (system);
  return QUERY_INTERFACE (cm, iMeshObjectFactory);
}

