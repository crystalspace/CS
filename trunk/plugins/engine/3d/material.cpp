/*
    Copyright (C) 2001-2006 by Jorrit Tyberghein
    Copyright (C) 2000 by W.C.A. Wijngaards

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
#include "csqint.h"

#include "iengine/texture.h"
#include "ivideo/texture.h"
#include "ivideo/txtmgr.h"
#include "plugins/engine/3d/engine.h"
#include "plugins/engine/3d/material.h"

CS_LEAKGUARD_IMPLEMENT (csMaterial);
CS_LEAKGUARD_IMPLEMENT (csMaterialWrapper);

CS_IMPLEMENT_STATIC_CLASSVAR_REF(csMaterial, svNames, SVNames, 
                                 csMaterial::SVNamesHolder, ())

void csMaterial::SetupSVNames()
{
  if ((CS::ShaderVarStringID)(SVNames().diffuseTex) == CS::InvalidShaderVarStringID)
  {
    SVNames().diffuseTex = CS::ShaderVarName (engine->svNameStringSet,
      CS_MATERIAL_TEXTURE_DIFFUSE);
  }
}

csMaterial::csMaterial (csEngine* engine) :
  scfImplementationType(this), engine (engine)
{
  SetupSVNames();

  SetTextureWrapper (0);
}

csMaterial::csMaterial (csEngine* engine,
			iTextureWrapper *w) :
  scfImplementationType(this), engine (engine)
{
  SetupSVNames();

  SetTextureWrapper (w);
}

csMaterial::~csMaterial ()
{
}

csShaderVariable* csMaterial::GetVar (CS::ShaderVarStringID name, bool create)
{
  csRef<csShaderVariable> var = 
    CS::ShaderVariableContextImpl::GetVariable (name);
  if ((var == 0) && create)
  {
    var.AttachNew (new csShaderVariable (name));
    CS::ShaderVariableContextImpl::AddVariable (var);
  }
  return var;
}

void csMaterial::SetTextureWrapper (iTextureWrapper *tex)
{
  SetTextureWrapper (SVNames().diffuseTex, tex);
}


iTextureWrapper* csMaterial::GetTextureWrapper (CS::ShaderVarStringID name)
{
  iTextureWrapper* tex;
  GetVar (name)->GetValue (tex);
  return tex;
}

void csMaterial::SetTextureWrapper (CS::ShaderVarStringID name, iTextureWrapper* tex)
{
  csShaderVariable* var = GetVar (name, true);
  var->SetValue (tex);
}



void csMaterial::SetShader (csStringID type, iShader* shd)
{
  shaders.PutUnique (type, shd);
}

iShader* csMaterial::GetShader(csStringID type)
{
  return shaders.Get (type, (iShader*)0);
}

iShader* csMaterial::GetFirstShader (const csStringID* types,
                                     size_t numTypes)
{
  iShader* s = 0;
  for (size_t i = 0; i < numTypes; i++)
  {
    s = shaders.Get (types[i], (iShader*)0);
    if (s != 0) break;
  }
  return s;
}

iTextureHandle *csMaterial::GetTexture ()
{
  iTextureWrapper* tex;
  GetVar (SVNames().diffuseTex)->GetValue (tex);
  if (tex) return tex->GetTextureHandle ();
  else return 0;
}

iTextureHandle* csMaterial::GetTexture (CS::ShaderVarStringID name)
{
  iTextureWrapper* tex;
  csShaderVariable* var = GetVar (name);
  if (var)
  {
    var->GetValue (tex);
    return tex ? tex->GetTextureHandle () : 0;
  }
  return 0;
}


void csMaterial::Visit ()
{
  // @@@ Implement me!!!!!!!!!!
#if 0
  if (texture)
  {
    texture->Visit ();
    int i;
    for (i = 0 ; i < num_texture_layers ; i++)
    {
      texture_layer_wrappers[i]->Visit ();
    }
  }
#endif
}

bool csMaterial::IsVisitRequired () const
{
  // @@@ Implement me!!!!!!!!!!
  return false;
#if 0
  if (texture)
  {
    if (texture->IsVisitRequired ()) return true;
    int i;
    for (i = 0 ; i < num_texture_layers ; i++)
       if (texture_layer_wrappers[i]->IsVisitRequired ())
         return true;
  }
  return false;
#endif
}

//---------------------------------------------------------------------------
csMaterialWrapper::csMaterialWrapper (iMaterialList* materials, 
                                      iMaterial *m) :
  scfImplementationType (this), materials (materials)
{
  material = m;
  matEngine = scfQueryInterface<iMaterialEngine> (material);
}

csMaterialWrapper::~csMaterialWrapper ()
{
}

void csMaterialWrapper::SelfDestruct ()
{
  materials->Remove (static_cast<iMaterialWrapper*> (this));
}

void csMaterialWrapper::SetMaterial (iMaterial *m)
{
  material = m;
  matEngine = scfQueryInterface<iMaterialEngine> (material);
}

void csMaterialWrapper::Visit ()
{
  if (matEngine)
  {
    matEngine->Visit ();
  }
}

bool csMaterialWrapper::IsVisitRequired () const
{
  if (matEngine)
  {
    return matEngine->IsVisitRequired ();
  }
  return false;
}

//------------------------------------------------------ csMaterialList -----//
csMaterialList::csMaterialList () : scfImplementationType (this)
{
  listener.AttachNew (new NameChangeListener (this));
}

csMaterialList::~csMaterialList()
{
}

void csMaterialList::NameChanged (iObject* object, const char* oldname,
  	const char* newname)
{
  csRef<iMaterialWrapper> mat = scfQueryInterface<iMaterialWrapper> (object);
  CS_ASSERT (mat != 0);
  CS::Threading::ScopedWriteLock lock(matLock);
  if (oldname) mat_hash.Delete (oldname, mat);
  if (newname) mat_hash.Put (newname, mat);
}

iMaterialWrapper *csMaterialList::NewMaterial (iMaterial *material,
	const char* name)
{
  csRef<iMaterialWrapper> tm;
  tm.AttachNew (new csMaterialWrapper (this, material));
  tm->QueryObject ()->SetName (name);
  CS::Threading::ScopedWriteLock lock(matLock);
  if (name)
    mat_hash.Put (name, tm);
  list.Push (tm);
  tm->QueryObject ()->AddNameChangeListener (listener);
  return tm;
}

csPtr<iMaterialWrapper> csMaterialList::CreateMaterial (iMaterial* material,
  	const char* name)
{
  csRef<iMaterialWrapper> tm;
  tm.AttachNew (new csMaterialWrapper (this, material));
  tm->QueryObject ()->SetName (name);
  return csPtr<iMaterialWrapper>(tm);
}

int csMaterialList::Add (iMaterialWrapper *obj)
{
  CS::Threading::ScopedWriteLock lock(matLock);
  const char* name = obj->QueryObject ()->GetName ();
  if (name)
    mat_hash.Put (name, obj);
  obj->QueryObject ()->AddNameChangeListener (listener);
  return (int)list.Push (obj);
}

void csMaterialList::AddBatch (csRef<iMaterialLoaderIterator> itr)
{
  CS::Threading::ScopedWriteLock lock(matLock);
  while(itr->HasNext())
  {
    iMaterialWrapper* obj = itr->Next();
    const char* name = obj->QueryObject ()->GetName ();
    if (name)
      mat_hash.Put (name, obj);
    obj->QueryObject ()->AddNameChangeListener (listener);
    list.Push (obj);
  }
}

bool csMaterialList::Remove (iMaterialWrapper *obj)
{
  CS::Threading::ScopedWriteLock lock(matLock);
  const char* name = obj->QueryObject ()->GetName ();
  if (name)
    mat_hash.Delete (name, obj);
  obj->QueryObject ()->RemoveNameChangeListener (listener);
  return list.Delete (obj);
}

bool csMaterialList::Remove (int n)
{
  CS::Threading::ScopedWriteLock lock(matLock);
  iMaterialWrapper* obj = list[n];
  const char* name = obj->QueryObject ()->GetName ();
  if (name)
    mat_hash.Delete (name, obj);
  obj->QueryObject ()->RemoveNameChangeListener (listener);
  return list.DeleteIndex (n);
}

void csMaterialList::RemoveAll ()
{
  CS::Threading::ScopedWriteLock lock(matLock);
  size_t i;
  for (i = 0 ; i < list.GetSize () ; i++)
    list[i]->QueryObject ()->RemoveNameChangeListener (listener);
  list.DeleteAll ();
  mat_hash.DeleteAll ();
}

int csMaterialList::GetCount () const
{
  CS::Threading::ScopedReadLock lock(matLock);
  return (int)list.GetSize ();
}

iMaterialWrapper* csMaterialList::Get (int n) const
{
  CS::Threading::ScopedReadLock lock(matLock);
  return list[n];
}

int csMaterialList::Find (iMaterialWrapper *obj) const
{
  CS::Threading::ScopedReadLock lock(matLock);
  return (int)list.Find (obj);
}

iMaterialWrapper *csMaterialList::FindByName (const char *Name) const
{
  CS::Threading::ScopedReadLock lock(matLock);
  return mat_hash.Get (Name, 0);
}
