/*
    Copyright (C) 2000 by Jorrit Tyberghein

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
#include "qint.h"
#include "csengine/region.h"
#include "csengine/cssprite.h"
#include "csengine/cscoll.h"
#include "csengine/thing.h"
#include "csengine/texture.h"
#include "csengine/sector.h"
#include "csengine/world.h"
#include "csengine/polytmap.h"
#include "csengine/curve.h"
#include "csobject/objiter.h"

//---------------------------------------------------------------------------

IMPLEMENT_CSOBJTYPE (csRegion,csObjectNoDel);

csRegion::csRegion (csWorld* world) : csObjectNoDel ()
{
  csRegion::world = world;
}

csRegion::~csRegion ()
{
  Clear ();
}

void csRegion::Clear ()
{
}

void csRegion::DeleteAll ()
{
  {
    for (csObjIterator iter = GetIterator (csCollection::Type);
  	!iter.IsFinished () ; ++iter)
    {
      csCollection* o = (csCollection*)iter.GetObj ();
      ObjRelease (o);
      world->RemoveCollection (o);
    }
  }
  {
    for (csObjIterator iter = GetIterator (csSprite::Type, true);
  	!iter.IsFinished () ; ++iter)
    {
      csSprite* o = (csSprite*)iter.GetObj ();
      ObjRelease (o);
      world->RemoveSprite (o);
    }
  }
  {
    for (csObjIterator iter = GetIterator (csSpriteTemplate::Type);
  	!iter.IsFinished () ; ++iter)
    {
      csSpriteTemplate* o = (csSpriteTemplate*)iter.GetObj ();
      ObjRelease (o);
      world->sprite_templates.Delete (world->sprite_templates.Find (o));
    }
  }
  {
    for (csObjIterator iter = GetIterator (csCurveTemplate::Type);
  	!iter.IsFinished () ; ++iter)
    {
      csCurveTemplate* o = (csCurveTemplate*)iter.GetObj ();
      ObjRelease (o);
      world->curve_templates.Delete (world->curve_templates.Find (o));
    }
  }
  {
    for (csObjIterator iter = GetIterator (csThing::Type);
  	!iter.IsFinished () ; ++iter)
    {
      csThing* o = (csThing*)iter.GetObj ();
      ObjRelease (o);
      int idx = world->thing_templates.Find (o);
      if (idx != -1)
        world->thing_templates.Delete (idx);
      else
        delete o;
    }
  }
  {
    for (csObjIterator iter = GetIterator (csSector::Type);
  	!iter.IsFinished () ; ++iter)
    {
      csSector* o = (csSector*)iter.GetObj ();
      ObjRelease (o);
      int idx = world->sectors.Find (o);
      if (idx != -1)
        world->sectors.Delete (idx);
      else
        delete o;
    }
  }
  {
    for (csObjIterator iter = GetIterator (csMaterialWrapper::Type);
  	!iter.IsFinished () ; ++iter)
    {
      csMaterialWrapper* o = (csMaterialWrapper*)iter.GetObj ();
      ObjRelease (o);
      int idx = world->GetMaterials ()->Find (o);
      if (idx != -1)
        world->GetMaterials ()->Delete (idx);
      else
        delete o;
    }
  }
  {
    for (csObjIterator iter = GetIterator (csTextureWrapper::Type);
  	!iter.IsFinished () ; ++iter)
    {
      csTextureWrapper* o = (csTextureWrapper*)iter.GetObj ();
      ObjRelease (o);
      int idx = world->GetTextures ()->Find (o);
      if (idx != -1)
        world->GetTextures ()->Delete (idx);
      else
        delete o;
    }
  }
  {
    for (csObjIterator iter = GetIterator (csPolyTxtPlane::Type);
  	!iter.IsFinished () ; ++iter)
    {
      csPolyTxtPlane* o = (csPolyTxtPlane*)iter.GetObj ();
      ObjRelease (o);
      int idx = world->planes.Find (o);
      o->DecRef ();
      if (idx != -1)
        world->planes[idx] = 0;
    }
  }
}

bool csRegion::Prepare ()
{
  return true;
}

