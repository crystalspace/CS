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
#include "csengine/world.h"
#include "csengine/particle.h"
#include <math.h>
#include <stdlib.h>


IMPLEMENT_CSOBJTYPE (csParticleSystem, csObject)
IMPLEMENT_CSOBJTYPE (csNewtonianParticleSystem, csParticleSystem)
IMPLEMENT_CSOBJTYPE (csSpiralParticleSystem, csNewtonianParticleSystem)
IMPLEMENT_CSOBJTYPE (csParSysExplosion, csNewtonianParticleSystem)
IMPLEMENT_IBASE (csParticleSystem)
  IMPLEMENTS_INTERFACE(iParticle)
IMPLEMENT_IBASE_END


csParticleSystem :: csParticleSystem()
  : csObject()
{
  CONSTRUCT_IBASE (NULL);
  particles.SetLength(0);
  self_destruct = false;
  time_to_live = 0;
  to_delete = false;
  // add me to the world
  csWorld::current_world->particle_systems.Push(this);
  // defaults
  change_size = false;
  change_color = false;
  change_alpha = false;
  change_rotation = false;
}


csParticleSystem :: ~csParticleSystem()
{
  // delete all my particles
  for(int i=0; i<particles.Length(); i++)
    if(particles[i])
      GetParticle(i)->DecRef();
}


void csParticleSystem :: AppendRectSprite(float width, float height, 
  csTextureHandle *txt, bool lighted)
{
  csSprite2D *part = new csSprite2D();
  csWorld::current_world->sprites.Push(part);
  csColoredVertices& vs = part->GetVertices();
  vs.SetLimit(4);
  vs.SetLength(4);
  vs[0].pos.Set(-width,-height); vs[0].u=0.; vs[0].v=0.;
  vs[1].pos.Set(-width,+height); vs[1].u=0.; vs[1].v=1.;
  vs[2].pos.Set(+width,+height); vs[2].u=1.; vs[2].v=1.;
  vs[3].pos.Set(+width,-height); vs[3].u=1.; vs[3].v=0.;
  part->SetLighting( lighted );
  part->SetColor( csColor(1.0, 1.0, 1.0) );
  part->SetTexture(txt);
  AppendParticle(part);
  part->DecRef(); 
}


void csParticleSystem :: AppendRegularSprite(int n, float radius, 
  csTextureHandle* txt, bool lighted)
{
  csSprite2D *part = new csSprite2D();
  csWorld::current_world->sprites.Push(part);
  part->CreateRegularVertices(n, true);
  part->ScaleBy(radius);
  part->SetTexture(txt);
  part->SetLighting( lighted );
  part->SetColor( csColor(1.0, 1.0, 1.0) );
  AppendParticle(part);
  part->DecRef(); 
}


void csParticleSystem :: SetMixmode(UInt mode)
{
  for(int i = 0; i<particles.Length(); i++)
    GetParticle(i)->SetMixmode(mode);
}


void csParticleSystem :: SetColor(const csColor& col)
{
  for(int i = 0; i<particles.Length(); i++)
    GetParticle(i)->SetColor(col);
}


void csParticleSystem :: AddColor(const csColor& col)
{
  for(int i = 0; i<particles.Length(); i++)
    GetParticle(i)->AddColor(col);
}


void csParticleSystem :: MoveToSector(csSector *sector)
{
  for(int i = 0; i<particles.Length(); i++)
    GetParticle(i)->MoveToSector(sector);
}


void csParticleSystem :: SetPosition(const csVector3& pos)
{
  for(int i = 0; i<particles.Length(); i++)
    GetParticle(i)->SetPosition(pos);
}


void csParticleSystem :: MovePosition(const csVector3& move)
{
  for(int i = 0; i<particles.Length(); i++)
    GetParticle(i)->MovePosition(move);
}


void csParticleSystem :: ScaleBy(float factor)
{
  for(int i = 0; i<particles.Length(); i++)
    GetParticle(i)->ScaleBy(factor);
}


void csParticleSystem :: Rotate(float angle)
{
  for(int i = 0; i<particles.Length(); i++)
    GetParticle(i)->Rotate(angle);
}


void csParticleSystem :: Update(time_t elapsed_time)
{
  if(self_destruct)
  {
    if(elapsed_time >= time_to_live)
    {
      to_delete = true;
      time_to_live = 0;
      /// and a calling virtual function can process without crashing
      return;
    }
    time_to_live -= elapsed_time;
  }
  float elapsed_seconds = ((float)elapsed_time) / 1000.0;
  if(change_color)
    AddColor(colorpersecond * elapsed_seconds);
  if(change_size)
    ScaleBy(pow(scalepersecond, elapsed_seconds));
  if(change_alpha)
  {
    alpha_now += alphapersecond * elapsed_seconds;
    if(alpha_now < 0.0f) alpha_now = 0.0f;
    else if(alpha_now > 1.0f) alpha_now = 1.0f;
    SetMixmode(CS_FX_SETALPHA(alpha_now));
  }
  if(change_rotation)
    Rotate(anglepersecond * elapsed_seconds);
}

//---------------------------------------------------------------------

/// helping func. Returns vector of with -1..+1 members. Varying length!
static csVector3& GetRandomDirection ()
{
  static csVector3 dir;
  dir.x = 2.0 * rand() / (1.0+RAND_MAX) - 1.0;
  dir.y = 2.0 * rand() / (1.0+RAND_MAX) - 1.0;
  dir.z = 2.0 * rand() / (1.0+RAND_MAX) - 1.0;
  return dir;
}

static csVector3& GetRandomDirection (const csVector3& magnitude,
	const csVector3& offset)
{
  static csVector3 dir;
  dir.x = (rand() / (1.0+RAND_MAX)) * magnitude.x;
  dir.y = (rand() / (1.0+RAND_MAX)) * magnitude.y;
  dir.z = (rand() / (1.0+RAND_MAX)) * magnitude.z;
  dir = dir + offset;
  return dir;
}


//-- csSpiralParticleSystem ------------------------------------------

csSpiralParticleSystem::csSpiralParticleSystem (int max,
	const csVector3& source, csTextureHandle* txt) : csNewtonianParticleSystem (max)
{
  csSpiralParticleSystem::max = max;
  csSpiralParticleSystem::source = source;
  csSpiralParticleSystem::txt = txt;
  time_before_new_particle = 0;
  last_reuse = 0;
}

csSpiralParticleSystem::~csSpiralParticleSystem ()
{
}

void csSpiralParticleSystem::MoveToSector (csSector *sector)
{
  this_sector = sector;
  csNewtonianParticleSystem::MoveToSector (sector);
}

void csSpiralParticleSystem::Update (time_t elapsed_time)
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
      AppendRegularSprite (3, .02, txt, false);	// @@@ PARAMETER
      part_idx = GetNumParticles ()-1;
      GetParticle (part_idx)->MoveToSector (this_sector);
    }
    iParticle* part = GetParticle (part_idx);
    part->SetPosition (source);
    csVector3 dir;
    dir = GetRandomDirection (csVector3 (.01, .01, .01), csVector3 (.1, .3, .1));

    SetSpeed (part_idx, dir);
    SetAccel (part_idx, csVector3 (0));
  }
  csNewtonianParticleSystem::Update (elapsed_time);
}

//-- csNewtonianParticleSystem ------------------------------------------

csNewtonianParticleSystem :: csNewtonianParticleSystem(int max)
  : csParticleSystem()
{
  // create csVector3's
  part_speed = new csVector3 [max];
  part_accel = new csVector3 [max];
}


csNewtonianParticleSystem :: ~csNewtonianParticleSystem()
{
  delete[] part_speed;
  delete[] part_accel;
}


void csNewtonianParticleSystem :: Update(time_t elapsed_time)
{
  csVector3 move;
  csParticleSystem::Update(elapsed_time);
  // time passed; together with CS 1 unit = 1 meter makes units right.
  float delta_t = elapsed_time / 1000.0; // in seconds
  for(int i=0; i<particles.Length(); i++)
  {
    // notice that the ordering of the lines (1) and (2) makes the
    // resulting newpos = a*dt^2 + v*dt + oldposition (i.e. paraboloid).
    part_speed[i] += part_accel[i] * delta_t; // (1)
    move = part_speed[i] * delta_t; // (2)
    GetParticle(i)->MovePosition (move); 
  }
}


//-- csParSysExplosion --------------------------------------------------


csParSysExplosion :: csParSysExplosion(int number_p, 
    const csVector3& explode_center, const csVector3& push, 
    csTextureHandle *txt, int nr_sides, float part_radius,
    bool lighted_particles,
    float spread_pos, float spread_speed, float spread_accel)
    : csNewtonianParticleSystem(number_p)
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
  for(i=0; i<number_p; i++)
  {
    AppendRegularSprite(nr_sides, part_radius, txt, lighted_particles);
    pos = center + GetRandomDirection() * spread_pos;
    GetParticle(i)->SetPosition (pos);
    part_speed[i] = push + spread_speed * GetRandomDirection();
    part_accel[i] = (pos - center) * spread_accel * GetRandomDirection();
  }

}


csParSysExplosion :: ~csParSysExplosion()
{
  if(has_light) RemoveLight();
}


void csParSysExplosion :: Update(time_t elapsed_time)
{
  csNewtonianParticleSystem::Update(elapsed_time);

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
  explight->Setup();
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


void csParSysExplosion :: AddLight(csWorld *world, csSector *sec, time_t fade)
{
  if(has_light) return;
  light_world = world;
  light_sector = sec;
  light_fade = fade;
  has_light = true;
  explight = new csDynLight(center.x, center.y, center.z, 5, 1, 1, 0);
  light_world->AddDynLight(explight);
  explight->SetSector(light_sector);
  explight->Setup();
}


void csParSysExplosion :: RemoveLight()
{
  if(!has_light) return;
  has_light = false;
  light_world->RemoveDynLight(explight);
  delete explight;
  explight = NULL;
  light_sector = NULL;
  light_world = NULL;
}
