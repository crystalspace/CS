/*
    Copyright (C) 2000-2007 by Jorrit Tyberghein

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
#include "csqsqrt.h"
#include "csgeom/sphere.h"
#include "imesh/objmodel.h"
#include "igeom/clip2d.h"
#include "plugins/engine/3d/sector.h"
#include "plugins/engine/3d/meshfact.h"
#include "plugins/engine/3d/meshlod.h"
#include "plugins/engine/3d/light.h"
#include "plugins/engine/3d/engine.h"
#include "iengine/portal.h"
#include "iengine/rview.h"
#include "ivideo/graph3d.h"
#include "ivideo/rendermesh.h"


CS_LEAKGUARD_IMPLEMENT (csMeshFactoryWrapper);

// ---------------------------------------------------------------------------
// csMeshFactoryWrapper
// ---------------------------------------------------------------------------


csMeshFactoryWrapper::csMeshFactoryWrapper (csEngine* engine,
                                            iMeshObjectFactory *meshFact)
  : scfImplementationType (this), meshFact (meshFact), parent (0),
  zbufMode (CS_ZBUF_USE), engine (engine), min_imposter_distance(0),
  imposter_renderReal(false)
{
  children.SetMeshFactory (this);

  render_priority = engine->GetObjectRenderPriority ();

  instanceFactory = 0;

  // Set up shader variable for instancing.
  csRef<iShaderVarStringSet> SVstrings = csQueryRegistryTagInterface<iShaderVarStringSet> (
    engine->objectRegistry, "crystalspace.shader.variablenameset");
  varTransform = SVstrings->Request ("instancing transforms");
  
  /* Work around iShaderVariableContext cast not working before 
     CS::ShaderVariableContextImpl construction */
  csRefTrackerAccess::AddAlias (
    static_cast<iShaderVariableContext*> (this),
    this);
}

csMeshFactoryWrapper::csMeshFactoryWrapper (csEngine* engine)
  : scfImplementationType (this), parent (0), zbufMode (CS_ZBUF_USE), 
  engine (engine), min_imposter_distance(0), imposter_renderReal(false)
{
  children.SetMeshFactory (this);

  render_priority = engine->GetObjectRenderPriority ();

  instanceFactory = 0;

  // Set up shader variable for instancing.
  csRef<iShaderVarStringSet> SVstrings = csQueryRegistryTagInterface<iShaderVarStringSet> (
    engine->objectRegistry, "crystalspace.shader.variablenameset");
  varTransform = SVstrings->Request ("instancing transforms");
  
  /* Work around iShaderVariableContext cast not working before 
     CS::ShaderVariableContextImpl construction */
  csRefTrackerAccess::AddAlias (
    static_cast<iShaderVariableContext*> (this),
    this);
}

csMeshFactoryWrapper::~csMeshFactoryWrapper ()
{
  // This line MUST be here to ensure that the children are not
  // removed after the destructor has already finished.
  children.RemoveAll ();

  // The factory has the memory ownership of the extra render buffers
  for (size_t i = 0; i < extraRenderMeshes.GetSize (); i++)
    delete extraRenderMeshes[i];
}

void csMeshFactoryWrapper::SelfDestruct ()
{
  engine->GetMeshFactories ()->Remove (
    static_cast<iMeshFactoryWrapper*> (this));
}

void csMeshFactoryWrapper::SetZBufModeRecursive (csZBufMode mode)
{
  SetZBufMode (mode);
  const iMeshFactoryList* ml = &children;
  if (!ml) return;
  int i;
  for (i = 0 ; i < ml->GetCount () ; i++)
    ml->Get (i)->SetZBufModeRecursive (mode);
}

void csMeshFactoryWrapper::SetRenderPriorityRecursive (CS::Graphics::RenderPriority rp)
{
  SetRenderPriority (rp);
  const iMeshFactoryList* ml = &children;
  if (!ml) return;
  int i;
  for (i = 0 ; i < ml->GetCount () ; i++)
    ml->Get (i)->SetRenderPriorityRecursive (rp);
}

void csMeshFactoryWrapper::SetRenderPriority (CS::Graphics::RenderPriority rp)
{
  render_priority = rp;
}

void csMeshFactoryWrapper::SetMeshObjectFactory (iMeshObjectFactory *meshFact)
{
  csMeshFactoryWrapper::meshFact = meshFact;
}

csPtr<iMeshWrapper> csMeshFactoryWrapper::CreateMeshWrapper ()
{
  csRef<iMeshObject> basemesh = meshFact->NewInstance ();
  csMeshWrapper* cmesh = new csMeshWrapper (engine, basemesh);
  iMeshWrapper* mesh = static_cast<iMeshWrapper*> (cmesh);
  basemesh->SetMeshWrapper (mesh);

  if (GetName ()) mesh->QueryObject ()->SetName (GetName ());
  mesh->SetFactory (this);
  mesh->SetRenderPriority (render_priority);
  mesh->SetZBufMode (zbufMode);
  mesh->GetFlags ().Set (flags.Get (), flags.Get ());

  if (static_lod)
  {
    iLODControl* lod = mesh->CreateStaticLOD ();
    iSharedVariable* varm, * vara;
    static_lod->GetLOD (varm, vara);
    if (varm)
    {
      lod->SetLOD (varm, vara);
    }
    else
    {
      float m, a;
      static_lod->GetLOD (m, a);
      lod->SetLOD (m, a);
    }
  }

  int i;
  for (i = 0; i < children.GetCount (); i++)
  {
    iMeshFactoryWrapper *childfact = children.Get (i);
    csRef<iMeshWrapper> child = childfact->CreateMeshWrapper ();
    child->QuerySceneNode ()->SetParent (mesh->QuerySceneNode ());
    child->GetMovable ()->SetTransform (childfact->GetTransform ());
    child->GetMovable ()->UpdateMove ();

    if (static_lod)
    {
      // We have static lod so we need to put the child in the right
      // lod level.
      int l;
      for (l = 0 ; l < static_lod->GetLODCount () ; l++)
      {
        csArray<iMeshFactoryWrapper*>& facts_for_lod =
      	  static_lod->GetMeshesForLOD (l);
        size_t j;
	for (j = 0 ; j < facts_for_lod.GetSize () ; j++)
	{
	  if (facts_for_lod[j] == childfact)
	    mesh->AddMeshToStaticLOD (l, child);
	}
      }
    }
  }

  for (size_t i = 0; i < extraRenderMeshes.GetSize (); i++)
    mesh->AddExtraRenderMesh (extraRenderMeshes[i]);

  return csPtr<iMeshWrapper> (mesh);
}

void csMeshFactoryWrapper::HardTransform (const csReversibleTransform &t)
{
  meshFact->HardTransform (t);
}

iLODControl* csMeshFactoryWrapper::CreateStaticLOD ()
{
  static_lod = csPtr<csStaticLODFactoryMesh> (new csStaticLODFactoryMesh ());
  return static_lod;
}

void csMeshFactoryWrapper::DestroyStaticLOD ()
{
  static_lod = 0;
}

iLODControl* csMeshFactoryWrapper::GetStaticLOD ()
{
  return (iLODControl*)static_lod;
}

void csMeshFactoryWrapper::SetStaticLOD (float m, float a)
{
  if (static_lod) static_lod->SetLOD (m, a);
}

void csMeshFactoryWrapper::GetStaticLOD (float& m, float& a) const
{
  if (static_lod)
    static_lod->GetLOD (m, a);
  else
  {
    m = 0;
    a = 0;
  }
}

void csMeshFactoryWrapper::RemoveFactoryFromStaticLOD (
	iMeshFactoryWrapper* fact)
{
  if (!static_lod) return;	// No static lod, nothing to do here.
  int lod;
  for (lod = 0 ; lod < static_lod->GetLODCount () ; lod++)
  {
    csArray<iMeshFactoryWrapper*>& meshes_for_lod =
    	static_lod->GetMeshesForLOD (lod);
    meshes_for_lod.Delete (fact);
  }
}

void csMeshFactoryWrapper::AddFactoryToStaticLOD (int lod,
	iMeshFactoryWrapper* fact)
{
  if (!static_lod) return;	// No static lod, nothing to do here.
  csArray<iMeshFactoryWrapper*>& meshes_for_lod =
  	static_lod->GetMeshesForLOD (lod);
  meshes_for_lod.Push (fact);
}

void csMeshFactoryWrapper::AddImposter(iMeshWrapper* mesh, iRenderView* rview)
{
  // Create a new imposter mesh.
  csRef<iImposterMesh> imposter = csPtr<iImposterMesh>(new csImposterMesh(engine,
    this, mesh, rview, imposter_shaders));
  imposters.Put(csPtrKey<iMeshWrapper>(mesh), imposter);
}

bool csMeshFactoryWrapper::RenderingImposter(iMeshWrapper* mesh)
{
  csRef<iImposterMesh> imposter = imposters.Get(csPtrKey<iMeshWrapper>(mesh), csRef<iImposterMesh>());
  if(imposter.IsValid())
  {
    return imposter->Rendered() || !imposter_renderReal;
  }

  return false;
}

void csMeshFactoryWrapper::RemoveImposter(iMeshWrapper* mesh)
{
  csRef<iImposterMesh> imposter = imposters.Get(csPtrKey<iMeshWrapper>(mesh), csRef<iImposterMesh>());
  if (imposter)
  {
    imposter->Destroy();
    imposters.Delete(csPtrKey<iMeshWrapper>(mesh), imposter);
  }
}

void csMeshFactoryWrapper::AddInstance(csVector3& position, csMatrix3& rotation)
{
  if(!transformVars.IsValid())
  {
    transformVars.AttachNew (new csShaderVariable (varTransform));
    transformVars->SetType (csShaderVariable::ARRAY);
    transformVars->SetArraySize (0);
  }

  csRef<csShaderVariable> transformVar;
  transformVar.AttachNew(new csShaderVariable);

  csReversibleTransform tr(rotation.GetInverse(), position);
  transformVar->SetValue (tr);

  transformVars->AddVariableToArray(transformVar);
}

size_t csMeshFactoryWrapper::AddExtraRenderMesh (CS::Graphics::RenderMesh* renderMesh)
{
  return extraRenderMeshes.Push (renderMesh);
}

CS::Graphics::RenderMesh* csMeshFactoryWrapper::GetExtraRenderMesh (size_t idx) const
{
  return extraRenderMeshes[idx];
}

void csMeshFactoryWrapper::RemoveExtraRenderMesh (csRenderMesh* renderMesh)
{
    size_t len = extraRenderMeshes.GetSize ();
    for (size_t a=0; a<len; ++a)
    {
        if (extraRenderMeshes[a] != renderMesh)
            continue;

        extraRenderMeshes.DeleteIndexFast (a);

        return;
    }
}

void csMeshFactoryWrapper::RemoveExtraRenderMesh (size_t index)
{
  extraRenderMeshes.DeleteIndexFast (index);
}

//--------------------------------------------------------------------------
// csMeshFactoryList
//--------------------------------------------------------------------------
csMeshFactoryList::csMeshFactoryList ()
  : scfImplementationType (this), list (64)
{
  listener.AttachNew (new NameChangeListener (this));
}

csMeshFactoryList::~csMeshFactoryList ()
{
  RemoveAll ();
}

void csMeshFactoryList::NameChanged (iObject* object, const char* oldname,
  	const char* newname)
{
  csRef<iMeshFactoryWrapper> mesh = 
    scfQueryInterface<iMeshFactoryWrapper> (object);
  CS_ASSERT (mesh != 0);
  CS::Threading::ScopedWriteLock lock(meshFactLock);
  if (oldname) factories_hash.Delete (oldname, mesh);
  if (newname) factories_hash.Put (newname, mesh);
}

int csMeshFactoryList::Add (iMeshFactoryWrapper *obj)
{
  PrepareFactory (obj);
  const char* name = obj->QueryObject ()->GetName ();
  CS::Threading::ScopedWriteLock lock(meshFactLock);
  if (name)
    factories_hash.Put (name, obj);
  obj->QueryObject ()->AddNameChangeListener (listener);
  return (int)list.Push (obj);
}

void csMeshFactoryList::AddBatch (csRef<iMeshFactLoaderIterator> itr)
{
  CS::Threading::ScopedWriteLock lock(meshFactLock);
  while(itr->HasNext())
  {
    iMeshFactoryWrapper* obj = itr->Next();
    PrepareFactory (obj);
    const char* name = obj->QueryObject ()->GetName ();
    if (name)
      factories_hash.Put (name, obj);
    obj->QueryObject ()->AddNameChangeListener (listener);
    list.Push (obj);
  }
}

bool csMeshFactoryList::Remove (iMeshFactoryWrapper *obj)
{
  FreeFactory (obj);
  const char* name = obj->QueryObject ()->GetName ();
  CS::Threading::ScopedWriteLock lock(meshFactLock);
  if (name)
    factories_hash.Delete (name, obj);
  obj->QueryObject ()->RemoveNameChangeListener (listener);
  list.Delete (obj);
  return true;
}

bool csMeshFactoryList::Remove (int n)
{
  return Remove (Get (n));
}

void csMeshFactoryList::RemoveAll ()
{
  CS::Threading::ScopedWriteLock lock(meshFactLock);
  size_t i;
  for (i = 0 ; i < list.GetSize () ; i++)
  {
    list[i]->QueryObject ()->RemoveNameChangeListener (listener);
    FreeFactory (list[i]);
  }
  factories_hash.DeleteAll ();
  list.DeleteAll ();
}

int csMeshFactoryList::GetCount () const 
{
  CS::Threading::ScopedReadLock lock(meshFactLock);
  return (int)list.GetSize ();
}

iMeshFactoryWrapper* csMeshFactoryList::Get (int n) const
{
  CS::Threading::ScopedReadLock lock(meshFactLock);
  return list.Get (n);
}

int csMeshFactoryList::Find (iMeshFactoryWrapper *obj) const
{
  CS::Threading::ScopedReadLock lock(meshFactLock);
  return (int)list.Find (obj);
}

iMeshFactoryWrapper *csMeshFactoryList::FindByName (
  const char *Name) const
{
  CS::Threading::ScopedReadLock lock(meshFactLock);
  if (!Name) return 0;
  return factories_hash.Get (Name, 0);
}

//--------------------------------------------------------------------------
// csMeshFactoryFactoryList
//--------------------------------------------------------------------------
void csMeshFactoryFactoryList::PrepareFactory (iMeshFactoryWrapper* child)
{
  CS_ASSERT (meshfact != 0);
  csMeshFactoryList::PrepareFactory (child);

  // unlink the factory from another possible parent.
  if (child->GetParentContainer ())
    child->GetParentContainer ()->GetChildren ()->Remove (child);

  child->SetParentContainer (meshfact);
}

void csMeshFactoryFactoryList::FreeFactory (iMeshFactoryWrapper* item)
{
  CS_ASSERT (meshfact != 0);
  item->SetParentContainer (0);
  meshfact->RemoveFactoryFromStaticLOD (item);
  csMeshFactoryList::FreeFactory (item);
}
