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
#include "csfx/partfire.h"
#include "csgeom/matrix3.h"
#include "csgeom/fastsqrt.h"
#include <math.h>
#include <stdlib.h>

IMPLEMENT_CSOBJTYPE (csFireParticleSystem, csParticleSystem)


csFireParticleSystem :: csFireParticleSystem(csObject* theParent, 
  int number, csMaterialWrapper* mat, UInt mixmode, 
  bool lighted_particles, float drop_width, float drop_height,
  float total_time, const csVector3& dir, const csVector3& origin,
  float swirl, float color_scale
  )
  : csParticleSystem(theParent)
{
  light = NULL;
  light_engine = NULL;
  delete_light = false;
  part_pos = new csVector3[number];
  part_speed = new csVector3[number];
  part_age = new float[number];
  direction = dir;
  csFireParticleSystem::total_time = total_time;
  csFireParticleSystem::origin = origin;
  csFireParticleSystem::swirl = swirl;
  csFireParticleSystem::color_scale = color_scale;
  amt = number;

  float radius = drop_width * swirl; // guessed radius of the fire
  csVector3 height = total_time * dir; // guessed height
  bbox.Set(origin - csVector3(-radius,0,-radius), 
    origin + csVector3(+radius, 0, +radius) + height );

  // create particles
  for(int i=0; i<number; i++)
  {
    AppendRectSprite(drop_width, drop_height, mat, lighted_particles);
    GetParticle(i)->SetMixmode(mixmode);
    RestartParticle(i, (total_time / float(number)) * float(number-i));
    bbox.AddBoundingVertexSmart( part_pos[i] );
  }
  time_left = 0.0;
  next_oldest = 0;
  light_time = (int) (3000.0 *rand() / (1.0 + RAND_MAX));
}

csFireParticleSystem :: ~csFireParticleSystem()
{
  if(light && delete_light)
  {
    light_engine->RemoveDynLight( (csDynLight*)light);
    delete light;
    light = NULL;
  }
  delete[] part_pos;
  delete[] part_speed;
  delete[] part_age;
}


void csFireParticleSystem :: RestartParticle(int index, float pre_move)
{
  part_pos[index] = origin;
  part_speed[index] = direction;
  part_age[index] = 0.0;
  GetParticle(index)->SetPosition(part_pos[index]);

  MoveAndAge(index, pre_move);
}


void csFireParticleSystem :: MoveAndAge(int i, float delta_t)
{
  csVector3 accel = GetRandomDirection() * swirl;
  part_speed[i] += accel * delta_t;
  part_pos[i] += part_speed[i] * delta_t;
  GetParticle(i)->SetPosition (part_pos[i]); 
  part_age[i] += delta_t;

  // set the colour based on the age of the particle
  //   white->yellow->red->gray->black
  // col_age: 1.0 means total_time;
  const float col_age[] = {0., 0.05, 0.2, 0.5, 1.0};
  const csColor cols[] = {
    csColor(1.,1.,1.),
    csColor(1.,1.,0.),
    csColor(1.,0.,0.),
    csColor(0.6,0.6,0.6),
    csColor(0.1,0.1,0.1)
  };
  const int nr_colors = 5;
  csColor col;
  col = cols[nr_colors-1];
  float age = part_age[i] / total_time;
  for(int k=1; k<nr_colors; k++)
  {
    if(age >= col_age[k-1] && age < col_age[k])
    {
      /// colouring fraction
      float fr = (age - col_age[k-1]) / (col_age[k] - col_age[k-1]);
      col = cols[k-1] * (1.0-fr) + cols[k] * fr;
    }
  }
  GetParticle(i)->SetColor(col * color_scale);
}


int csFireParticleSystem :: FindOldest()
{
  int ret = next_oldest;
  next_oldest = (next_oldest + 1 ) % amt;
  return ret;
}

void csFireParticleSystem :: Update(cs_time elapsed_time)
{
  csParticleSystem::Update(elapsed_time);
  if(light)
  {
    light_time += elapsed_time;
    csColor newcol;
    newcol.red =   1.0 - 0.3*sin(light_time/10. + origin.x);
    newcol.green = 0.7 - 0.3*sin(light_time/15. + origin.y);
    newcol.blue =  0.3 + 0.3*sin(light_time/10. + origin.z);
    light->SetColor(newcol);
  }

  float delta_t = elapsed_time / 1000.0f; // in seconds
  // move particles;
  int i;
  for(i=0; i<particles.Length(); i++)
  {
    MoveAndAge(i, delta_t);
  }

  /// restart a number of particles
  float intersperse = total_time / (float)amt;
  float todo_time = delta_t + time_left;
  while(todo_time > intersperse)
  {
    RestartParticle(FindOldest(), todo_time);
    todo_time -= intersperse;
  }
  time_left = todo_time;
}


void csFireParticleSystem :: AddLight(csEngine *engine, csSector *sec)
{
  if(light) return;
  csDynLight *explight = new csDynLight(origin.x, origin.y, origin.z, 
    5, 1, 1, 0);
  engine->AddDynLight(explight);
  explight->SetSector(sec);
  explight->Setup();
  light = explight;
  delete_light = true;
  light_engine = engine;
}
