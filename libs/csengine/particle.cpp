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
#include "csgeom/matrix3.h"
#include "csgeom/fastsqrt.h"
#include <math.h>
#include <stdlib.h>

IMPLEMENT_CSOBJTYPE (csParticleSystem, csSprite)
IMPLEMENT_CSOBJTYPE (csNewtonianParticleSystem, csParticleSystem)
IMPLEMENT_CSOBJTYPE (csSpiralParticleSystem, csNewtonianParticleSystem)
IMPLEMENT_CSOBJTYPE (csParSysExplosion, csNewtonianParticleSystem)
IMPLEMENT_CSOBJTYPE (csRainParticleSystem, csParticleSystem)
IMPLEMENT_CSOBJTYPE (csSnowParticleSystem, csParticleSystem)
IMPLEMENT_CSOBJTYPE (csFountainParticleSystem, csParticleSystem)
IMPLEMENT_CSOBJTYPE (csFireParticleSystem, csParticleSystem)


csParticleSystem :: csParticleSystem(csObject* theParent)
  : csSprite (theParent), ptree_bbox (NULL)
{
  CONSTRUCT_IBASE (NULL);
  ptree_bbox.SetOwner (this);
  ptree_obj = &ptree_bbox;
  particles.SetLength(0);
  self_destruct = false;
  time_to_live = 0;
  to_delete = false;
  // add me to the engine
  csEngine::current_engine->sprites.Push (this);
  // defaults
  change_size = false;
  change_color = false;
  change_alpha = false;
  change_rotation = false;
  // bbox is empty.
}


csParticleSystem :: ~csParticleSystem()
{
  // delete all my particles
  for(int i=0; i<particles.Length(); i++)
    if(particles[i])
      GetParticle(i)->DecRef();
}

void csParticleSystem::UpdateInPolygonTrees ()
{
  ptree_bbox.RemoveFromTree ();

  // If we are not in a sector which has a polygon tree
  // then we don't really update. We should consider if this is
  // a good idea. Do we only want this object updated when we
  // want to use it in a polygon tree? It is certainly more
  // efficient to do it this way when the sprite is currently
  // moving in normal convex sectors.
  int i;
  csPolygonTree* tree = NULL;
  csVector& sects = movable.GetSectors ();
  for (i = 0 ; i < sects.Length () ; i++)
  {
    tree = ((csSector*)sects[i])->GetStaticTree ();
    if (tree) break;
  }
  if (!tree) return;

  // The bounding box for a particle system is world space.
  csBox3 b = GetBoundingBox ();
  ptree_bbox.Update (b, this);

  // Here we need to insert in trees where this sprite lives.
  for (i = 0 ; i < sects.Length () ; i++)
  {
    tree = ((csSector*)sects[i])->GetStaticTree ();
    if (tree)
    {
      // Temporarily increase reference to prevent free.
      ptree_bbox.GetBaseStub ()->IncRef ();
      tree->AddObject (&ptree_bbox);
      ptree_bbox.GetBaseStub ()->DecRef ();
    }
  }
}


void csParticleSystem :: AppendRectSprite(float width, float height, 
  csMaterialWrapper *mat, bool lighted)
{
  csSprite2D *part = new csSprite2D(this);
  iParticle *pTicle;
  //csEngine::current_engine->sprites.Push(part);
  csColoredVertices& vs = part->GetVertices();
  vs.SetLimit(4);
  vs.SetLength(4);
  vs[0].pos.Set(-width,-height); vs[0].u=0.; vs[0].v=1.;
  vs[1].pos.Set(-width,+height); vs[1].u=0.; vs[1].v=0.;
  vs[2].pos.Set(+width,+height); vs[2].u=1.; vs[2].v=0.;
  vs[3].pos.Set(+width,-height); vs[3].u=1.; vs[3].v=1.;
  part->SetLighting( lighted );
  part->SetColor( csColor(1.0, 1.0, 1.0) );
  part->SetMaterial (mat);
  AppendParticle(pTicle = QUERY_INTERFACE(part, iParticle));
  pTicle->DecRef ();
  part->DecRef(); 
  
}


void csParticleSystem :: AppendRegularSprite(int n, float radius, 
  csMaterialWrapper* mat, bool lighted)
{
  csSprite2D *part = new csSprite2D(this);
  iParticle *pTicle;
  //csEngine::current_engine->sprites.Push(part);
  part->CreateRegularVertices(n, true);
  part->ScaleBy(radius);
  part->SetMaterial (mat);
  part->SetLighting( lighted );
  part->SetColor( csColor(1.0, 1.0, 1.0) );
  AppendParticle(pTicle = QUERY_INTERFACE(part, iParticle));
  pTicle->DecRef ();
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
  //for(int i = 0; i<particles.Length(); i++)
    //GetParticle(i)->MoveToSector(sector);
  csSprite::MoveToSector (sector);
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


void csParticleSystem :: Update(cs_time elapsed_time)
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

void csParticleSystem::Draw (csRenderView& rview)
{
  for (int i = 0; i<particles.Length(); i++)
    GetParticle(i)->Draw (rview);
}

void csParticleSystem::UpdateLighting (csLight** lights, int num_lights)
{
  defered_num_lights = 0;
  for (int i = 0; i<particles.Length(); i++)
    GetParticle(i)->UpdateLighting (lights, num_lights);
}

void csParticleSystem::DeferUpdateLighting (int flags, int num_lights)
{
  csSprite::DeferUpdateLighting (flags, num_lights);
  for (int i = 0; i<particles.Length(); i++)
    GetParticle(i)->DeferUpdateLighting (flags, num_lights);
}

const csVector3& csParticleSystem::GetPosition () const
{
  // @@@ Can we return anything useful here?
  static csVector3 v (0);
  return v;
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

//-- csNewtonianParticleSystem ------------------------------------------

csNewtonianParticleSystem :: csNewtonianParticleSystem(csObject* theParent, int max)
  : csParticleSystem(theParent)
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


void csNewtonianParticleSystem :: Update(cs_time elapsed_time)
{
  csVector3 move;
  csParticleSystem::Update(elapsed_time);
  // time passed; together with CS 1 unit = 1 meter makes units right.
  float delta_t = elapsed_time / 1000.0f; // in seconds
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


//-- csRainParticleSystem --------------------------------------------------

csRainParticleSystem :: csRainParticleSystem(csObject* theParent, int number, csMaterialWrapper* mat, 
  UInt mixmode, bool lighted_particles, float drop_width, float drop_height,
  const csVector3& rainbox_min, const csVector3& rainbox_max, 
  const csVector3& fall_speed)
  : csParticleSystem(theParent)
{
  part_pos = new csVector3[number];
  rain_dir = fall_speed;
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
  }
}

csRainParticleSystem :: ~csRainParticleSystem()
{
  delete[] part_pos;
}

void csRainParticleSystem :: Update(cs_time elapsed_time)
{
  csParticleSystem::Update(elapsed_time);
  float delta_t = elapsed_time / 1000.0f; // in seconds
  // move particles;
  csVector3 move, pos;
  int i;
  for(i=0; i<particles.Length(); i++)
  {
    move = rain_dir * delta_t;
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
    }
  }
}


//-- csSnowParticleSystem --------------------------------------------------

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


//-- csFountainParticleSystem --------------------------------------------------

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



//-- csFireParticleSystem --------------------------------------------------

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
