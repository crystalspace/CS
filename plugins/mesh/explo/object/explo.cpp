/*
    Copyright (C) 2000-2001 by Jorrit Tyberghein
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
#include "csgeom/math3d.h"
#include "csgeom/matrix3.h"
#include "csgeom/transfrm.h"
#include "explo.h"
#include "ivideo/material.h"
#include "iengine/material.h"
#include "iengine/engine.h"
#include "iengine/dynlight.h"
#include "iengine/light.h"
#include "qsqrt.h"
#include <math.h>
#include <stdlib.h>

CS_IMPLEMENT_PLATFORM_PLUGIN

SCF_IMPLEMENT_IBASE_EXT (csExploMeshObject)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iExplosionState)
SCF_IMPLEMENT_IBASE_EXT_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csExploMeshObject::ExplosionState)
  SCF_IMPLEMENTS_INTERFACE (iExplosionState)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

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
      if (part_speed && part_accel)
      {
        part_speed[i] = push + spread_speed * GetRandomDirection();
        part_accel[i] = (pos - center) * spread_accel * GetRandomDirection();
        if (part_speed[i].SquaredNorm() > sqmaxspeed)
          sqmaxspeed = part_speed[i].SquaredNorm();
        if (part_accel[i].SquaredNorm() > sqmaxaccel)
          sqmaxaccel = part_accel[i].SquaredNorm();
      }
      bbox.AddBoundingVertexSmart(pos+bbox_radius);
      bbox.AddBoundingVertexSmart(pos-bbox_radius);
    }
    startbox = bbox;
    radiusnow = 1.0;
    maxspeed = qsqrt (sqmaxspeed);
    maxaccel = qsqrt (sqmaxaccel);
    SetupColor ();
    SetupMixMode ();
    float r = qsqrt (csSquaredDist::PointPoint (bbox.Max (), bbox.Min ())) / 2;
    radius.Set (r, r, r);
  }
}

csExploMeshObject::csExploMeshObject (iObjectRegistry* object_reg,
  iMeshObjectFactory* factory) : csNewtonianParticleSystem (object_reg, factory)
{
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiExplosionState);
  /// defaults
  has_light = false;
  light_sector = NULL;
  explight = NULL;
  ilight = NULL;
  scale_particles = false;
  push.Set (0, 0, 0);
  center.Set (0, 0, 0);
  number = 50;
  nr_sides = 3;
  part_radius = .1;
}

csExploMeshObject::~csExploMeshObject()
{
  if (has_light) RemoveLight();
}


void csExploMeshObject::Update (csTicks elapsed_time)
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

void csExploMeshObject::AddLight (iEngine *engine, iSector *sec, csTicks fade)
{
  if (has_light) return;
  light_engine = engine;
  light_sector = sec;
  light_fade = fade;
  has_light = true;
  explight = engine->CreateDynLight (center, 5, csColor (1, 1, 0));
  ilight = SCF_QUERY_INTERFACE (explight, iLight);
  ilight->SetSector (light_sector);
  ilight->DecRef ();
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
  shapenr++;
}

//----------------------------------------------------------------------

SCF_IMPLEMENT_IBASE (csExploMeshObjectFactory)
  SCF_IMPLEMENTS_INTERFACE (iMeshObjectFactory)
SCF_IMPLEMENT_IBASE_END

csExploMeshObjectFactory::csExploMeshObjectFactory (iBase *p,
	iObjectRegistry* s)
{
  SCF_CONSTRUCT_IBASE (p);
  logparent = NULL;
  object_reg = s;
}

csExploMeshObjectFactory::~csExploMeshObjectFactory ()
{
}

iMeshObject* csExploMeshObjectFactory::NewInstance ()
{
  csExploMeshObject* cm =
    new csExploMeshObject (object_reg, (iMeshObjectFactory*)this);
  iMeshObject* im = SCF_QUERY_INTERFACE (cm, iMeshObject);
  im->DecRef ();
  return im;
}

//----------------------------------------------------------------------

SCF_IMPLEMENT_IBASE (csExploMeshObjectType)
  SCF_IMPLEMENTS_INTERFACE (iMeshObjectType)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csExploMeshObjectType::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_FACTORY (csExploMeshObjectType)

SCF_EXPORT_CLASS_TABLE (explo)
  SCF_EXPORT_CLASS (csExploMeshObjectType,
    "crystalspace.mesh.object.explosion",
    "Crystal Space Explosion Mesh Type")
SCF_EXPORT_CLASS_TABLE_END

csExploMeshObjectType::csExploMeshObjectType (iBase* pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
}

csExploMeshObjectType::~csExploMeshObjectType ()
{
}

iMeshObjectFactory* csExploMeshObjectType::NewFactory ()
{
  csExploMeshObjectFactory* cm = new csExploMeshObjectFactory (this,
  	object_reg);
  iMeshObjectFactory* ifact = SCF_QUERY_INTERFACE (cm, iMeshObjectFactory);
  ifact->DecRef ();
  return ifact;
}
