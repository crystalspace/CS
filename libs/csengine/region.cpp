/*
    Copyright (C) 2000-2001 by Jorrit Tyberghein

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
#include "csengine/meshobj.h"
#include "csengine/cscoll.h"
#include "csengine/thing.h"
#include "csengine/texture.h"
#include "csengine/sector.h"
#include "csengine/engine.h"
#include "csengine/campos.h"
#include "csengine/material.h"
#include "csengine/polytmap.h"
#include "csengine/curve.h"
#include "csobject/objiter.h"
#include "csengine/terrobj.h"
#include "csutil/csvector.h"
#include "ivideo/txtmgr.h"

//---------------------------------------------------------------------------

IMPLEMENT_IBASE (csRegion)
  IMPLEMENTS_EMBEDDED_INTERFACE (iRegion)
IMPLEMENT_IBASE_END

IMPLEMENT_EMBEDDED_IBASE (csRegion::Region)
  IMPLEMENTS_INTERFACE (iRegion)
IMPLEMENT_EMBEDDED_IBASE_END

IMPLEMENT_CSOBJTYPE (csRegion,csObjectNoDel);

IMPLEMENT_OBJECT_INTERFACE (csRegion)
  IMPLEMENTS_EMBEDDED_OBJECT_TYPE (iRegion)
IMPLEMENT_OBJECT_INTERFACE_END

csRegion::csRegion (csEngine* e) : csObjectNoDel ()
{
  CONSTRUCT_IBASE (NULL);
  CONSTRUCT_EMBEDDED_IBASE (scfiRegion);
  engine = e;
}

csRegion::~csRegion ()
{
  scfiRegion.Clear ();
}

void csRegion::Region::Clear ()
{
  scfParent->ObjReleaseAll ();
}

void csRegion::Region::DeleteAll ()
{
  iObjectIterator *iter;

  // First we need to copy the objects to a csVector to avoid
  // messing up the iterator while we are deleting them.
  csVector copy;
  for (iter = scfParent->GetIterator ();
  	!iter->IsFinished () ; iter->Next ())
  {
    iObject* o = iter->GetObject ();
    copy.Push (o);
  }
  iter->DecRef ();

  // Now we iterate over all objects in the 'copy' vector and
  // delete them. This will release them as csObject children
  // from this region parent.
  // Note that we traverse the list again and again for every
  // object type since the order in which objects types are deleted
  // is important. i.e. we should first delete all meshes and things
  // and only then delete the sectors.
  int i;
  for (i = 0 ; i < copy.Length () ; i++)
    if (copy[i])
    {
      iObject* obj = (iObject*)copy[i];
      iCollection* o = QUERY_INTERFACE_FAST (obj, iCollection);
      if (!o) continue;
      scfParent->engine->RemoveCollection (((csCollection::Collection*)o)->scfParent);
      copy[i] = NULL;
//      o->DecRef (); @@@ Uncomment when the engine does a DecRef, not delete
    }

  for (i = 0 ; i < copy.Length () ; i++)
    if (copy[i])
    {
      iObject* obj = (iObject*)copy[i];
      iMeshWrapper* o = QUERY_INTERFACE_FAST (obj, iMeshWrapper);
      if (!o) continue;
      scfParent->engine->RemoveMesh (o->GetPrivateObject ());
      copy[i] = NULL;
//      o->DecRef (); @@@ Uncomment when the engine does a DecRef, not delete
    }

  // @@@ Should mesh factories be deleted if there are still mesh objects
  // (in other regions) using them? Maybe a ref counter. Also make
  // sure to ObjRelease when you don't delete a mesh factory.
  for (i = 0 ; i < copy.Length () ; i++)
    if (copy[i])
    {
      iObject* obj = (iObject*)copy[i];
      iMeshFactoryWrapper* o = QUERY_INTERFACE_FAST (obj, iMeshFactoryWrapper);
      if (!o) continue;
      scfParent->engine->mesh_factories.Delete (
        scfParent->engine->mesh_factories.Find (o->GetPrivateObject ()));
      copy[i] = NULL;
//      o->DecRef (); @@@ Uncomment when the engine does a DecRef, not delete
    }

  for (i = 0 ; i < copy.Length () ; i++)
    if (copy[i])
    {
      iObject* obj = (iObject*)copy[i];
      iCurveTemplate* o = QUERY_INTERFACE_FAST (obj, iCurveTemplate);
      if (!o) continue;
      scfParent->engine->curve_templates.Delete (
        scfParent->engine->curve_templates.Find (
	  ((csCurveTemplate::CurveTemplate*)o)->scfParent));
      copy[i] = NULL;
//      o->DecRef (); @@@ Uncomment when the engine does a DecRef, not delete
    }

  for (i = 0 ; i < copy.Length () ; i++)
    if (copy[i])
    {
      iObject* obj = (iObject*)copy[i];
      iSector* o = QUERY_INTERFACE_FAST (obj, iSector);
      if (!o) continue;
      int idx = scfParent->engine->sectors.Find (o->GetPrivateObject ());
      if (idx != -1)
        scfParent->engine->sectors.Delete (idx);
      else
        o->DecRef ();
      copy[i] = NULL;
//      o->DecRef (); @@@ Uncomment when the engine does a DecRef, not delete
    }

  for (i = 0 ; i < copy.Length () ; i++)
    if (copy[i])
    {
      iObject* obj = (iObject*)copy[i];
      iMaterialWrapper* o = QUERY_INTERFACE_FAST (obj, iMaterialWrapper);
      if (!o) continue;
      int idx = scfParent->engine->GetMaterials ()->Find (o->GetPrivateObject ());
      if (idx != -1)
        scfParent->engine->GetMaterials ()->Delete (idx);
      else
        o->DecRef ();
      copy[i] = NULL;
//      o->DecRef (); @@@ Uncomment when the engine does a DecRef, not delete
    }

  for (i = 0 ; i < copy.Length () ; i++)
    if (copy[i])
    {
      iObject* obj = (iObject*)copy[i];
      iTextureWrapper* o = QUERY_INTERFACE_FAST (obj, iTextureWrapper);
      if (!o) continue;
      int idx = scfParent->engine->GetTextures ()->Find (o->GetPrivateObject ());
      if (idx != -1)
        scfParent->engine->GetTextures ()->Delete (idx);
      else
        o->DecRef ();
      copy[i] = NULL;
//      o->DecRef (); @@@ Uncomment when the engine does a DecRef, not delete
    }

  for (i = 0 ; i < copy.Length () ; i++)
    if (copy[i])
    {
      iObject* obj = (iObject*)copy[i];
      iCameraPosition* o = QUERY_INTERFACE_FAST (obj, iCameraPosition);
      if (!o) continue;
      int idx = scfParent->engine->camera_positions.Find (
        ((csCameraPosition::CameraPosition*)o)->scfParent);
      if (idx != -1)
        scfParent->engine->camera_positions.Delete (idx);
      else
        o->DecRef ();
      copy[i] = NULL;
//      o->DecRef (); @@@ Uncomment when the engine does a DecRef, not delete
    }

  for (i = 0 ; i < copy.Length () ; i++)
    if (copy[i])
    {
      iObject* obj = (iObject*)copy[i];
      iPolyTxtPlane* o = QUERY_INTERFACE_FAST (obj, iPolyTxtPlane);
      if (!o) continue;
      // Do a release here because the plane may still be used by other
      // polygons not belonging to this sector and we want to be sure
      // to release it from this region.
      scfParent->ObjRelease (o->QueryObject ());
      int idx = scfParent->engine->planes.Find (o->GetPrivateObject ());
      o->DecRef ();
      if (idx != -1)
        scfParent->engine->planes[idx] = 0;
      copy[i] = NULL;
//      o->DecRef (); @@@ Uncomment when the engine does a DecRef, not delete
    }

#if defined(CS_DEBUG)
  // Sanity check (only in debug mode). There should be no more
  // non-NULL references in the copy array now.
  for (i = 0 ; i < copy.Length () ; i++)
    if (copy[i])
    {
      iObject* o = (iObject*)copy[i];
      CsPrintf (MSG_INTERNAL_ERROR, "\
There is still an object in the array after deleting region contents!\n\
Object name is '%s'\n",
	o->GetName () ? o->GetName () : "<NoName>");
    }
#endif // CS_DEBUG
}

bool csRegion::Region::PrepareTextures ()
{
  iObjectIterator *iter;
  iTextureManager* txtmgr = csEngine::current_engine->G3D->GetTextureManager();
  //txtmgr->ResetPalette ();

  // First register all textures to the texture manager.
  {
    for (iter = scfParent->GetIterator ();
  	!iter->IsFinished () ; iter->Next())
    {
      iTextureWrapper* csth =
        QUERY_INTERFACE_FAST (iter->GetObject (), iTextureWrapper);
      if (csth)
      {
        if (!csth->GetTextureHandle ())
          csth->Register (txtmgr);
        csth->DecRef ();
      }
    }
    iter->DecRef ();
  }

  // Prepare all the textures.
  //@@@ Only prepare new textures: txtmgr->PrepareTextures ();
  {
    for (iter = scfParent->GetIterator ();
  	!iter->IsFinished () ; iter->Next())
    {
      iTextureWrapper* csth =
        QUERY_INTERFACE_FAST (iter->GetObject (), iTextureWrapper);
      if (csth)
      {
        csth->GetTextureHandle ()->Prepare ();
        csth->DecRef ();
      }
    }
    iter->DecRef ();
  }

  // Then register all materials to the texture manager.
  {
    for (iter = scfParent->GetIterator ();
  	!iter->IsFinished () ; iter->Next())
    {
      iMaterialWrapper* csmh =
        QUERY_INTERFACE_FAST (iter->GetObject (), iMaterialWrapper);
      if (csmh)
      {
        if (!csmh->GetMaterialHandle ())
          csmh->Register (txtmgr);
        csmh->DecRef ();
      }
    }
    iter->DecRef ();
  }

  // Prepare all the materials.
  //@@@ Only prepare new materials: txtmgr->PrepareMaterials ();
  {
    for (iter = scfParent->GetIterator ();
  	!iter->IsFinished () ; iter->Next())
    {
      iMaterialWrapper* csmh =
        QUERY_INTERFACE_FAST (iter->GetObject (), iMaterialWrapper);
      if (csmh)
      {
        csmh->GetMaterialHandle ()->Prepare ();
        csmh->DecRef ();
      }
    }
    iter->DecRef ();
  }

  return true;
}

bool csRegion::Region::ShineLights ()
{
  scfParent->engine->ShineLights (scfParent);
  return true;
}

bool csRegion::Region::Prepare ()
{
  if (!PrepareTextures ()) return false;
  if (!ShineLights ()) return false;
  return true;
}

void csRegion::AddToRegion (iObject* obj)
{
  ObjAdd (obj);
}

void csRegion::ReleaseFromRegion (iObject* obj)
{
  ObjRelease (obj);
}

iObject* csRegion::FindObject (const char* iName, const csIdType& type,
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

iSector* csRegion::Region::FindSector (const char *iName)
{
  csSector* obj = GET_NAMED_CHILD_OBJECT(scfParent, csSector, iName);
  if (!obj) return NULL;
  return &obj->scfiSector;
}

iMeshWrapper* csRegion::Region::FindMeshObject (const char *iName)
{
  csMeshWrapper* obj =
    GET_NAMED_CHILD_OBJECT(scfParent, csMeshWrapper, iName);
  if (!obj) return NULL;
  return &obj->scfiMeshWrapper;
}

iMeshFactoryWrapper* csRegion::Region::FindMeshFactory (const char *iName)
{
  csMeshFactoryWrapper* obj =
    GET_NAMED_CHILD_OBJECT(scfParent, csMeshFactoryWrapper, iName);
  if (!obj) return NULL;
  return &obj->scfiMeshFactoryWrapper;
}

iTerrainWrapper* csRegion::Region::FindTerrainObject (const char *iName)
{
  csTerrainWrapper* obj =
    GET_NAMED_CHILD_OBJECT(scfParent, csTerrainWrapper, iName);
  if (!obj)
    return NULL;
  return &obj->scfiTerrainWrapper;
}

iTerrainFactoryWrapper* csRegion::Region::FindTerrainFactory (const char *iName)
{
  csTerrainFactoryWrapper* obj =
    GET_NAMED_CHILD_OBJECT(scfParent, csTerrainFactoryWrapper, iName);
  if (!obj)
    return NULL;
  return &obj->scfiTerrainFactoryWrapper;
}

iTextureWrapper* csRegion::Region::FindTexture (const char *iName)
{
  csTextureWrapper* obj =
    GET_NAMED_CHILD_OBJECT(scfParent, csTextureWrapper, iName);
  if (!obj) return NULL;
  return &obj->scfiTextureWrapper;
}

iMaterialWrapper* csRegion::Region::FindMaterial (const char *iName)
{
  csMaterialWrapper* obj =
    GET_NAMED_CHILD_OBJECT(scfParent, csMaterialWrapper, iName);
  if (!obj) return NULL;
  return &obj->scfiMaterialWrapper;
}

iCameraPosition* csRegion::Region::FindCameraPosition (const char *iName)
{
  csCameraPosition* obj =
    GET_NAMED_CHILD_OBJECT(scfParent, csCameraPosition, iName);
  if (!obj) return NULL;
  return &obj->scfiCameraPosition;
}

iCollection* csRegion::Region::FindCollection (const char *iName)
{
  csCollection* obj =
    GET_NAMED_CHILD_OBJECT(scfParent, csCollection, iName);
  if (!obj) return NULL;
  return &obj->scfiCollection;
}

bool csRegion::IsInRegion (iObject* iobj)
{
  return iobj->GetObjectParentI () == this;
}

bool csRegion::Region::IsInRegion (iObject* iobj)
{
  return scfParent->IsInRegion (iobj);
}
