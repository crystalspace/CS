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
#include "iengine/sector.h"
#include "iengine/engine.h"
#include "csqsqrt.h"
#include <math.h>
#include <stdlib.h>

CS_IMPLEMENT_PLUGIN

SCF_IMPLEMENT_IBASE_EXT (csFireMeshObject)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iFireState)
SCF_IMPLEMENT_IBASE_EXT_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csFireMeshObject::FireState)
  SCF_IMPLEMENTS_INTERFACE (iFireState)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

// Aging ratios
#define COL_AGE0	0.0f
#define COL_AGE1	0.05f
#define COL_AGE2	0.2f
#define COL_AGE3	0.5f
#define COL_AGE4	1.0f
// COL_DAGE(x) = 1.0 / (COL_AGE(x) - COL_AGE(x-1))
#define COL_DAGE0	0.0f
#define COL_DAGE1	(1.0f / (COL_AGE1-COL_AGE0))
#define COL_DAGE2	(1.0f / (COL_AGE2-COL_AGE1))
#define COL_DAGE3	(1.0f / (COL_AGE3-COL_AGE2))
#define COL_DAGE4	(1.0f / (COL_AGE4-COL_AGE3))

csFireMeshObject::ColorInfo* csFireMeshObject::Colors = 0;

void csFireMeshObject::SetupColors()
{
  if (Colors == 0)
  {
    static ColorInfo c[MAX_COLORS];
    c[0].c.Set(1.0f,1.0f,1.0f); c[0].age = COL_AGE0; c[0].dage = COL_DAGE0;
    c[1].c.Set(1.0f,1.0f,0.0f); c[1].age = COL_AGE1; c[1].dage = COL_DAGE1;
    c[2].c.Set(1.0f,0.0f,0.0f); c[2].age = COL_AGE2; c[2].dage = COL_DAGE2;
    c[3].c.Set(0.6f,0.6f,0.6f); c[3].age = COL_AGE3; c[3].dage = COL_DAGE3;
    c[4].c.Set(0.1f,0.1f,0.1f); c[4].age = COL_AGE4; c[4].dage = COL_DAGE4;
    Colors = c;
  }
}

void csFireMeshObject::SetupObject ()
{
  if (!initialized)
  {
    csParticleSystem::SetupObject ();
    initialized = true;
    RemoveParticles ();
    delete[] part_pos;
    delete[] part_speed;
    delete[] part_age;

    part_pos = new csVector3[number];
    part_speed = new csVector3[number];
    part_age = new float[number];
    amt = (int)number;

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
    radius = csQsqrt (a*a + a*a);

    // create particles
    int i;
    for (i=0 ; i < (int)number ; i++)
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

csFireMeshObject::csFireMeshObject (iObjectRegistry* object_reg,
  iMeshObjectFactory* factory) : csParticleSystem (object_reg, factory)
{
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiFireState);
  part_pos = 0;
  part_speed = 0;
  part_age = 0;
  direction.Set (0, 1, 0);
  origin.Set (0,0,0, 0,0,0);
  total_time = 1;
  inv_total_time = 1. / total_time;
  swirl = 1;
  color_scale = 1;
  number = 40;
  delete_light = false;
  light_engine = 0;
  precalc_valid = false;
  SetupColors ();
}

csFireMeshObject::~csFireMeshObject()
{
  if (dynlight && delete_light)
  {
    light_engine->RemoveLight (dynlight);
  }
  delete[] part_pos;
  delete[] part_speed;
  delete[] part_age;
  SCF_DESTRUCT_EMBEDDED_IBASE (scfiFireState);
}

void csFireMeshObject::SetControlledLight (iLight *l)
{
  dynlight = l;
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
  int k;

  if (!precalc_valid)
  {
    precalc_valid = true;
    ColorInfo const* prev_info = GetColorInfo(0);
    for (k = 1 ; k < MAX_COLORS ; k++)
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

void csFireMeshObject::Update (csTicks elapsed_time)
{
  SetupObject ();
  csParticleSystem::Update (elapsed_time);
  if (dynlight)
  {
    light_time += elapsed_time;
    csColor newcol;
    newcol.red =   1.0 - 0.3*sin(light_time/10. + origin.Min().x);
    newcol.green = 0.7 - 0.3*sin(light_time/15. + origin.Min().y);
    newcol.blue =  0.3 + 0.3*sin(light_time/10. + origin.Min().z);
    dynlight->SetColor (newcol);
  }

  float delta_t = elapsed_time / 1000.0f; // in seconds
  // move particles;
  int i;
  for (i=0 ; i < (int)particles.Length () ; i++)
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
  if (dynlight) return;
  dynlight = engine->CreateLight ("", origin.GetCenter(), 5, csColor (1, 1, 0),
  	CS_LIGHT_DYNAMICTYPE_DYNAMIC);
  sec->GetLights ()->Add (dynlight);
  dynlight->Setup ();
  // @@@ BUG!
  dynlight->Setup ();
  delete_light = true;
  light_engine = engine;
}

void csFireMeshObject::HardTransform (const csReversibleTransform& t)
{
  origin.Set( t.This2Other (origin.Min()),  t.This2Other (origin.Max()));
  initialized = false;
  scfiObjectModel.ShapeChanged ();
}

//----------------------------------------------------------------------

SCF_IMPLEMENT_IBASE (csFireMeshObjectFactory)
  SCF_IMPLEMENTS_INTERFACE (iMeshObjectFactory)
SCF_IMPLEMENT_IBASE_END

csFireMeshObjectFactory::csFireMeshObjectFactory(iMeshObjectType* b, iObjectRegistry* s)
{
  SCF_CONSTRUCT_IBASE (b);
  logparent = 0;
  fire_type= b;
  object_reg = s;
}

csFireMeshObjectFactory::~csFireMeshObjectFactory ()
{
  SCF_DESTRUCT_IBASE ();
}

csPtr<iMeshObject> csFireMeshObjectFactory::NewInstance ()
{
  csFireMeshObject* cm =
    new csFireMeshObject (object_reg, (iMeshObjectFactory*)this );
  csRef<iMeshObject> im (SCF_QUERY_INTERFACE (cm, iMeshObject));
  cm->DecRef ();
  return csPtr<iMeshObject> (im);
}

//----------------------------------------------------------------------

SCF_IMPLEMENT_IBASE (csFireMeshObjectType)
  SCF_IMPLEMENTS_INTERFACE (iMeshObjectType)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csFireMeshObjectType::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_FACTORY (csFireMeshObjectType)


csFireMeshObjectType::csFireMeshObjectType (iBase* pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
}

csFireMeshObjectType::~csFireMeshObjectType ()
{
  SCF_DESTRUCT_EMBEDDED_IBASE(scfiComponent);
  SCF_DESTRUCT_IBASE ();
}

csPtr<iMeshObjectFactory> csFireMeshObjectType::NewFactory ()
{
  csFireMeshObjectFactory* cm = new csFireMeshObjectFactory (this, object_reg);
  csRef<iMeshObjectFactory> ifact (
  	SCF_QUERY_INTERFACE (cm, iMeshObjectFactory));
  cm->DecRef ();
  return csPtr<iMeshObjectFactory> (ifact);
}

