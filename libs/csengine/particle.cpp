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

#include "sysdef.h"
#include "csengine/csspr2d.h"
#include "csengine/light.h"
#include "csengine/world.h"
#include "csengine/particle.h"
#include <math.h>
#include <stdlib.h>


IMPLEMENT_CSOBJTYPE (csParticleSystem, csObject)
IMPLEMENT_CSOBJTYPE (csNewtonianParticleSystem, csParticleSystem)
IMPLEMENT_CSOBJTYPE (csParSysExplosion, csNewtonianParticleSystem)

csParticleSystem :: csParticleSystem(int max_part)
  : csObject()
{
  max_particles = max_part;
  num_particles = 0;
  self_destruct = false;
  time_to_live = 0;
  to_delete = false;
  part_2d = new csSprite2D* [max_particles];
  // for robustness
  for(int i=0; i<max_particles; i++)
    part_2d[i] = NULL;
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
  // first delete all sprites
  for(int i=0; i<num_particles; i++)
    if(part_2d[i])
    {
      //both lines should do the same.
      //csWorld::current_world->RemoveSprite(part_2d[i]);
      delete part_2d[i]; 
    }
  delete[] part_2d;
}


void csParticleSystem :: AppendSprite()
{
  if(num_particles<max_particles) 
  {
    part_2d[num_particles] = new csSprite2D();
    csWorld::current_world->sprites.Push(part_2d[num_particles]);
    num_particles ++;
  }
}


void csParticleSystem :: AppendRectSprite(float width, float height, 
  csTextureHandle *txt)
{
  if(num_particles>=max_particles) return;
  part_2d[num_particles] = new csSprite2D();
  csWorld::current_world->sprites.Push(part_2d[num_particles]);
  csColoredVertices& vs = part_2d[num_particles]->GetVertices();
  vs.SetLimit(4);
  vs.SetLength(4);
  vs[0].pos.Set(-width,-height); vs[0].u=0.; vs[0].v=0.;
  vs[1].pos.Set(-width,+height); vs[1].u=0.; vs[1].v=1.;
  vs[2].pos.Set(+width,+height); vs[2].u=1.; vs[2].v=1.;
  vs[3].pos.Set(+width,-height); vs[3].u=1.; vs[3].v=0.;
  for(int i=0; i<vs.Length(); i++)
    vs[i].color_init.Set(1.0, 1.0, 1.0);
  part_2d[num_particles]->SetTexture(txt);
  num_particles++;
}


void csParticleSystem :: AppendRegularSprite(int n, float radius, 
  csTextureHandle* txt)
{
  int idx = num_particles;
  AppendSprite();
  part_2d[idx]->CreateRegularVertices(n, true);
  part_2d[idx]->ScaleBy(radius);
  part_2d[idx]->SetTexture(txt);
  csColoredVertices& vs = part_2d[idx]->GetVertices();
  for(int i=0; i<vs.Length(); i++)
    vs[i].color_init.Set(1.0, 1.0, 1.0);
}


void csParticleSystem :: SetMixmodes(UInt mode)
{
  for(int i = 0; i<num_particles; i++)
    part_2d[i]->SetMixmode(mode);
}


void csParticleSystem :: SetOwner(csObject *owner)
{
  for(int i = 0; i<num_particles; i++)
    part_2d[i]->SetMyOwner(owner);
}


void csParticleSystem :: SetLighting(bool b)
{
  for(int i = 0; i<num_particles; i++)
    part_2d[i]->SetLighting(b);
}


void csParticleSystem :: SetColors(const csColor& col)
{
  for(int i = 0; i<num_particles; i++)
    part_2d[i]->SetColor(col);
}


void csParticleSystem :: MoveToSector(csSector *sector)
{
  for(int i = 0; i<num_particles; i++)
    part_2d[i]->MoveToSector(sector);
}


void csParticleSystem :: Update(time_t elapsed_time)
{
  if(self_destruct)
  {
    if(elapsed_time >= time_to_live)
    {
      to_delete = true;
      time_to_live = 0;
      /// and calling virtual function can process without crashing
      return;
    }
    time_to_live -= elapsed_time;
  }
  int i;
  float elapsed_seconds = ((float)elapsed_time) / 1000.0;
  if(change_color)
  {
    csColor change = colorpersecond;
    change *= elapsed_seconds;
    for(i=0; i<num_particles; i++)
      part_2d[i]->AddColor(change);
    if(num_particles > 0 && !part_2d[0]->HasLighting())
      SetLighting(false); // to copy color_init towards color in sprite2d.
  }
  if(change_size)
  {
    float sizefactor = pow(scalepersecond, elapsed_seconds);
    for(i=0; i<num_particles; i++)
      part_2d[i]->ScaleBy(sizefactor);
  }
  if(change_alpha)
  {
    float alpnow = 
      (part_2d[0]->GetMixmode() & CS_FX_MASK_ALPHA) / (float)CS_FX_MASK_ALPHA + 
      alphapersecond * elapsed_seconds;
    if(alpnow < 0.0f) alpnow = 0.0f;
    else if(alpnow > 1.0f) alpnow = 1.0f;
    SetMixmodes(CS_FX_SETALPHA(alpnow));
  }
  if(change_rotation)
  {
    float angle = anglepersecond * elapsed_seconds;
    for(i=0; i<num_particles; i++)
      part_2d[i]->Rotate(angle);
  }
}


//-- csNewtonianParticleSystem ------------------------------------------

csNewtonianParticleSystem :: csNewtonianParticleSystem(int max)
  : csParticleSystem(max)
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
  for(int i=0; i<num_particles; i++)
  {
    // notice that the ordering of the lines (1) and (2) makes the
    // resulting newpos = a*dt^2 + v*dt + oldposition (i.e. paraboloid).
    part_speed[i] += part_accel[i] * delta_t; // (1)
    move = part_speed[i] * delta_t; // (2)
    part_2d[i]->MovePosition (move); 
  }
}


//-- csParSysExplosion --------------------------------------------------


/// helping func. Returns vector of with -1..+1 members. Varying length!
static csVector3& GetRandomDirection()
{
  static csVector3 dir;
  dir.x = 2.0 * rand() / (1.0+RAND_MAX) - 1.0;
  dir.y = 2.0 * rand() / (1.0+RAND_MAX) - 1.0;
  dir.z = 2.0 * rand() / (1.0+RAND_MAX) - 1.0;
  return dir;
}


csParSysExplosion :: csParSysExplosion(int number_p, 
    const csVector3& explode_center, const csVector3& push, 
    csTextureHandle *txt, int nr_sides, float part_radius,
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
  scale_sprites = false;
  /// add particles
  for(i=0; i<number_p; i++)
  {
    //AppendRectSprite(0.25, 0.25, txt);
    AppendRegularSprite(nr_sides, part_radius, txt);
    pos = center + GetRandomDirection() * spread_pos;
    part_2d[i]->SetPosition (pos);
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
  int i;
  csNewtonianParticleSystem::Update(elapsed_time);

  // size of particles is exponentially reduced in fade time.
  if(scale_sprites && self_destruct && time_to_live < sprites_fade )
  {
    float scaleamt = 1.0 - (sprites_fade - time_to_live)/((float)sprites_fade);
    for(i=0; i<num_particles; i++)
      part_2d[i]->ScaleBy(scaleamt);
  }
  if(!has_light) return;
  csColor newcol;
  newcol.red =   1.0 - 0.3*sin(time_to_live/10. + center.x);
  newcol.green = 1.0 - 0.3*sin(time_to_live/15. + center.y);
  newcol.blue =  0.3 + 0.3*sin(time_to_live/10. + center.z);
  if(self_destruct && time_to_live < light_fade)
  {
    float fade_amt = 1.0 - (light_fade - time_to_live)/((float)light_fade);
    newcol *= fade_amt;
  }
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
