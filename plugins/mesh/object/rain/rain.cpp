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
#include "rain.h"
#include "imater.h"
#include "qsqrt.h"
#include <math.h>
#include <stdlib.h>

IMPLEMENT_IBASE_EXT (csRainMeshObject)
  IMPLEMENTS_EMBEDDED_INTERFACE (iRainState)
IMPLEMENT_IBASE_EXT_END

IMPLEMENT_EMBEDDED_IBASE (csRainMeshObject::RainState)
  IMPLEMENTS_INTERFACE (iRainState)
IMPLEMENT_EMBEDDED_IBASE_END

void csRainMeshObject::SetupObject ()
{
  if (!initialized)
  {
    RemoveParticles ();
    initialized = true;
    delete[] part_pos;

    part_pos = new csVector3[number];
    bbox = rainbox;
    /// spread particles evenly through box
    csVector3 size = rainbox.Max () - rainbox.Min ();

    // Calculate the maximum radius.
    float max_size = size.x;
    if (size.y > max_size) max_size = size.y;
    if (size.z > max_size) max_size = size.z;
    float a = max_size/2.;
    radius = qsqrt (a*a + a*a);

    csVector3 pos;
    for (int i=0 ; i < number ; i++)
    {
      AppendRectSprite (drop_width, drop_height, mat, lighted_particles);
      GetParticle (i)->SetMixmode (MixMode);
      pos = GetRandomDirection (size, rainbox.Min ()) ;
      GetParticle (i)->SetPosition (pos);
      part_pos[i] = pos;
    }
    SetupColor ();
    SetupMixMode ();
  }
}

csRainMeshObject::csRainMeshObject (iSystem* system, iMeshObjectFactory* factory)
	: csParticleSystem (system, factory)
{
  CONSTRUCT_EMBEDDED_IBASE (scfiRainState);
  initialized = false;
  part_pos = NULL;
  rainbox.Set (csVector3 (0, 0, 0), csVector3 (1, 1, 1));
  rain_dir.Set (0, -1, 0);
  drop_width = drop_height = .1;
  lighted_particles = false;
  number = 50;
}

csRainMeshObject::~csRainMeshObject()
{
  delete[] part_pos;
}


void csRainMeshObject::Update (cs_time elapsed_time)
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
    part_pos[i] += move;
    GetParticle (i)->SetPosition (part_pos[i]); 
  }
  // check if particles are out of the box.
  for (i=0 ; i < particles.Length () ; i++)
  {
    if (!rainbox.In(part_pos[i]))
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
      pos = GetRandomDirection( csVector3 (rainbox.MaxX() - rainbox.MinX(), 
        0.0f, rainbox.MaxZ() - rainbox.MinZ()), rainbox.Min() );
      pos.y = rainbox.MaxY() - toolow;
      if(pos.y < rainbox.MinY() || pos.y > rainbox.MaxY()) 
        pos.y = rainbox.MaxY() - height * ((float)rand() / (1.0 + RAND_MAX));
      GetParticle (i)->SetPosition (pos);
      part_pos[i] = pos;
    }
  }
}

void csRainMeshObject::HardTransform (const csReversibleTransform& /*t*/)
{
}

//----------------------------------------------------------------------

IMPLEMENT_IBASE (csRainMeshObjectFactory)
  IMPLEMENTS_INTERFACE (iMeshObjectFactory)
IMPLEMENT_IBASE_END

csRainMeshObjectFactory::csRainMeshObjectFactory (iSystem* system)
{
  CONSTRUCT_IBASE (NULL);
  csRainMeshObjectFactory::system = system;
}

csRainMeshObjectFactory::~csRainMeshObjectFactory ()
{
}

iMeshObject* csRainMeshObjectFactory::NewInstance ()
{
  csRainMeshObject* cm = new csRainMeshObject (system,
  	QUERY_INTERFACE (this, iMeshObjectFactory));
  return QUERY_INTERFACE (cm, iMeshObject);
}

//----------------------------------------------------------------------

IMPLEMENT_IBASE (csRainMeshObjectType)
  IMPLEMENTS_INTERFACE (iMeshObjectType)
  IMPLEMENTS_INTERFACE (iPlugIn)
IMPLEMENT_IBASE_END

IMPLEMENT_FACTORY (csRainMeshObjectType)

EXPORT_CLASS_TABLE (rain)
  EXPORT_CLASS (csRainMeshObjectType, "crystalspace.mesh.object.rain",
    "Crystal Space Rain Mesh Type")
EXPORT_CLASS_TABLE_END

csRainMeshObjectType::csRainMeshObjectType (iBase* pParent)
{
  CONSTRUCT_IBASE (pParent);
}

csRainMeshObjectType::~csRainMeshObjectType ()
{
}

bool csRainMeshObjectType::Initialize (iSystem* system)
{
  csRainMeshObjectType::system = system;
  return true;
}

iMeshObjectFactory* csRainMeshObjectType::NewFactory ()
{
  csRainMeshObjectFactory* cm = new csRainMeshObjectFactory (system);
  return QUERY_INTERFACE (cm, iMeshObjectFactory);
}

