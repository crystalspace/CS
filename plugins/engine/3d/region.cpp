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
#include "csutil/sysfunc.h"
#include "csutil/array.h"
#include "ivideo/txtmgr.h"
#include "ivideo/texture.h"
#include "ivideo/material.h"
#include "ivideo/shader/shader.h"
#include "plugins/engine/3d/region.h"
#include "plugins/engine/3d/engine.h"
#include "iengine/sector.h"
#include "iengine/collectn.h"
#include "iengine/campos.h"
#include "iengine/texture.h"
#include "iengine/material.h"
#include "iengine/engine.h"
#include "iengine/mesh.h"
#include "ivaria/engseq.h"
#include "isndsys/ss_manager.h"

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
  csRef<iObjectIterator> iter;
  iEngineSequenceManager* engseq = csEngine::currentEngine
    ->GetEngineSequenceManager (false);

  // First we need to copy the objects to a vector to avoid
  // messing up the iterator while we are deleting them.
  csArray<iObject*> copy;
  iter = GetIterator ();
  while (iter->HasNext ())
  {
    iObject *o = iter->Next ();
    copy.Push (o);
  }

  // Now we iterate over all objects in the 'copy' vector and
  // delete them. This will release them as csObject children
  // from this region parent.
  // Note that we traverse the list again and again for every
  // object type since the order in which objects types are deleted
  // is important. i.e. we should first delete all meshes and things
  // and only then delete the sectors.
  size_t i;

  for (i = 0; i < copy.Length (); i++)
  {
    if (copy[i])
    {
      iObject *obj = copy[i];
      csRef<iSndSysWrapper> o = SCF_QUERY_INTERFACE (obj, iSndSysWrapper);
      if (!o) continue;

      if (!snd_manager)
      {
	snd_manager = CS_QUERY_REGISTRY (
  	  csEngine::currentEngine->objectRegistry, iSndSysManager);
      }
      if (snd_manager) snd_manager->RemoveSound (o);
      ObjRemove (obj);
      copy[i] = 0;
    }
  }

  for (i = 0; i < copy.Length (); i++)
  {
    if (copy[i])
    {
      iObject *obj = copy[i];
      csRef<iSequenceWrapper> o (SCF_QUERY_INTERFACE (obj, iSequenceWrapper));
      if (!o) continue;

      if (engseq) engseq->RemoveSequence (o);
      ObjRemove (obj);
      copy[i] = 0;
    }
  }

  for (i = 0; i < copy.Length (); i++)
  {
    if (copy[i])
    {
      iObject *obj = copy[i];
      csRef<iSequenceTrigger> o (SCF_QUERY_INTERFACE (obj, iSequenceTrigger));
      if (!o) continue;

      if (engseq) engseq->RemoveTrigger (o);
      ObjRemove (obj);
      copy[i] = 0;
    }
  }

  for (i = 0; i < copy.Length (); i++)
  {
    if (copy[i])
    {
      iObject *obj = copy[i];
      csRef<iSharedVariable> o (SCF_QUERY_INTERFACE (obj, iSharedVariable));
      if (!o) continue;

      engine->GetVariableList ()->Remove (o);
      ObjRemove (obj);
      copy[i] = 0;
    }
  }

  for (i = 0; i < copy.Length (); i++)
  {
    if (copy[i])
    {
      iObject *obj = copy[i];
      csRef<iCollection> o (SCF_QUERY_INTERFACE (obj, iCollection));
      if (!o) continue;

      engine->GetCollections ()->Remove (o);
      ObjRemove (obj);
      copy[i] = 0;
    }
  }

  for (i = 0; i < copy.Length (); i++)
  {
    if (copy[i])
    {
      iObject *obj = copy[i];
      csRef<iLight> o (SCF_QUERY_INTERFACE (obj, iLight));
      if (!o) continue;

      engine->RemoveLight (o);
      ObjRemove (obj);
      copy[i] = 0;
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
      copy[i] = 0;
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
      copy[i] = 0;
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
      copy[i] = 0;
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
      copy[i] = 0;
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
      copy[i] = 0;
    }
  }

  csRef<iShaderManager> shmgr = CS_QUERY_REGISTRY (
  	csEngine::currentEngine->objectRegistry, iShaderManager);
  if (shmgr)
    for (i = 0; i < copy.Length (); i++)
    {
      if (copy[i])
      {
        iObject *obj = copy[i];
        csRef<iShader> o (SCF_QUERY_INTERFACE (obj, iShader));
        if (!o) continue;

        shmgr->UnregisterShader (o);
        ObjRemove (obj);
        copy[i] = 0;
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
      copy[i] = 0;
    }
  }

#ifdef CS_DEBUG
  // Sanity check (only in debug mode). There should be no more
  // non-0 references in the copy array now.
  for (i = 0; i < copy.Length (); i++)
  {
    if (copy[i])
    {
      iObject *o = copy[i];
      csEngine::currentEngine->ReportBug (
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
    csRef<iMeshWrapper> m (CS_GET_NAMED_CHILD_OBJECT (
        this, iMeshWrapper, cname));
    delete[] cname;
    if (m)
    {
      return m->GetChildren ()->FindByName (p+1);
    }
    return 0;
  }
  else
  {
    csRef<iMeshWrapper> m (CS_GET_NAMED_CHILD_OBJECT (
        this, iMeshWrapper, Name));
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
