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
#include "csengine/engine.h"
#include "csengine/material.h"
#include "csengine/polytmap.h"
#include "csengine/curve.h"
#include "csobject/objiter.h"
#include "csutil/csvector.h"
#include "itxtmgr.h"

//---------------------------------------------------------------------------

IMPLEMENT_IBASE (csRegion)
  IMPLEMENTS_INTERFACE (iRegion)
IMPLEMENT_IBASE_END

IMPLEMENT_CSOBJTYPE (csRegion,csObjectNoDel);

csRegion::csRegion (csEngine* engine) : csObjectNoDel ()
{
  CONSTRUCT_IBASE (NULL);
  csRegion::engine = engine;
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
      engine->RemoveCollection (o);
      copy[i] = NULL;
    }

  for (i = 0 ; i < copy.Length () ; i++)
    if (copy[i] && ((csObject*)copy[i])->GetType () >= csSprite::Type)
    {
      csSprite* o = (csSprite*)copy[i];
      engine->RemoveSprite (o);
      copy[i] = NULL;
    }

  // @@@ Should sprite templates be deleted if there are still sprites
  // (in other regions) using them? Maybe a ref counter. Also make
  // sure to ObjRelease when you don't delete a sprite template.
  for (i = 0 ; i < copy.Length () ; i++)
    if (copy[i] && ((csObject*)copy[i])->GetType () == csSpriteTemplate::Type)
    {
      csSpriteTemplate* o = (csSpriteTemplate*)copy[i];
      engine->sprite_templates.Delete (engine->sprite_templates.Find (o));
      copy[i] = NULL;
    }

  for (i = 0 ; i < copy.Length () ; i++)
    if (copy[i] && ((csObject*)copy[i])->GetType () == csCurveTemplate::Type)
    {
      csCurveTemplate* o = (csCurveTemplate*)copy[i];
      engine->curve_templates.Delete (engine->curve_templates.Find (o));
      copy[i] = NULL;
    }

  // First things and only then thing templates.
  for (i = 0 ; i < copy.Length () ; i++)
    if (copy[i] && ((csObject*)copy[i])->GetType () == csThing::Type)
    {
      csThing* o = (csThing*)copy[i];
      int idx = engine->thing_templates.Find (o);
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
      int idx = engine->thing_templates.Find (o);
      if (idx != -1)
      {
        engine->thing_templates.Delete (idx);
	copy[i] = NULL;
      }
    }

  for (i = 0 ; i < copy.Length () ; i++)
    if (copy[i] && ((csObject*)copy[i])->GetType () == csSector::Type)
    {
      csSector* o = (csSector*)copy[i];
      int idx = engine->sectors.Find (o);
      if (idx != -1)
        engine->sectors.Delete (idx);
      else
        delete o;
      copy[i] = NULL;
    }

  for (i = 0 ; i < copy.Length () ; i++)
    if (copy[i] && ((csObject*)copy[i])->GetType () == csMaterialWrapper::Type)
    {
      csMaterialWrapper* o = (csMaterialWrapper*)copy[i];
      int idx = engine->GetMaterials ()->Find (o);
      if (idx != -1)
        engine->GetMaterials ()->Delete (idx);
      else
        delete o;
      copy[i] = NULL;
    }

  for (i = 0 ; i < copy.Length () ; i++)
    if (copy[i] && ((csObject*)copy[i])->GetType () == csTextureWrapper::Type)
    {
      csTextureWrapper* o = (csTextureWrapper*)copy[i];
      int idx = engine->GetTextures ()->Find (o);
      if (idx != -1)
        engine->GetTextures ()->Delete (idx);
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
      int idx = engine->planes.Find (o);
      o->DecRef ();
      if (idx != -1)
        engine->planes[idx] = 0;
      copy[i] = NULL;
    }

#if defined(CS_DEBUG)
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

bool csRegion::PrepareTextures ()
{
  iTextureManager* txtmgr = csEngine::current_engine->G3D->GetTextureManager ();
  txtmgr->ResetPalette ();

  // First register all textures to the texture manager.
  {
    for (csObjIterator iter = GetIterator (csTextureWrapper::Type, false);
  	!iter.IsFinished () ; ++iter)
    {
      csTextureWrapper* csth = (csTextureWrapper*)iter.GetObj ();
      if (!csth->GetTextureHandle ())
        csth->Register (txtmgr);
    }
  }

  // Prepare all the textures.
  txtmgr->PrepareTextures ();

  // Then register all materials to the texture manager.
  {
    for (csObjIterator iter = GetIterator (csMaterialWrapper::Type, false);
  	!iter.IsFinished () ; ++iter)
    {
      csMaterialWrapper* csmh = (csMaterialWrapper*)iter.GetObj ();
      if (!csmh->GetMaterialHandle ())
        csmh->Register (txtmgr);
    }
  }

  // Prepare all the materials.
  txtmgr->PrepareMaterials ();
  return true;
}

bool csRegion::PrepareSectors ()
{
  for (csObjIterator iter = GetIterator (csSector::Type);
  	!iter.IsFinished () ; ++iter)
  {
    csSector* s = (csSector*)iter.GetObj ();
    s->Prepare (s);
  }
  return true;
}

bool csRegion::ShineLights ()
{
  engine->ShineLights (this);
  return true;
}

bool csRegion::Prepare ()
{
  if (!PrepareTextures ()) return false;
  if (!PrepareSectors ()) return false;
  if (!ShineLights ()) return false;
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

csObject* csRegion::FindObject (const char* iName, const csIdType& type,
      	bool derived)
{
  for (csObjIterator iter = GetIterator (type, derived);
  	!iter.IsFinished () ; ++iter)
  {
    csObject* o = iter.GetObj ();
    const char* on = o->GetName ();
    if (on && !strcmp (on, iName)) return o;
  }
  return NULL;
}

iSector* csRegion::FindSector (const char *iName)
{
  csSector* obj = (csSector*)FindObject (iName, csSector::Type, false);
  if (!obj) return NULL;
  return &obj->scfiSector;
}

iThing* csRegion::FindThing (const char *iName)
{
  for (csObjIterator iter = GetIterator (csThing::Type, false);
  	!iter.IsFinished () ; ++iter)
  {
    csThing* o = (csThing*)iter.GetObj ();
    if (!o->IsSky () && !o->IsTemplate ())
    {
      // Only if not a sky and not a template.
      const char* on = o->GetName ();
      if (on && !strcmp (on, iName)) return &o->scfiThing;
    }
  }
  return NULL;
}

iThing* csRegion::FindSky (const char *iName)
{
  for (csObjIterator iter = GetIterator (csThing::Type, false);
  	!iter.IsFinished () ; ++iter)
  {
    csThing* o = (csThing*)iter.GetObj ();
    if (o->IsSky () && !o->IsTemplate ())
    {
      // Only if a sky and not a template.
      const char* on = o->GetName ();
      if (on && !strcmp (on, iName)) return &o->scfiThing;
    }
  }
  return NULL;
}

iThing* csRegion::FindThingTemplate (const char *iName)
{
  for (csObjIterator iter = GetIterator (csThing::Type, false);
  	!iter.IsFinished () ; ++iter)
  {
    csThing* o = (csThing*)iter.GetObj ();
    if (o->IsTemplate ())
    {
      // Only if a template.
      const char* on = o->GetName ();
      if (on && !strcmp (on, iName)) return &o->scfiThing;
    }
  }
  return NULL;
}

iSprite* csRegion::FindSprite (const char *iName)
{
  csSprite* obj = (csSprite*)FindObject (iName, csSprite::Type, true);
  if (!obj) return NULL;
  return &obj->scfiSprite;
}

iSpriteTemplate* csRegion::FindSpriteTemplate (const char *iName)
{
  csSpriteTemplate* obj = (csSpriteTemplate*)FindObject (iName, csSpriteTemplate::Type, false);
  if (!obj) return NULL;
  return (iSpriteTemplate*)obj;
}

iTextureWrapper* csRegion::FindTexture (const char *iName)
{
  csObject* obj = FindObject (iName, csTextureWrapper::Type, false);
  if (!obj) return NULL;
  // Cast two times to avoid problems with multi-inherit.
  csTextureWrapper* wrapper = (csTextureWrapper*)obj;
  return (iTextureWrapper*)wrapper;
}

iMaterialWrapper* csRegion::FindMaterial (const char *iName)
{
  csObject* obj = FindObject (iName, csMaterialWrapper::Type, false);
  if (!obj) return NULL;
  // Cast two times to avoid problems with multi-inherit.
  csMaterialWrapper* wrapper = (csMaterialWrapper*)obj;
  return (iMaterialWrapper*)wrapper;
}

bool csRegion::IsInRegion (iObject* iobj)
{
  csObject* obj = (csObject*)iobj;
  csObject* objpar = obj->GetObjectParent ();
  return objpar == (csObject*)this;
}

