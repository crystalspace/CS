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
#include "fire.h"
#include "ivideo/material.h"
#include "iengine/material.h"
#include "iengine/light.h"
#include "iengine/dynlight.h"
#include "iengine/sector.h"
#include "iengine/engine.h"
#include "qsqrt.h"
#include <math.h>
#include <stdlib.h>

CS_IMPLEMENT_PLUGIN

IMPLEMENT_IBASE_EXT (csFireMeshObject)
  IMPLEMENTS_EMBEDDED_INTERFACE (iFireState)
IMPLEMENT_IBASE_EXT_END

IMPLEMENT_EMBEDDED_IBASE (csFireMeshObject::FireState)
  IMPLEMENTS_INTERFACE (iFireState)
IMPLEMENT_EMBEDDED_IBASE_END

// Aging ratios
#define COL_AGE0	0.0
#define COL_AGE1	0.05
#define COL_AGE2	0.2
#define COL_AGE3	0.5
#define COL_AGE4	1.0
// COL_DAGE(x) = 1.0 / (COL_AGE(x) - COL_AGE(x-1))
#define COL_DAGE0	0.0
#define COL_DAGE1	(1.0 / (COL_AGE1-COL_AGE0))
#define COL_DAGE2	(1.0 / (COL_AGE2-COL_AGE1))
#define COL_DAGE3	(1.0 / (COL_AGE3-COL_AGE2))
#define COL_DAGE4	(1.0 / (COL_AGE4-COL_AGE3))

csFireMeshObject::ColorInfo* csFireMeshObject::Colors = 0;

void csFireMeshObject::SetupColors()
{
  if (Colors == 0)
  {
    static ColorInfo c[MAX_COLORS];
    c[0].c.Set(1.0,1.0,1.0); c[0].age = COL_AGE0; c[0].dage = COL_DAGE0;
    c[1].c.Set(1.0,1.0,0.0); c[1].age = COL_AGE1; c[1].dage = COL_DAGE1;
    c[2].c.Set(1.0,0.0,0.0); c[2].age = COL_AGE2; c[2].dage = COL_DAGE2;
    c[3].c.Set(0.6,0.6,0.6); c[3].age = COL_AGE3; c[3].dage = COL_DAGE3;
    c[4].c.Set(0.1,0.1,0.1); c[4].age = COL_AGE4; c[4].dage = COL_DAGE4;
    Colors = c;
  }
}

void csFireMeshObject::SetupObject ()
{
  if (!initialized)
  {
    RemoveParticles ();
    initialized = true;
    delete[] part_pos;
    delete[] part_speed;
    delete[] part_age;

    part_pos = new csVector3[number];
    part_speed = new csVector3[number];
    part_age = new float[number];
    amt = number;

    float fradius = drop_width * swirl; // guessed radius of the fire
    csVector3 height = total_time * direction; // guessed height
    bbox.Set (origin.Min() - csVector3 (-fradius,0,-fradius), 
      origin.Max() + csVector3 (+fradius, 0, +fradius) + height );

    // Calculate the maximum radius.
    csVector3 size = bbox.Max () - bbox.Min ();
    float max_size = size.x;
    if (size.y > max_size) max_size = size.y;
    if (size.z > max_size) max_size = size.z;
    float a = max_size/2.;
    radius = qsqrt (a*a + a*a);

    // create particles
    for (int i=0 ; i < number ; i++)
    {
      AppendRectSprite (drop_width, drop_height, mat, lighted_particles);
      GetParticle (i)->SetMixMode (MixMode);
      RestartParticle (i, (total_time / float(number)) * float(number-i));
      bbox.AddBoundingVertexSmart (part_pos[i]);
    }
    time_left = 0.0;
    next_oldest = 0;
    light_time = (int) (3000.0 *rand() / (1.0 + RAND_MAX));
    SetupColor ();
    SetupMixMode ();
  }
}

csFireMeshObject::csFireMeshObject (iSystem* system,
  iMeshObjectFactory* factory) : csParticleSystem (system, factory)
{
  CONSTRUCT_EMBEDDED_IBASE (scfiFireState);
  part_pos = NULL;
  part_speed = NULL;
  part_age = NULL;
  direction.Set (0, 1, 0);
  origin.Set (0,0,0, 0,0,0);
  total_time = 1;
  inv_total_time = 1. / total_time;
  swirl = 1;
  color_scale = 1;
  number = 40;
  light = NULL;
  dynlight = NULL;
  delete_light = false;
  light_engine = NULL;
  precalc_valid = false;
  SetupColors ();
}

csFireMeshObject::~csFireMeshObject()
{
  if (dynlight && delete_light)
  {
    light_engine->RemoveDynLight (dynlight);
  }
  if (dynlight) dynlight->DecRef ();
  if (light) light->DecRef ();
  delete[] part_pos;
  delete[] part_speed;
  delete[] part_age;
}

void csFireMeshObject::SetControlledLight (iLight *l)
{
  light = l;
  if (dynlight) dynlight->DecRef ();
  dynlight = QUERY_INTERFACE_SAFE (light, iDynLight);
}

void csFireMeshObject::RestartParticle (int index, float pre_move)
{
  part_pos[index] = GetRandomPosition(origin);
  part_speed[index] = direction;
  part_age[index] = 0.0;
  GetParticle (index)->SetPosition (part_pos[index]);
  MoveAndAge (index, pre_move);
}


void csFireMeshObject::MoveAndAge (int i, float delta_t)
{
  csVector3 accel = GetRandomDirection () * swirl;
  part_speed[i] += accel * delta_t;
  part_pos[i] += part_speed[i] * delta_t;
  GetParticle (i)->SetPosition (part_pos[i]); 
  part_age[i] += delta_t;

  if (!precalc_valid)
  {
    precalc_valid = true;
    ColorInfo const* prev_info = GetColorInfo(0);
    for (int k = 1 ; k < MAX_COLORS ; k++)
    {
      ColorInfo const* info = GetColorInfo(k);
      precalc_add[k] = color_scale * (prev_info->c
      	+ prev_info->c * prev_info->age * info->dage
    	- info->c * prev_info->age * info->dage);
      precalc_mul[k] = color_scale * info->dage * (info->c - prev_info->c);
      prev_info = info;
    }
  }

  float age = part_age[i] * inv_total_time;
  int k;
  if (age < COL_AGE1)	   k = 1;
  else if (age < COL_AGE2) k = 2;
  else if (age < COL_AGE3) k = 3;
  else			   k = 4;

  /// colouring fraction
  csColor col = age * precalc_mul[k] + precalc_add[k];
  GetParticle (i)->SetColor (col);
}


int csFireMeshObject::FindOldest ()
{
  int ret = next_oldest;
  next_oldest = (next_oldest + 1 ) % amt;
  return ret;
}

void csFireMeshObject::Update (cs_time elapsed_time)
{
  SetupObject ();
  csParticleSystem::Update (elapsed_time);
  if (light)
  {
    light_time += elapsed_time;
    csColor newcol;
    newcol.red =   1.0 - 0.3*sin(light_time/10. + origin.Min().x);
    newcol.green = 0.7 - 0.3*sin(light_time/15. + origin.Min().y);
    newcol.blue =  0.3 + 0.3*sin(light_time/10. + origin.Min().z);
    light->SetColor (newcol);
  }

  float delta_t = elapsed_time / 1000.0f; // in seconds
  // move particles;
  int i;
  for (i=0 ; i < particles.Length () ; i++)
  {
    MoveAndAge (i, delta_t);
  }

  /// restart a number of particles
  float intersperse = total_time / (float)amt;
  float todo_time = delta_t + time_left;
  while (todo_time > intersperse)
  {
    RestartParticle (FindOldest (), todo_time);
    todo_time -= intersperse;
  }
  time_left = todo_time;
}


void csFireMeshObject::AddLight (iEngine *engine, iSector *sec)
{
  if (light) return;
  dynlight = engine->CreateDynLight (origin.GetCenter(), 5, csColor (1, 1, 0));
  light = QUERY_INTERFACE (dynlight, iLight);
  light->SetSector (sec);
  dynlight->Setup ();
  delete_light = true;
  light_engine = engine;
}

void csFireMeshObject::HardTransform (const csReversibleTransform& t)
{
  origin.Set( t.This2Other (origin.Min()),  t.This2Other (origin.Max()));
  initialized = false;
  shapenr++;
}

//----------------------------------------------------------------------

IMPLEMENT_IBASE (csFireMeshObjectFactory)
  IMPLEMENTS_INTERFACE (iMeshObjectFactory)
IMPLEMENT_IBASE_END

csFireMeshObjectFactory::csFireMeshObjectFactory(iBase* b, iSystem* s)
{
  CONSTRUCT_IBASE (b);
  system = s;
}

csFireMeshObjectFactory::~csFireMeshObjectFactory ()
{
}

iMeshObject* csFireMeshObjectFactory::NewInstance ()
{
  csFireMeshObject* cm =
    new csFireMeshObject (system, (iMeshObjectFactory*)this );
  iMeshObject* im = QUERY_INTERFACE (cm, iMeshObject);
  im->DecRef ();
  return im;
}

//----------------------------------------------------------------------

IMPLEMENT_IBASE (csFireMeshObjectType)
  IMPLEMENTS_INTERFACE (iMeshObjectType)
  IMPLEMENTS_INTERFACE (iPlugIn)
IMPLEMENT_IBASE_END

IMPLEMENT_FACTORY (csFireMeshObjectType)

EXPORT_CLASS_TABLE (fire)
  EXPORT_CLASS (csFireMeshObjectType, "crystalspace.mesh.object.fire",
    "Crystal Space Fire Mesh Type")
EXPORT_CLASS_TABLE_END

csFireMeshObjectType::csFireMeshObjectType (iBase* pParent)
{
  CONSTRUCT_IBASE (pParent);
}

csFireMeshObjectType::~csFireMeshObjectType ()
{
}

bool csFireMeshObjectType::Initialize (iSystem* system)
{
  csFireMeshObjectType::system = system;
  return true;
}

iMeshObjectFactory* csFireMeshObjectType::NewFactory ()
{
  csFireMeshObjectFactory* cm = new csFireMeshObjectFactory (this, system);
  iMeshObjectFactory* ifact = QUERY_INTERFACE (cm, iMeshObjectFactory);
  ifact->DecRef ();
  return ifact;
}
