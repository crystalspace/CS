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
#include "snow.h"
#include "imater.h"
#include <math.h>
#include <stdlib.h>

IMPLEMENT_IBASE_EXT (csSnowMeshObject)
  IMPLEMENTS_EMBEDDED_INTERFACE (iSnowState)
IMPLEMENT_IBASE_EXT_END

IMPLEMENT_EMBEDDED_IBASE (csSnowMeshObject::SnowState)
  IMPLEMENTS_INTERFACE (iSnowState)
IMPLEMENT_EMBEDDED_IBASE_END

void csSnowMeshObject::SetupObject ()
{
  if (!initialized)
  {
    RemoveParticles ();
    initialized = true;
    delete[] part_pos;
    delete[] part_speed;

    part_pos = new csVector3[number];
    part_speed = new csVector3[number];
    bbox = rainbox;
    /// spread particles evenly through box
    csVector3 size = rainbox.Max () - rainbox.Min ();
    csVector3 pos;
    for (int i=0 ; i < number ; i++)
    {
      AppendRectSprite (drop_width, drop_height, mat, lighted_particles);
      GetParticle (i)->SetMixmode (MixMode);
      pos = GetRandomDirection (size, rainbox.Min ()) ;
      GetParticle (i)->SetPosition (pos);
      part_pos[i] = pos;
      part_speed[i] = 0.0;
    }
    SetupColor ();
    SetupMixMode ();
  }
}

csSnowMeshObject::csSnowMeshObject (iSystem* system)
	: csParticleSystem (system)
{
  CONSTRUCT_EMBEDDED_IBASE (scfiSnowState);
  initialized = false;
  part_pos = NULL;
  part_speed = NULL;
  rainbox.Set (csVector3 (0, 0, 0), csVector3 (1, 1, 1));
  swirl_amount = 1;
  rain_dir.Set (0, -1, 0);
  drop_width = drop_height = .1;
  lighted_particles = false;
  number = 50;
}

csSnowMeshObject::~csSnowMeshObject()
{
  delete[] part_pos;
  delete[] part_speed;
}


void csSnowMeshObject::Update (cs_time elapsed_time)
{
  SetupObject ();
  csParticleSystem::Update (elapsed_time);
  float delta_t = elapsed_time / 1000.0f; // in seconds
  // move particles;
  csVector3 move, pos;
  int i;
  for (i=0 ; i < particles.Length () ; i++)
  {
    move = rain_dir * delta_t;
    /// swirl a bit, for snow drifting in the wind...
    csVector3 swirl = GetRandomDirection () * swirl_amount;
    swirl.y = 0.0;
    part_speed[i] += swirl * delta_t;
    move += part_speed[i] * delta_t;
    part_pos[i] += move;
    GetParticle (i)->SetPosition (part_pos[i]); 
  }
  // check if particles are out of the box.
  for (i=0 ; i < particles.Length () ; i++)
  {
    if (!rainbox.In (part_pos[i]))
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
      while (toolow>height) toolow-=height;
      pos = GetRandomDirection (csVector3 (rainbox.MaxX() - rainbox.MinX(), 
        0.0f, rainbox.MaxZ() - rainbox.MinZ()), rainbox.Min() );
      pos.y = rainbox.MaxY() - toolow;
      if(pos.y < rainbox.MinY() || pos.y > rainbox.MaxY()) 
        pos.y = rainbox.MaxY() - height * ((float)rand() / (1.0 + RAND_MAX));
      GetParticle (i)->SetPosition (pos);
      part_pos[i] = pos;
      part_speed[i] = 0.0;
    }
  }
}

//----------------------------------------------------------------------

IMPLEMENT_IBASE (csSnowMeshObjectFactory)
  IMPLEMENTS_INTERFACE (iMeshObjectFactory)
IMPLEMENT_IBASE_END

csSnowMeshObjectFactory::csSnowMeshObjectFactory (iSystem* system)
{
  CONSTRUCT_IBASE (NULL);
  csSnowMeshObjectFactory::system = system;
}

csSnowMeshObjectFactory::~csSnowMeshObjectFactory ()
{
}

iMeshObject* csSnowMeshObjectFactory::NewInstance ()
{
  csSnowMeshObject* cm = new csSnowMeshObject (system);
  return QUERY_INTERFACE (cm, iMeshObject);
}

//----------------------------------------------------------------------

IMPLEMENT_IBASE (csSnowMeshObjectType)
  IMPLEMENTS_INTERFACE (iMeshObjectType)
  IMPLEMENTS_INTERFACE (iPlugIn)
IMPLEMENT_IBASE_END

IMPLEMENT_FACTORY (csSnowMeshObjectType)

EXPORT_CLASS_TABLE (snow)
  EXPORT_CLASS (csSnowMeshObjectType, "crystalspace.mesh.object.snow",
    "Crystal Space Snow Mesh Type")
EXPORT_CLASS_TABLE_END

csSnowMeshObjectType::csSnowMeshObjectType (iBase* pParent)
{
  CONSTRUCT_IBASE (pParent);
}

csSnowMeshObjectType::~csSnowMeshObjectType ()
{
}

bool csSnowMeshObjectType::Initialize (iSystem* system)
{
  csSnowMeshObjectType::system = system;
  return true;
}

iMeshObjectFactory* csSnowMeshObjectType::NewFactory ()
{
  csSnowMeshObjectFactory* cm = new csSnowMeshObjectFactory (system);
  return QUERY_INTERFACE (cm, iMeshObjectFactory);
}

