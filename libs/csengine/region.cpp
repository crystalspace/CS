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
#include "csutil/csvector.h"
#include "ivideo/txtmgr.h"
#include "ivideo/texture.h"
#include "ivideo/material.h"
#include "csengine/region.h"
#include "csengine/engine.h"
#include "iengine/sector.h"
#include "iengine/collectn.h"
#include "iengine/campos.h"
#include "iengine/texture.h"
#include "iengine/material.h"
#include "iengine/engine.h"
#include "iengine/mesh.h"
#include "imesh/thing/curve.h"
#include "imesh/thing/ptextype.h"
#include "imesh/thing/polytmap.h"

CS_DECLARE_TYPED_VECTOR_NODELETE (csObjectVectorNodelete, iObject);

//---------------------------------------------------------------------------
SCF_IMPLEMENT_IBASE_EXT(csRegion)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE(iRegion)
SCF_IMPLEMENT_IBASE_EXT_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csRegion::Region)
  SCF_IMPLEMENTS_INTERFACE(iRegion)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

csRegion::csRegion (iEngine *e) :
  csObject()
{
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiRegion);
  engine = e;
}

csRegion::~csRegion ()
{
  scfiRegion.Clear ();
}

void csRegion::DeleteAll ()
{
  csRef<iObjectIterator> iter;

  // First we need to copy the objects to a csVector to avoid
  // messing up the iterator while we are deleting them.
  csObjectVectorNodelete copy;
  iter = GetIterator ();
  for ( ; !iter->IsFinished (); iter->Next ())
  {
    iObject *o = iter->GetObject ();
    copy.Push (o);
  }

  // Now we iterate over all objects in the 'copy' vector and
  // delete them. This will release them as csObject children
  // from this region parent.
  // Note that we traverse the list again and again for every
  // object type since the order in which objects types are deleted
  // is important. i.e. we should first delete all meshes and things
  // and only then delete the sectors.
  int i;
  for (i = 0; i < copy.Length (); i++)
  {
    if (copy[i])
    {
      iObject *obj = copy[i];
      csRef<iCollection> o (SCF_QUERY_INTERFACE (obj, iCollection));
      if (!o) continue;

      engine->GetCollections ()->Remove (o);
      ObjRemove (obj);
      copy[i] = NULL;
    }
  }

  for (i = 0; i < copy.Length (); i++)
  {
    if (copy[i])
    {
      iObject *obj = copy[i];
      csRef<iMeshWrapper> o (SCF_QUERY_INTERFACE (obj, iMeshWrapper));
      if (!o) continue;

      engine->GetMeshes ()->Remove (o);
      ObjRemove (obj);
      copy[i] = NULL;
    }
  }

  for (i = 0; i < copy.Length (); i++)
  {
    if (copy[i])
    {
      iObject *obj = copy[i];
      csRef<iMeshFactoryWrapper> o (SCF_QUERY_INTERFACE (
          obj,
          iMeshFactoryWrapper));
      if (!o) continue;

      engine->GetMeshFactories ()->Remove (o);
      ObjRemove (obj);
      copy[i] = NULL;
    }
  }

  for (i = 0; i < copy.Length (); i++)
  {
    if (copy[i])
    {
      iObject *obj = copy[i];
      csRef<iSector> o (SCF_QUERY_INTERFACE (obj, iSector));
      if (!o) continue;

      engine->GetSectors ()->Remove (o);
      ObjRemove (obj);
      copy[i] = NULL;
    }
  }

  for (i = 0; i < copy.Length (); i++)
  {
    if (copy[i])
    {
      iObject *obj = copy[i];
      csRef<iMaterialWrapper> o (SCF_QUERY_INTERFACE (obj, iMaterialWrapper));
      if (!o) continue;

      engine->GetMaterialList ()->Remove (o);
      ObjRemove (obj);
      copy[i] = NULL;
    }
  }

  for (i = 0; i < copy.Length (); i++)
  {
    if (copy[i])
    {
      iObject *obj = copy[i];
      csRef<iTextureWrapper> o (SCF_QUERY_INTERFACE (obj, iTextureWrapper));
      if (!o) continue;

      engine->GetTextureList ()->Remove (o);
      ObjRemove (obj);
      copy[i] = NULL;
    }
  }

  for (i = 0; i < copy.Length (); i++)
  {
    if (copy[i])
    {
      iObject *obj = copy[i];
      csRef<iCameraPosition> o (SCF_QUERY_INTERFACE (obj, iCameraPosition));
      if (!o) continue;

      engine->GetCameraPositions ()->Remove (o);
      ObjRemove (obj);
      copy[i] = NULL;
    }
  }

  for (i = 0; i < copy.Length (); i++)
  {
    if (copy[i])
    {
      iObject *obj = copy[i];
      csRef<iCurveTemplate> o (SCF_QUERY_INTERFACE (obj, iCurveTemplate));
      if (!o) continue;
      ObjRemove (obj);
      copy[i] = NULL;

      csRef<iThingEnvironment> te (SCF_QUERY_INTERFACE (
          engine->GetThingType (),
          iThingEnvironment));
      te->RemoveCurveTemplate (o);
    }
  }

  for (i = 0; i < copy.Length (); i++)
  {
    if (copy[i])
    {
      iObject *obj = copy[i];
      csRef<iPolyTxtPlane> o (SCF_QUERY_INTERFACE (obj, iPolyTxtPlane));
      if (!o) continue;
      ObjRemove (obj);
      copy[i] = NULL;

      csRef<iThingEnvironment> te (SCF_QUERY_INTERFACE (
          engine->GetThingType (),
          iThingEnvironment));
      te->RemovePolyTxtPlane (o);
    }
  }

#ifdef CS_DEBUG
  // Sanity check (only in debug mode). There should be no more
  // non-NULL references in the copy array now.
  for (i = 0; i < copy.Length (); i++)
  {
    if (copy[i])
    {
      iObject *o = copy[i];
      csEngine::current_engine->ReportBug (
          "\
There is still an object in the array after deleting region contents!\n\
Object name is '%s'",
          o->GetName () ? o->GetName () : "<NoName>");
      CS_ASSERT (false);
    }
  }
#endif // CS_DEBUG
}

bool csRegion::PrepareTextures ()
{
  csRef<iObjectIterator> iter;
  iTextureManager *txtmgr = csEngine::current_engine->G3D->GetTextureManager ();

  // First register all textures to the texture manager.
  {
    iter = GetIterator ();
    for ( ; !iter->IsFinished (); iter->Next ())
    {
      csRef<iTextureWrapper> csth (SCF_QUERY_INTERFACE (
          iter->GetObject (),
          iTextureWrapper));
      if (csth)
        if (!csth->GetTextureHandle ()) csth->Register (txtmgr);
    }
  }

  // Prepare all the textures.

  //@@@ Only prepare new textures: txtmgr->PrepareTextures ();
  {
    iter = GetIterator ();
    for ( ; !iter->IsFinished (); iter->Next ())
    {
      csRef<iTextureWrapper> csth (SCF_QUERY_INTERFACE (
          iter->GetObject (),
          iTextureWrapper));
      if (csth)
        csth->GetTextureHandle ()->Prepare ();
    }
  }

  // Then register all materials to the texture manager.
  {
    iter = GetIterator ();
    for ( ; !iter->IsFinished (); iter->Next ())
    {
      csRef<iMaterialWrapper> csmh (SCF_QUERY_INTERFACE (
          iter->GetObject (),
          iMaterialWrapper));
      if (csmh)
        if (!csmh->GetMaterialHandle ()) csmh->Register (txtmgr);
    }
  }

  // Prepare all the materials.
  //@@@ Only prepare new materials: txtmgr->PrepareMaterials ();
  {
    iter = GetIterator ();
    for ( ; !iter->IsFinished (); iter->Next ())
    {
      csRef<iMaterialWrapper> csmh (SCF_QUERY_INTERFACE (
          iter->GetObject (),
          iMaterialWrapper));
      if (csmh)
        csmh->GetMaterialHandle ()->Prepare ();
    }
  }

  return true;
}

bool csRegion::ShineLights ()
{
  engine->ShineLights (&scfiRegion);
  return true;
}

bool csRegion::Prepare ()
{
  if (!PrepareTextures ()) return false;
  if (!ShineLights ()) return false;
  return true;
}

iObject *csRegion::Region::QueryObject ()
{
  return scfParent;
}

void csRegion::Region::Clear ()
{
  scfParent->ObjRemoveAll ();
}

void csRegion::Region::DeleteAll ()
{
  scfParent->DeleteAll ();
}

bool csRegion::Region::PrepareTextures ()
{
  return scfParent->PrepareTextures ();
}

bool csRegion::Region::ShineLights ()
{
  return scfParent->ShineLights ();
}

bool csRegion::Region::Prepare ()
{
  return scfParent->Prepare ();
}

iSector *csRegion::Region::FindSector (const char *iName)
{
  csRef<iSector> sector (CS_GET_NAMED_CHILD_OBJECT (
      scfParent, iSector, iName));
  return sector;	// DecRef is ok here.
}

iMeshWrapper *csRegion::Region::FindMeshObject (const char *Name)
{
  char* p = strchr (Name, ':');
  if (p)
  {
    char* cname = csStrNew (Name);
    char* p2 = strchr (cname, ':');
    *p2 = 0;
    csRef<iMeshWrapper> m (CS_GET_NAMED_CHILD_OBJECT (
        scfParent, iMeshWrapper, cname));
    delete[] cname;
    if (m)
    {
      return m->GetChildren ()->FindByName (p+1);
    }
    return NULL;
  }
  else
  {
    csRef<iMeshWrapper> m (CS_GET_NAMED_CHILD_OBJECT (
        scfParent, iMeshWrapper, Name));
    return m;	// DecRef is ok here.
  }
}

iMeshFactoryWrapper *csRegion::Region::FindMeshFactory (const char *iName)
{
  csRef<iMeshFactoryWrapper> mf (CS_GET_NAMED_CHILD_OBJECT (
      scfParent, iMeshFactoryWrapper, iName));
  return mf;	// DecRef is ok here.
}

iTextureWrapper *csRegion::Region::FindTexture (const char *iName)
{
  csRef<iTextureWrapper> t (CS_GET_NAMED_CHILD_OBJECT (
      scfParent, iTextureWrapper, iName));
  return t;	// DecRef is ok here.
}

iMaterialWrapper *csRegion::Region::FindMaterial (const char *iName)
{
  csRef<iMaterialWrapper> m (CS_GET_NAMED_CHILD_OBJECT (
      scfParent, iMaterialWrapper, iName));
  return m;	// DecRef is ok here.
}

iCameraPosition *csRegion::Region::FindCameraPosition (const char *iName)
{
  csRef<iCameraPosition> cp (CS_GET_NAMED_CHILD_OBJECT (
      scfParent, iCameraPosition, iName));
  return cp;	// DecRef is ok here.
}

iCollection *csRegion::Region::FindCollection (const char *iName)
{
  csRef<iCollection> col (CS_GET_NAMED_CHILD_OBJECT (
      scfParent, iCollection, iName));
  return col;	// DecRef is ok here.
}

bool csRegion::IsInRegion (iObject *iobj)
{
  return iobj->GetObjectParent () == this;
}

bool csRegion::Region::IsInRegion (iObject *iobj)
{
  return scfParent->IsInRegion (iobj);
}

// ---------------------------------------------------------------------------
SCF_IMPLEMENT_IBASE(csRegionList)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE(iRegionList)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csRegionList::RegionList)
  SCF_IMPLEMENTS_INTERFACE(iRegionList)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

csRegionList::csRegionList () :
  csRegionListHelper(16, 16)
{
  SCF_CONSTRUCT_IBASE (NULL);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiRegionList);
}

int csRegionList::RegionList::GetCount () const
{
  return scfParent->Length ();
}

iRegion *csRegionList::RegionList::Get (int n) const
{
  return scfParent->Get (n);
}

int csRegionList::RegionList::Add (iRegion *obj)
{
  return scfParent->Push (obj);
}

bool csRegionList::RegionList::Remove (iRegion *obj)
{
  return scfParent->Delete (obj);
}

bool csRegionList::RegionList::Remove (int n)
{
  return scfParent->Delete (n);
}

void csRegionList::RegionList::RemoveAll ()
{
  scfParent->DeleteAll ();
}

int csRegionList::RegionList::Find (iRegion *obj) const
{
  return scfParent->Find (obj);
}

iRegion *csRegionList::RegionList::FindByName (const char *Name) const
{
  return scfParent->FindByName (Name);
}
