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
#include "csengine/particle.h"
#include "csengine/rview.h"
#include "csengine/sector.h"
#include "csfx/partfoun.h"
#include "csgeom/matrix3.h"
#include "csgeom/fastsqrt.h"
#include <math.h>
#include <stdlib.h>

IMPLEMENT_CSOBJTYPE (csFountainParticleSystem, csParticleSystem)


csFountainParticleSystem :: csFountainParticleSystem(csObject* theParent, 
  int number, csMaterialWrapper* mat, UInt mixmode, 
  bool lighted_particles, float drop_width, float drop_height,
  const csVector3& spot, const csVector3& accel, float fall_time,
  float speed, float opening, float azimuth, float elevation)
  : csParticleSystem(theParent)
{
  part_pos = new csVector3[number];
  part_speed = new csVector3[number];
  part_age = new float[number];
  origin = spot;
  csFountainParticleSystem::accel = accel;
  csFountainParticleSystem::fall_time = fall_time;
  csFountainParticleSystem::speed = speed;
  csFountainParticleSystem::opening = opening;
  csFountainParticleSystem::azimuth = azimuth;
  csFountainParticleSystem::elevation = elevation;
  amt = number;

  float radius = 10.0; // guessed radius of the fountain
  float height = 10.0; // guessed height
  bbox.Set(spot - csVector3(-radius,0,-radius), 
    spot + csVector3(+radius, +height, +radius) );

  // create particles
  for(int i=0; i<number; i++)
  {
    AppendRectSprite(drop_width, drop_height, mat, lighted_particles);
    GetParticle(i)->SetMixmode(mixmode);
    RestartParticle(i, (fall_time / float(number)) * float(number-i));
    bbox.AddBoundingVertexSmart( part_pos[i] );
  }
  time_left = 0.0;
  next_oldest = 0;
}

csFountainParticleSystem :: ~csFountainParticleSystem()
{
  delete[] part_pos;
  delete[] part_speed;
  delete[] part_age;
}


void csFountainParticleSystem :: RestartParticle(int index, float pre_move)
{
  csVector3 dest; // destination spot of particle (for speed at start)
  dest.Set(speed, 0.0f, 0.0f);
  /// now make it shoot to a circle in the x direction
  float rotz_open = 2.0 * opening * (rand() / (1.0+RAND_MAX)) - opening;
  csZRotMatrix3 openrot(rotz_open);
  dest = openrot * dest;
  float rot_around = 2.0 * PI * (rand() / (1.0+RAND_MAX));
  csXRotMatrix3 xaround(rot_around);
  dest = xaround * dest;
  /// now dest point to somewhere in a circular cur of a sphere around the 
  /// x axis.

  /// direct the fountain to the users dirction
  csZRotMatrix3 elev(elevation);
  dest = elev * dest;
  csYRotMatrix3 compassdir(azimuth);
  dest = compassdir * dest;

  /// now dest points to the exit speed of the spout if that spout was
  /// at 0,0,0.
  part_pos[index] = origin;
  part_speed[index] = dest;

  // pre move a bit (in a perfect arc)
  part_speed[index] += accel * pre_move;
  part_pos[index] += part_speed[index] * pre_move;
  part_age[index] = pre_move;

  GetParticle(index)->SetPosition(part_pos[index]);
}


int csFountainParticleSystem :: FindOldest()
{
  int ret = next_oldest;
  next_oldest = (next_oldest + 1 ) % amt;
  return ret;
}

void csFountainParticleSystem :: Update(cs_time elapsed_time)
{
  csParticleSystem::Update(elapsed_time);
  float delta_t = elapsed_time / 1000.0f; // in seconds
  // move particles;
  int i;
  for(i=0; i<particles.Length(); i++)
  {
    part_speed[i] += accel * delta_t;
    part_pos[i] += part_speed[i] * delta_t;
    GetParticle(i)->SetPosition (part_pos[i]); 
    part_age[i] += delta_t;
  }

  /// restart a number of particles
  float intersperse = fall_time / (float)amt;
  float todo_time = delta_t + time_left;
  while(todo_time > intersperse)
  {
    RestartParticle(FindOldest(), todo_time);
    todo_time -= intersperse;
  }
  time_left = todo_time;
}

