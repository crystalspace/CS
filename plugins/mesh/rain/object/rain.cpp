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
#include "iengine/sector.h"
#include "csgeom/transfrm.h"
#include "rain.h"
#include "ivideo/material.h"
#include "iengine/material.h"
#include "iengine/mesh.h"
#include "iengine/movable.h"
#include "csqsqrt.h"
#include <math.h>
#include <stdlib.h>

CS_IMPLEMENT_PLUGIN

SCF_IMPLEMENT_IBASE_EXT (csRainMeshObject)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iRainState)
SCF_IMPLEMENT_IBASE_EXT_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csRainMeshObject::RainState)
  SCF_IMPLEMENTS_INTERFACE (iRainState)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

csRainMeshObject::csRainMeshObject (iEngine *engine, iMeshObjectFactory *fact)
  : csNewParticleSystem (engine, fact,
    CS_PARTICLE_SCALE | CS_PARTICLE_AXIS | CS_PARTICLE_ALIGN_Y)
{
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiRainState);
  MixMode = CS_FX_ADD;
  Axis = csVector3 (0, 1, 0);
  Speed = csVector3 (0, -1, 0);
  SetBox (csVector3 (-1, -1, -1), csVector3 (1, 1, 1));
}

csRainMeshObject::~csRainMeshObject ()
{
  SCF_DESTRUCT_EMBEDDED_IBASE (scfiRainState);
}

void csRainMeshObject::Spread (int first, int limit)
{
  csVector3 min = Bounds.Min ();
  csVector3 size = Bounds.Max () - min;

  for (int i=first; i<limit; i++)
  {
    float x = ((float)rand() / (1.0 + RAND_MAX));
    float y = ((float)rand() / (1.0 + RAND_MAX));
    float z = ((float)rand() / (1.0 + RAND_MAX));
    PositionArray [i] = csVector3 (
      min.x + x * size.x,
      min.y + y * size.y,
      min.z + z * size.z);
  }
}

void csRainMeshObject::Update (csTicks elapsed_time)
{
  csNewParticleSystem::Update (elapsed_time);
  for (int i=0; i<ParticleCount; i++)
  {
    csVector3& v = PositionArray[i];
    v += Speed * (float)elapsed_time / 1000.0f;
    while (!Bounds.In (v))
    {
      // Can't simply place it randomly (the user would notice that). Also
      // can't simply put it somewhere at the top of the box since the
      // falling direction might not be straight downwards and so part of
      // the box would never get get if we did.
      if (v.x < Bounds.MinX ()) v.x += Bounds.MaxX () - Bounds.MinX ();
      if (v.y < Bounds.MinY ()) v.y += Bounds.MaxY () - Bounds.MinY ();
      if (v.z < Bounds.MinZ ()) v.z += Bounds.MaxZ () - Bounds.MinZ ();
      if (v.x > Bounds.MaxX ()) v.x += Bounds.MinX () - Bounds.MaxX ();
      if (v.y > Bounds.MaxY ()) v.y += Bounds.MinY () - Bounds.MaxY ();
      if (v.z > Bounds.MaxZ ()) v.z += Bounds.MinZ () - Bounds.MaxZ ();
    }
  }
}

void csRainMeshObject::SetParticleCount (int num)
{
  int old = ParticleCount;
  SetCount (num);
  if (num > old) Spread (old, num);
}

void csRainMeshObject::SetDropSize (float dropwidth, float dropheight)
{
  Scale = csVector2 (dropwidth, dropheight);
}

void csRainMeshObject::SetBox (const csVector3& minbox, const csVector3& maxbox)
{
  Bounds.Set (minbox.x, minbox.y, minbox.z, 
              maxbox.x, maxbox.y, maxbox.z);
  Spread (0, ParticleCount);
}

void csRainMeshObject::SetFallSpeed (const csVector3& s)
{
  Speed = s;
  Axis = s.Unit ();
}

int csRainMeshObject::GetParticleCount () const
{
  return ParticleCount;
}

void csRainMeshObject::GetDropSize (float& dropwidth, float& dropheight) const
{
  dropwidth = Scale.x;
  dropwidth = Scale.y;
}

void csRainMeshObject::GetBox (csVector3& minbox, csVector3& maxbox) const
{
  minbox = Bounds.Min ();
  maxbox = Bounds.Max ();
}

const csVector3& csRainMeshObject::GetFallSpeed () const
{
  return Speed;
}

void csRainMeshObject::SetCollisionDetection (bool cd)
{
}

bool csRainMeshObject::GetCollisionDetection () const
{
  return false;
}

CS_DECLARE_SIMPLE_MESH_FACTORY (csRainFactory, csRainMeshObject)
CS_DECLARE_SIMPLE_MESH_PLUGIN (csRainPlugin, csRainFactory)

SCF_IMPLEMENT_FACTORY (csRainPlugin)

