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
#include "csengine/csspr2d.h"
#include "csengine/light.h"
#include "csengine/engine.h"
#include "csengine/rview.h"
#include "csengine/sector.h"
#include "csgeom/matrix3.h"
#include "csgeom/fastsqrt.h"
#include "csfx/partsnow.h"
#include <math.h>
#include <stdlib.h>

IMPLEMENT_CSOBJTYPE (csSnowParticleSystem, csParticleSystem)


csSnowParticleSystem :: csSnowParticleSystem(csObject* theParent, int number, csMaterialWrapper* mat, 
  UInt mixmode, bool lighted_particles, float drop_width, float drop_height,
  const csVector3& rainbox_min, const csVector3& rainbox_max, 
  const csVector3& fall_speed, float swirl)
  : csParticleSystem(theParent)
{
  part_pos = new csVector3[number];
  part_speed = new csVector3[number];
  rain_dir = fall_speed;
  swirl_amount = swirl;
  rainbox.Set(rainbox_min, rainbox_max);
  bbox.Set(rainbox_min, rainbox_max);
  /// spread particles evenly through box
  csVector3 size = rainbox_max - rainbox_min;
  csVector3 pos;
  for(int i=0; i<number; i++)
  {
    AppendRectSprite(drop_width, drop_height, mat, lighted_particles);
    GetParticle(i)->SetMixmode(mixmode);
    pos = GetRandomDirection(size, rainbox.Min()) ;
    GetParticle(i)->SetPosition(pos);
    part_pos[i] = pos;
    part_speed[i] = 0.0;
  }
}

csSnowParticleSystem :: ~csSnowParticleSystem()
{
  delete[] part_pos;
  delete[] part_speed;
}

void csSnowParticleSystem :: Update(cs_time elapsed_time)
{
  csParticleSystem::Update(elapsed_time);
  float delta_t = elapsed_time / 1000.0f; // in seconds
  // move particles;
  csVector3 move, pos;
  int i;
  for(i=0; i<particles.Length(); i++)
  {
    move = rain_dir * delta_t;
    /// swirl a bit, for snow drifting in the wind...
    csVector3 swirl = GetRandomDirection() * swirl_amount;
    swirl.y = 0.0;
    part_speed[i] += swirl * delta_t;
    move += part_speed[i] * delta_t;
    part_pos[i] += move;
    GetParticle(i)->SetPosition (part_pos[i]); 
  }
  // check if particles are out of the box.
  for(i=0; i<particles.Length(); i++)
  {
    if(!rainbox.In(part_pos[i]))
    {
      // this particle has left the box.
      // it will disappear.
      // To keep the number of particles (and thus the raininess)
      // constant another particle will appear in sight now.
      // @@@ rain only appears in box ceiling now, should appear on
      // opposite side of rain_dir... 

      // @@@ also shifty will not work very nicely with slanted rain.
      //   but perhaps it won't be too bad...
      float toolow = ABS(rainbox.MinY() - part_pos[i].y);
      float height = rainbox.MaxY() - rainbox.MinY();
      while(toolow>height) toolow-=height;
      pos = GetRandomDirection( csVector3 (rainbox.MaxX() - rainbox.MinX(), 
        0.0f, rainbox.MaxZ() - rainbox.MinZ()), rainbox.Min() );
      pos.y = rainbox.MaxY() - toolow;
      if(pos.y < rainbox.MinY() || pos.y > rainbox.MaxY()) 
        pos.y = rainbox.MaxY() - height * ((float)rand() / (1.0 + RAND_MAX));
      GetParticle(i)->SetPosition(pos);
      part_pos[i] = pos;
      part_speed[i] = 0.0;
    }
  }
}

