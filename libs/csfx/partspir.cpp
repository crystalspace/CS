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
#include "csfx/partspir.h"
#include "csgeom/matrix3.h"
#include "csgeom/fastsqrt.h"
#include <math.h>
#include <stdlib.h>

IMPLEMENT_CSOBJTYPE (csSpiralParticleSystem, csNewtonianParticleSystem)


//-- csSpiralParticleSystem ------------------------------------------

csSpiralParticleSystem::csSpiralParticleSystem (csObject* theParent, int max,
	const csVector3& source, csMaterialWrapper* mat) : csNewtonianParticleSystem (theParent, max)
{
  csSpiralParticleSystem::max = max;
  csSpiralParticleSystem::source = source;
  csSpiralParticleSystem::mat = mat;
  time_before_new_particle = 0;
  last_reuse = 0;
  float radius = 10.0; // guessed radius of the spiral;
  float height = 10.0; // guessed height
  bbox.Set(source - csVector3(-radius,0,-radius), 
    source + csVector3(+radius, +height, +radius) );
}

csSpiralParticleSystem::~csSpiralParticleSystem ()
{
}

void csSpiralParticleSystem::MoveToSector (csSector *sector)
{
  this_sector = sector;
  csNewtonianParticleSystem::MoveToSector (sector);
}

void csSpiralParticleSystem::Update (cs_time elapsed_time)
{
  int i;
  // Update the acceleration vectors first.
  for (i=0 ; i<particles.Length () ; i++)
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

