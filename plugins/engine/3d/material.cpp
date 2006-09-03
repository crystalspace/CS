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

#include "csutil/debug.h"
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
  if (SVNames().diffuseTex == csInvalidStringID)
  {
    SVNames().diffuseTex = CS::ShaderVarName (engine->globalStringSet,
      CS_MATERIAL_TEXTURE_DIFFUSE);
    SVNames().flatcolor = CS::ShaderVarName (engine->globalStringSet,
      CS_MATERIAL_VARNAME_FLATCOLOR);
  }
}

csMaterial::csMaterial (csEngine* engine) :
  scfImplementationType(this), engine (engine)
{
  SetupSVNames();

  SetTextureWrapper (0);
  SetFlatColor (csRGBcolor (255, 255, 255));
}

csMaterial::csMaterial (csEngine* engine,
			iTextureWrapper *w) :
  scfImplementationType(this), engine (engine)
{
  SetupSVNames();

  SetTextureWrapper (w);
  SetFlatColor (csRGBcolor (255, 255, 255));
}

csMaterial::~csMaterial ()
{
}

csShaderVariable* csMaterial::GetVar (csStringID name, bool create)
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

void csMaterial::GetFlatColor (csRGBpixel &oColor, bool /*useTextureMean*/)
{
  csRGBcolor flat_color;
  csShaderVariable* var = GetVar (SVNames().flatcolor);
  if (var == 0) 
  {
    flat_color.Set (255, 255, 255);
  }
  else
  {
    csVector3 v;
    var->GetValue (v);
    flat_color.Set (csQint (v.x * 255.99f), csQint (v.y * 255.99f), 
      csQint (v.z * 255.99f));
  }
  oColor = flat_color;
}

void csMaterial::SetFlatColor (const csRGBcolor& col)
{ 
  csShaderVariable* var = GetVar (SVNames().flatcolor, true);
  csVector3 v (((float)col.red) / 255.0f, ((float)col.green) / 255.0f, 
    ((float)col.blue) / 255.0f);
  var->SetValue (v);
}

void csMaterial::SetTextureWrapper (iTextureWrapper *tex)
{
  SetTextureWrapper (SVNames().diffuseTex, tex);
}


iTextureWrapper* csMaterial::GetTextureWrapper (csStringID name)
{
  iTextureWrapper* tex;
  GetVar (name)->GetValue (tex);
  return tex;
}

void csMaterial::SetTextureWrapper (csStringID name, iTextureWrapper* tex)
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
  return shaders.Get (type, 0);
}

iTextureHandle *csMaterial::GetTexture ()
{
  iTextureWrapper* tex;
  GetVar (SVNames().diffuseTex)->GetValue (tex);
  if (tex) return tex->GetTextureHandle ();
  else return 0;
}

iTextureHandle* csMaterial::GetTexture (csStringID name)
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
  matEngine = SCF_QUERY_INTERFACE (material, iMaterialEngine);
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
  matEngine = SCF_QUERY_INTERFACE (material, iMaterialEngine);
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
  if (oldname) mat_hash.Delete (oldname, mat);
  if (newname) mat_hash.Put (newname, mat);
}

iMaterialWrapper *csMaterialList::NewMaterial (iMaterial *material,
	const char* name)
{
  csRef<iMaterialWrapper> tm;
  tm.AttachNew (new csMaterialWrapper (this, material));
  tm->QueryObject ()->SetName (name);
  if (name)
    mat_hash.Put (name, tm);
  list.Push (tm);
  tm->QueryObject ()->AddNameChangeListener (listener);
  return tm;
}

int csMaterialList::Add (iMaterialWrapper *obj)
{
  const char* name = obj->QueryObject ()->GetName ();
  if (name)
    mat_hash.Put (name, obj);
  obj->QueryObject ()->AddNameChangeListener (listener);
  return (int)list.Push (obj);
}

bool csMaterialList::Remove (iMaterialWrapper *obj)
{
  const char* name = obj->QueryObject ()->GetName ();
  if (name)
    mat_hash.Delete (name, obj);
  obj->QueryObject ()->RemoveNameChangeListener (listener);
  return list.Delete (obj);
}

bool csMaterialList::Remove (int n)
{
  iMaterialWrapper* obj = list[n];
  const char* name = obj->QueryObject ()->GetName ();
  if (name)
    mat_hash.Delete (name, obj);
  obj->QueryObject ()->RemoveNameChangeListener (listener);
  return list.DeleteIndex (n);
}

void csMaterialList::RemoveAll ()
{
  size_t i;
  for (i = 0 ; i < list.Length () ; i++)
    list[i]->QueryObject ()->RemoveNameChangeListener (listener);
  list.DeleteAll ();
  mat_hash.DeleteAll ();
}

int csMaterialList::Find (iMaterialWrapper *obj) const
{
  return (int)list.Find (obj);
}

iMaterialWrapper *csMaterialList::FindByName (const char *Name) const
{
  return mat_hash.Get (Name, 0);
}
