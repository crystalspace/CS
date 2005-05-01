/*
    Copyright (C) 2001 by Jorrit Tyberghein
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
#include "plugins/engine/3d/material.h"
#include "plugins/engine/3d/engine.h"
#include "csutil/debug.h"
#include "ivideo/txtmgr.h"
#include "ivideo/texture.h"
#include "iengine/texture.h"

CS_LEAKGUARD_IMPLEMENT (csMaterial);
CS_LEAKGUARD_IMPLEMENT (csMaterialWrapper);

SCF_IMPLEMENT_IBASE(csMaterial)
  SCF_IMPLEMENTS_INTERFACE(iMaterial)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE(iMaterialEngine)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csMaterial::MaterialEngine)
  SCF_IMPLEMENTS_INTERFACE(iMaterialEngine)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

csStringID csMaterial::nameDiffuseParam;
csStringID csMaterial::nameAmbientParam;
csStringID csMaterial::nameReflectParam;
csStringID csMaterial::nameFlatColorParam;
csStringID csMaterial::nameDiffuseTexture;

csMaterial::csMaterial (csEngine* engine)
{
  SCF_CONSTRUCT_IBASE (0);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiMaterialEngine);

  csMaterial::engine = engine;


  nameDiffuseParam = engine->Strings->Request (CS_MATERIAL_VARNAME_DIFFUSE);
  nameAmbientParam = engine->Strings->Request (CS_MATERIAL_VARNAME_AMBIENT);
  nameReflectParam = engine->Strings->Request (CS_MATERIAL_VARNAME_REFLECTION);
  nameFlatColorParam = engine->Strings->Request (CS_MATERIAL_VARNAME_FLATCOLOR);
  nameDiffuseTexture = engine->Strings->Request (CS_MATERIAL_TEXTURE_DIFFUSE);

  SetTextureWrapper (0);
  // @@@ This will force the shader vars to be created...
  // @@@ So you can't globally override them
  SetFlatColor (csRGBcolor (255, 255, 255));
  SetDiffuse (CS_DEFMAT_DIFFUSE);
  SetAmbient (CS_DEFMAT_AMBIENT);
  SetReflection (CS_DEFMAT_REFLECTION);
}

csMaterial::csMaterial (csEngine* engine,
			iTextureWrapper *w)
{
  SCF_CONSTRUCT_IBASE (0);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiMaterialEngine);

  csMaterial::engine = engine;

  nameDiffuseParam = engine->Strings->Request (CS_MATERIAL_VARNAME_DIFFUSE);
  nameAmbientParam = engine->Strings->Request (CS_MATERIAL_VARNAME_AMBIENT);
  nameReflectParam = engine->Strings->Request (CS_MATERIAL_VARNAME_REFLECTION);
  nameFlatColorParam = engine->Strings->Request (CS_MATERIAL_VARNAME_FLATCOLOR);
  nameDiffuseTexture = engine->Strings->Request (CS_MATERIAL_TEXTURE_DIFFUSE);


  SetTextureWrapper (w);
  SetFlatColor (csRGBcolor (255, 255, 255));
  SetDiffuse (CS_DEFMAT_DIFFUSE);
  SetAmbient (CS_DEFMAT_AMBIENT);
  SetReflection (CS_DEFMAT_REFLECTION);
}

csMaterial::~csMaterial ()
{
  SCF_DESTRUCT_EMBEDDED_IBASE (scfiMaterialEngine);
  SCF_DESTRUCT_IBASE ();
}

csShaderVariable* csMaterial::GetVar (csStringID name, bool create)
{
  csRef<csShaderVariable> var = GetVariable (name);
  if ((var == 0) && create)
  {
    var.AttachNew (new csShaderVariable (name));
    AddVariable (var);
  }
  return var;
}

csRGBcolor& csMaterial::GetFlatColor ()
{ 
  csShaderVariable* var = GetVar (nameFlatColorParam);
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
  return flat_color;
}
  
void csMaterial::GetFlatColor (csRGBpixel &oColor, bool useTextureMean)
{
  oColor = GetFlatColor ();
}

float csMaterial::GetDiffuse ()
{ 
  csShaderVariable* var = GetVar (nameDiffuseParam);
  if (var == 0) return CS_DEFMAT_DIFFUSE;
  float f;
  var->GetValue (f);
  return f; 
}

void csMaterial::SetDiffuse (float val) 
{ 
  csShaderVariable* var = GetVar (nameDiffuseParam, true);
  var->SetValue (val);
}

float csMaterial::GetAmbient ()
{ 
  csShaderVariable* var = GetVar (nameAmbientParam);
  if (var == 0) return CS_DEFMAT_AMBIENT;
  float f;
  var->GetValue (f);
  return f; 
}

void csMaterial::SetAmbient (float val) 
{ 
  csShaderVariable* var = GetVar (nameAmbientParam, true);
  var->SetValue (val);
}

float csMaterial::GetReflection ()
{ 
  csShaderVariable* var = GetVar (nameReflectParam);
  if (var == 0) return CS_DEFMAT_REFLECTION;
  float f;
  var->GetValue (f);
  return f; 
}

void csMaterial::SetReflection (float val) 
{ 
  csShaderVariable* var = GetVar (nameReflectParam, true);
  var->SetValue (val);
}

void csMaterial::SetFlatColor (const csRGBcolor& col)
{ 
  csShaderVariable* var = GetVar (nameFlatColorParam, true);
  csVector3 v (((float)col.red) / 255.0f, ((float)col.green) / 255.0f, 
    ((float)col.blue) / 255.0f);
  var->SetValue (v);
}

void csMaterial::GetReflection (
  float &oDiffuse,
  float &oAmbient,
  float &oReflection)
{
  oDiffuse = GetDiffuse ();
  oAmbient = GetAmbient ();
  oReflection = GetReflection ();
}

void csMaterial::SetReflection (float oDiffuse, float oAmbient,
  float oReflection)
{
  SetDiffuse (oDiffuse);
  SetDiffuse (oAmbient);
  SetReflection (oReflection);
}


void csMaterial::SetTextureWrapper (iTextureWrapper *tex)
{
  SetTextureWrapper (nameDiffuseTexture, tex);
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
  GetVar (nameDiffuseTexture)->GetValue (tex);
  return tex->GetTextureHandle ();
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
SCF_IMPLEMENT_IBASE_EXT(csMaterialWrapper)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE(iMaterialWrapper)
  SCF_IMPLEMENTS_INTERFACE(csMaterialWrapper)
SCF_IMPLEMENT_IBASE_EXT_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csMaterialWrapper::MaterialWrapper)
  SCF_IMPLEMENTS_INTERFACE(iMaterialWrapper)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

csMaterialWrapper::csMaterialWrapper (iMaterial *m) : csObject()
{
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiMaterialWrapper);
  material = m;
  matEngine = SCF_QUERY_INTERFACE (material, iMaterialEngine);
}

csMaterialWrapper::csMaterialWrapper (csMaterialWrapper &w) : csObject(w)
{
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiMaterialWrapper);

  material = w.material;
  matEngine = w.matEngine;
}

csMaterialWrapper::~csMaterialWrapper ()
{
  SCF_DESTRUCT_EMBEDDED_IBASE (scfiMaterialWrapper);
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
SCF_IMPLEMENT_IBASE(csMaterialList)
  SCF_IMPLEMENTS_INTERFACE(iMaterialList)
SCF_IMPLEMENT_IBASE_END

csMaterialList::csMaterialList ()
{
  SCF_CONSTRUCT_IBASE (0);
}

csMaterialList::~csMaterialList()
{
  SCF_DESTRUCT_IBASE ();
}

iMaterialWrapper *csMaterialList::NewMaterial (iMaterial *material,
	const char* name)
{
  iMaterialWrapper *tm = &
    (new csMaterialWrapper (material))->scfiMaterialWrapper;
  tm->QueryObject ()->SetName (name);
  if (name)
    mat_hash.Put (name, tm);
  list.Push (tm);
  tm->DecRef ();
  return tm;
}

int csMaterialList::Add (iMaterialWrapper *obj)
{
  const char* name = obj->QueryObject ()->GetName ();
  if (name)
    mat_hash.Put (name, obj);
  return (int)list.Push (obj);
}

bool csMaterialList::Remove (iMaterialWrapper *obj)
{
  const char* name = obj->QueryObject ()->GetName ();
  if (name)
    mat_hash.Delete (name, obj);
  return list.Delete (obj);
}

bool csMaterialList::Remove (int n)
{
  iMaterialWrapper* obj = list[n];
  const char* name = obj->QueryObject ()->GetName ();
  if (name)
    mat_hash.Delete (name, obj);
  return list.DeleteIndex (n);
}

void csMaterialList::RemoveAll ()
{
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
