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
#include "csengine/light.h"
#include "csengine/engine.h"
#include "csengine/particle.h"
#include "csengine/rview.h"
#include "csengine/sector.h"
#include "csfx/partexp.h"
#include "csgeom/matrix3.h"
#include "csgeom/fastsqrt.h"
#include <math.h>
#include <stdlib.h>

IMPLEMENT_CSOBJTYPE (csParSysExplosion, csNewtonianParticleSystem)


csParSysExplosion :: csParSysExplosion(csObject* theParent, int number_p, 
    const csVector3& explode_center, const csVector3& push, 
    csMaterialWrapper *mat, int nr_sides, float part_radius,
    bool lighted_particles,
    float spread_pos, float spread_speed, float spread_accel)
    : csNewtonianParticleSystem(theParent, number_p)
{
  int i;
  csVector3 pos;
  center = explode_center;
  /// defaults
  has_light = false;
  light_sector = NULL;
  explight = NULL;
  scale_particles = false;
  /// add particles
  bbox.AddBoundingVertex(center);
  float sqmaxaccel = 0.0;
  float sqmaxspeed = 0.0;
  csVector3 bbox_radius (part_radius, part_radius, part_radius);
  bbox_radius *= 10.;
  // The bounding box for the explosion particle system is not accurate.
  // For efficiency reasons we overestimate this bounding box and never
  // calculate it again.
  for(i=0; i<number_p; i++)
  {
    AppendRegularSprite(nr_sides, part_radius, mat, lighted_particles);
    pos = center + GetRandomDirection() * spread_pos;
    GetParticle(i)->SetPosition (pos);
    part_speed[i] = push + spread_speed * GetRandomDirection();
    part_accel[i] = (pos - center) * spread_accel * GetRandomDirection();
    if(part_speed[i].SquaredNorm() > sqmaxspeed) 
      sqmaxspeed = part_speed[i].SquaredNorm();
    if(part_accel[i].SquaredNorm() > sqmaxaccel) 
      sqmaxaccel = part_accel[i].SquaredNorm();
    bbox.AddBoundingVertexSmart(pos+bbox_radius);
    bbox.AddBoundingVertexSmart(pos-bbox_radius);
  }
  startbox = bbox;
  radiusnow = 1.0;
  maxspeed = FastSqrt(sqmaxspeed);
  maxaccel = FastSqrt(sqmaxaccel);
}


csParSysExplosion :: ~csParSysExplosion()
{
  if(has_light) RemoveLight();
}


void csParSysExplosion :: Update(cs_time elapsed_time)
{
  csNewtonianParticleSystem::Update(elapsed_time);

  float delta_t = elapsed_time / 1000.0f;
  float addedradius = ( maxspeed + maxaccel * delta_t ) * delta_t;
  radiusnow += addedradius;

  // size of particles is exponentially reduced in fade time.
  if(scale_particles && self_destruct && time_to_live < fade_particles)
    ScaleBy (1.0 - (fade_particles - time_to_live)/((float)fade_particles));
  if(!has_light) return;
  csColor newcol;
  newcol.red =   1.0 - 0.3*sin(time_to_live/10. + center.x);
  newcol.green = 1.0 - 0.3*sin(time_to_live/15. + center.y);
  newcol.blue =  0.3 + 0.3*sin(time_to_live/10. + center.z);
  if(self_destruct && time_to_live < light_fade)
    newcol *= 1.0 - (light_fade - time_to_live)/((float)light_fade);
  explight->SetColor(newcol);
}


void csParSysExplosion :: MoveToSector(csSector *sector)
{
  csParticleSystem :: MoveToSector(sector); // move sprites
  if(has_light)
  {
    light_sector = sector;
    explight->SetSector(light_sector);
    explight->Setup();
  }
}


void csParSysExplosion :: AddLight(csEngine *engine, csSector *sec, cs_time fade)
{
  if(has_light) return;
  light_engine = engine;
  light_sector = sec;
  light_fade = fade;
  has_light = true;
  explight = new csDynLight(center.x, center.y, center.z, 5, 1, 1, 0);
  light_engine->AddDynLight(explight);
  explight->SetSector(light_sector);
  explight->Setup();
}


void csParSysExplosion :: RemoveLight()
{
  if(!has_light) return;
  has_light = false;
  light_engine->RemoveDynLight(explight);
  delete explight;
  explight = NULL;
  light_sector = NULL;
  light_engine = NULL;
}


