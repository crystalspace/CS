/*
    Copyright (C) 1998-2000 by Jorrit Tyberghein

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
#include "csengine/pol2d.h"
#include "csengine/cssprite.h"
#include "csengine/light.h"
#include "csengine/camera.h"
#include "csengine/engine.h"
#include "csengine/texture.h"
#include "csengine/sector.h"
#include "csengine/bspbbox.h"
#include "csengine/dumper.h"
#include "csgeom/polyclip.h"
#include "csutil/garray.h"
#include "csutil/rng.h"
#include "igraph3d.h"
#include "qsqrt.h"


//=============================================================================

IMPLEMENT_CSOBJTYPE (csSprite, csPObject)

csSprite::csSprite (csObject* theParent) : csPObject ()
{
  movable.scfParent = this;
  MixMode = CS_FX_COPY;
  defered_num_lights = 0;
  defered_lighting_flags = 0;
  is_visible = false;
  camera_cookie = 0;
  ptree_obj = NULL;
  myOwner = NULL;
  parent = theParent;
  movable.SetObject (this);
  if (parent->GetType () >= csSprite::Type)
  {
    csSprite* sparent = (csSprite*)parent;
    movable.SetParent (&sparent->GetMovable ());
  }
  csEngine::current_engine->AddToCurrentRegion (this);
}

csSprite::~csSprite ()
{
  if (parent->GetType () == csEngine::Type)
  {
    csEngine* engine = (csEngine*)parent;
    engine->UnlinkSprite (this);
  }
}

void csSprite::UpdateMove ()
{
  UpdateInPolygonTrees ();
}

void csSprite::MoveToSector (csSector* s)
{
  s->sprites.Push (this);
}

void csSprite::RemoveFromSectors ()
{
  if (GetPolyTreeObject ())
    GetPolyTreeObject ()->RemoveFromTree ();
  if (parent->GetType () != csEngine::Type) return;
  int i;
  csVector& sectors = movable.GetSectors ();
  for (i = 0 ; i < sectors.Length () ; i++)
  {
    csSector* ss = (csSector*)sectors[i];
    if (ss)
    {
      int idx = ss->sprites.Find (this);
      if (idx >= 0)
      {
        ss->sprites[idx] = NULL;
        ss->sprites.Delete (idx);
      }
    }
  }
}

/// The list of lights that hit the sprite
static DECLARE_GROWING_ARRAY_REF (light_worktable, csLight*);

void csSprite::UpdateDeferedLighting (const csVector3& pos)
{
  if (defered_num_lights)
  {
    if (defered_num_lights > light_worktable.Limit ())
      light_worktable.SetLimit (defered_num_lights);

    csSector* sect = movable.GetSector (0);
    int num_lights = csEngine::current_engine->GetNearbyLights (sect,
      pos, defered_lighting_flags,
      light_worktable.GetArray (), defered_num_lights);
    UpdateLighting (light_worktable.GetArray (), num_lights);
  }
}

void csSprite::DeferUpdateLighting (int flags, int num_lights)
{
  defered_num_lights = num_lights;
  defered_lighting_flags = flags;
}

bool csSprite::HitBeam (const csVector3& start, const csVector3& end,
	csVector3& isect, float* pr)
{
  const csReversibleTransform& trans = movable.GetTransform ();
  csVector3 startObj = trans * start;
  csVector3 endObj = trans * end;
  bool rc = HitBeamObject (startObj, endObj, isect, pr);
  if (rc)
    isect = trans.This2Other (isect);
  return rc;
}

void csSprite::ScaleBy (float factor)
{
  csMatrix3 trans = movable.GetTransform ().GetT2O ();
  trans.m11 *= factor;
  trans.m22 *= factor;
  trans.m33 *= factor;
  movable.SetTransform (trans);
  UpdateMove ();
}


void csSprite::Rotate (float angle)
{
  csZRotMatrix3 rotz(angle);
  movable.Transform (rotz);
  csXRotMatrix3 rotx(angle);
  movable.Transform (rotx);
  UpdateMove ();
}

