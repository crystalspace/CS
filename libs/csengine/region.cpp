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
#include "csutil/csvector.h"

//---------------------------------------------------------------------------

IMPLEMENT_IBASE (csRegion)
  IMPLEMENTS_INTERFACE (iRegion)
IMPLEMENT_IBASE_END

IMPLEMENT_CSOBJTYPE (csRegion,csObjectNoDel);

csRegion::csRegion (csWorld* world) : csObjectNoDel ()
{
  CONSTRUCT_IBASE (NULL);
  csRegion::world = world;
}

csRegion::~csRegion ()
{
  Clear ();
}

void csRegion::Clear ()
{
  csObject* obj = GetChild (csObject::Type, true);
  while (obj)
  {
    ObjRelease (obj);
    obj = GetChild (csObject::Type, true);
  }
}

void csRegion::DeleteAll ()
{
  // First we need to copy the objects to a csVector to avoid
  // messing up the iterator while we are deleting them.
  csVector copy;
  for (csObjIterator iter = GetIterator (csObject::Type, true);
  	!iter.IsFinished () ; ++iter)
  {
    csObject* o = iter.GetObj ();
    copy.Push (o);
  }

  // Now we iterate over all objects in the 'copy' vector and
  // delete them. This will release them as csObject children
  // from this region parent.
  // Note that we traverse the list again and again for every
  // object type since the order in which objects types are deleted
  // is important. i.e. we should first delete all sprites and things
  // and only then delete the sectors.
  int i;
  for (i = 0 ; i < copy.Length () ; i++)
    if (((csObject*)copy[i])->GetType () == csCollection::Type)
    {
      csCollection* o = (csCollection*)copy[i];
      world->RemoveCollection (o);
      copy[i] = NULL;
    }

  for (i = 0 ; i < copy.Length () ; i++)
    if (copy[i] && ((csObject*)copy[i])->GetType () >= csSprite::Type)
    {
      csSprite* o = (csSprite*)copy[i];
      world->RemoveSprite (o);
      copy[i] = NULL;
    }

  // @@@ Should sprite templates be deleted if there are still sprites
  // (in other regions) using them? Maybe a ref counter. Also make
  // sure to ObjRelease when you don't delete a sprite template.
  for (i = 0 ; i < copy.Length () ; i++)
    if (copy[i] && ((csObject*)copy[i])->GetType () == csSpriteTemplate::Type)
    {
      csSpriteTemplate* o = (csSpriteTemplate*)copy[i];
      world->sprite_templates.Delete (world->sprite_templates.Find (o));
      copy[i] = NULL;
    }

  for (i = 0 ; i < copy.Length () ; i++)
    if (copy[i] && ((csObject*)copy[i])->GetType () == csCurveTemplate::Type)
    {
      csCurveTemplate* o = (csCurveTemplate*)copy[i];
      world->curve_templates.Delete (world->curve_templates.Find (o));
      copy[i] = NULL;
    }

  // First things and only then thing templates.
  for (i = 0 ; i < copy.Length () ; i++)
    if (copy[i] && ((csObject*)copy[i])->GetType () == csThing::Type)
    {
      csThing* o = (csThing*)copy[i];
      int idx = world->thing_templates.Find (o);
      if (idx == -1)
      {
        delete o;
	copy[i] = NULL;
      }
    }
  // @@@ Should thing templates be deleted if there are still things
  // (in other regions) using them? Maybe a ref counter. Also make
  // sure to ObjRelease when you don't delete a thing template.
  for (i = 0 ; i < copy.Length () ; i++)
    if (copy[i] && ((csObject*)copy[i])->GetType () == csThing::Type)
    {
      csThing* o = (csThing*)copy[i];
      int idx = world->thing_templates.Find (o);
      if (idx != -1)
      {
        world->thing_templates.Delete (idx);
	copy[i] = NULL;
      }
    }

  for (i = 0 ; i < copy.Length () ; i++)
    if (copy[i] && ((csObject*)copy[i])->GetType () == csSector::Type)
    {
      csSector* o = (csSector*)copy[i];
      int idx = world->sectors.Find (o);
      if (idx != -1)
        world->sectors.Delete (idx);
      else
        delete o;
      copy[i] = NULL;
    }

  for (i = 0 ; i < copy.Length () ; i++)
    if (copy[i] && ((csObject*)copy[i])->GetType () == csMaterialWrapper::Type)
    {
      csMaterialWrapper* o = (csMaterialWrapper*)copy[i];
      int idx = world->GetMaterials ()->Find (o);
      if (idx != -1)
        world->GetMaterials ()->Delete (idx);
      else
        delete o;
      copy[i] = NULL;
    }

  for (i = 0 ; i < copy.Length () ; i++)
    if (copy[i] && ((csObject*)copy[i])->GetType () == csTextureWrapper::Type)
    {
      csTextureWrapper* o = (csTextureWrapper*)copy[i];
      int idx = world->GetTextures ()->Find (o);
      if (idx != -1)
        world->GetTextures ()->Delete (idx);
      else
        delete o;
      copy[i] = NULL;
    }

  for (i = 0 ; i < copy.Length () ; i++)
    if (copy[i] && ((csObject*)copy[i])->GetType () == csPolyTxtPlane::Type)
    {
      csPolyTxtPlane* o = (csPolyTxtPlane*)copy[i];
      // Do a release here because the plane may still be used by other
      // polygons not belonging to this sector and we want to be sure
      // to release it from this region.
      ObjRelease (o);
      int idx = world->planes.Find (o);
      o->DecRef ();
      if (idx != -1)
        world->planes[idx] = 0;
      copy[i] = NULL;
    }

#if CS_DEBUG
  // Sanity check (only in debug mode). There should be no more
  // non-NULL references in the copy array now.
  for (i = 0 ; i < copy.Length () ; i++)
    if (copy[i])
    {
      csObject* o = (csObject*)copy[i];
      CsPrintf (MSG_INTERNAL_ERROR, "\
There is still an object in the array after deleting region contents!\n\
Object name is '%s', object type is '%s'\n",
	o->GetName () ? o->GetName () : "<NoName>",
	o->GetType ().ID);
    }
#endif // CS_DEBUG
}

bool csRegion::Prepare ()
{
  for (csObjIterator iter = GetIterator (csSector::Type);
  	!iter.IsFinished () ; ++iter)
  {
    csSector* s = (csSector*)iter.GetObj ();
    s->Prepare (s);
  }
  return true;
}

void csRegion::AddToRegion (csObject* obj)
{
  ObjAdd (obj);
}

void csRegion::ReleaseFromRegion (csObject* obj)
{
  ObjRelease (obj);
}

