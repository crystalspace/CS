/*
    Copyright (C) 2000-2006 by Jorrit Tyberghein

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
#include "csutil/sysfunc.h"
#include "csutil/array.h"
#include "plugins/engine/3d/region.h"
#include "plugins/engine/3d/engine.h"
#include "iengine/sector.h"
#include "iengine/collectn.h"
#include "iengine/campos.h"
#include "iengine/texture.h"
#include "iengine/material.h"
#include "iengine/engine.h"
#include "iengine/mesh.h"

//---------------------------------------------------------------------------


csRegion::csRegion (csEngine *e) 
  : scfImplementationType (this), engine (e)
{
}

csRegion::~csRegion ()
{
  Clear ();
}

void csRegion::DeleteAll ()
{
  // First we need to copy the objects to a vector to avoid
  // messing up the iterator while we are deleting them.
  csArray<iObject*> copy (1024, 256);
  csRef<iObjectIterator> iter = GetIterator ();
  while (iter->HasNext ())
  {
    iObject *o = iter->Next ();
    copy.Push (o);
  }

  size_t total = copy.Length ();

  // Now we iterate over all objects in the 'copy' vector and
  // delete them. This will release them as csObject children
  // from this region parent.
  size_t i;

  // The first loop is the most general one where we just use
  // engine->RemoveObject().
  for (i = 0; i < copy.Length (); i++)
  {
    iBase* b = (iBase*)copy[i];
    if (csEngine::currentEngine->RemoveObject (b))
    {
      copy[i] = 0;
      total--;
    }
  }

  if (!total) return;

  if (total > 0)
  {
    // Sanity check. There should be no more
    // non-0 references in the copy array now.
    for (i = 0; i < copy.Length (); i++)
      if (copy[i])
      {
        iObject *o = copy[i];
        csEngine::currentEngine->ReportBug (
          "\
There is still an object in the array after deleting region contents!\n\
Object name is '%s'",
          o->GetName () ? o->GetName () : "<NoName>");
      }
    CS_ASSERT (false);
  }
}

bool csRegion::PrepareTextures ()
{
  csRef<iObjectIterator> iter;
  iTextureManager *txtmgr = csEngine::currentEngine->G3D->GetTextureManager ();

  // First register all textures to the texture manager.
  iter = GetIterator ();
  while (iter->HasNext ())
  {
    csRef<iTextureWrapper> csth (SCF_QUERY_INTERFACE (
        iter->Next (),
        iTextureWrapper));
    if (csth)
    {
      if (!csth->GetTextureHandle ()) csth->Register (txtmgr);
      if (!csth->KeepImage ()) csth->SetImageFile (0);
    }
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
  if (!ShineLights ()) return false;
  return true;
}

iObject *csRegion::QueryObject ()
{
  return this;
}

void csRegion::Clear ()
{
  ObjRemoveAll ();
}


iSector *csRegion::FindSector (const char *iName)
{
  csRef<iSector> sector (CS_GET_NAMED_CHILD_OBJECT (
      this, iSector, iName));
  return sector;	// DecRef is ok here.
}

iMeshWrapper *csRegion::FindMeshObject (const char *Name)
{
  char const* p = strchr (Name, ':');
  if (p)
  {
    char* cname = csStrNew (Name);
    char* p2 = strchr (cname, ':');
    *p2 = 0;
    csRef<iMeshWrapper> m = CS_GET_NAMED_CHILD_OBJECT (
        this, iMeshWrapper, cname);
    delete[] cname;
    if (m)
    {
      return m->FindChildByName (p+1);
    }
    return 0;
  }
  else
  {
    csRef<iMeshWrapper> m = CS_GET_NAMED_CHILD_OBJECT (
        this, iMeshWrapper, Name);
    return m;	// DecRef is ok here.
  }
}

iMeshFactoryWrapper *csRegion::FindMeshFactory (const char *iName)
{
  csRef<iMeshFactoryWrapper> mf (CS_GET_NAMED_CHILD_OBJECT (
      this, iMeshFactoryWrapper, iName));
  return mf;	// DecRef is ok here.
}

iTextureWrapper *csRegion::FindTexture (const char *iName)
{
  csRef<iTextureWrapper> t (CS_GET_NAMED_CHILD_OBJECT (
      this, iTextureWrapper, iName));
  return t;	// DecRef is ok here.
}

iMaterialWrapper *csRegion::FindMaterial (const char *iName)
{
  csRef<iMaterialWrapper> m (CS_GET_NAMED_CHILD_OBJECT (
      this, iMaterialWrapper, iName));
  return m;	// DecRef is ok here.
}

iCameraPosition *csRegion::FindCameraPosition (const char *iName)
{
  csRef<iCameraPosition> cp (CS_GET_NAMED_CHILD_OBJECT (
      this, iCameraPosition, iName));
  return cp;	// DecRef is ok here.
}

iCollection *csRegion::FindCollection (const char *iName)
{
  csRef<iCollection> col (CS_GET_NAMED_CHILD_OBJECT (
      this, iCollection, iName));
  return col;	// DecRef is ok here.
}

bool csRegion::IsInRegion (iObject *iobj)
{
  return iobj->GetObjectParent () == this;
}


// ---------------------------------------------------------------------------

csRegionList::csRegionList () 
  : scfImplementationType (this),
  regionList (16, 16)
{
}

csRegionList::~csRegionList()
{
}

int csRegionList::GetCount () const
{
  return (int)regionList.Length ();
}

iRegion *csRegionList::Get (int n) const
{
  return regionList.Get (n);
}

int csRegionList::Add (iRegion *obj)
{
  return (int)regionList.Push (obj);
}

bool csRegionList::Remove (iRegion *obj)
{
  return regionList.Delete (obj);
}

bool csRegionList::Remove (int n)
{
  return regionList.DeleteIndex (n);
}

void csRegionList::RemoveAll ()
{
  regionList.DeleteAll ();
}

int csRegionList::Find (iRegion *obj) const
{
  return (int)regionList.Find (obj);
}

iRegion *csRegionList::FindByName (const char *Name) const
{
  return regionList.FindByName (Name);
}
