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

CS_LEAKGUARD_IMPLEMENT (csMaterial)
CS_LEAKGUARD_IMPLEMENT (csMaterialWrapper)

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

csStringID csMaterial::nameTextureLayer1;
csStringID csMaterial::nameTextureLayer2;
csStringID csMaterial::nameTextureLayer3;
csStringID csMaterial::nameTextureLayer4;

csMaterial::csMaterial (csEngine* engine)
{
  SCF_CONSTRUCT_IBASE (0);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiMaterialEngine);

#ifdef CS_USE_NEW_RENDERER
  csMaterial::engine = engine;
#endif // CS_USE_NEW_RENDERER

  nameDiffuseParam = engine->Strings->Request (CS_MATERIAL_VARNAME_DIFFUSE);
  nameAmbientParam = engine->Strings->Request (CS_MATERIAL_VARNAME_AMBIENT);
  nameReflectParam = engine->Strings->Request (CS_MATERIAL_VARNAME_REFLECTION);
  nameFlatColorParam = engine->Strings->Request (CS_MATERIAL_VARNAME_FLATCOLOR);
  nameDiffuseTexture = engine->Strings->Request (CS_MATERIAL_TEXTURE_DIFFUSE);

  nameTextureLayer1 = engine->Strings->Request (CS_MATERIAL_TEXTURE_LAYER1);
  nameTextureLayer2 = engine->Strings->Request (CS_MATERIAL_TEXTURE_LAYER2);
  nameTextureLayer3 = engine->Strings->Request (CS_MATERIAL_TEXTURE_LAYER3);
  nameTextureLayer4 = engine->Strings->Request (CS_MATERIAL_TEXTURE_LAYER4);

  SetTextureWrapper (0);
  // @@@ This will force the shader vars to be created...
  // @@@ So you can't globally override them
  SetFlatColor (csRGBcolor (255, 255, 255));
  SetDiffuse (CS_DEFMAT_DIFFUSE);
  SetAmbient (CS_DEFMAT_AMBIENT);
  SetReflection (CS_DEFMAT_REFLECTION);
  shadersCustomized = false;

#ifndef CS_USE_NEW_RENDERER
  num_texture_layers = 0;
#endif
}

csMaterial::csMaterial (csEngine* engine,
			iTextureWrapper *w)
{
  SCF_CONSTRUCT_IBASE (0);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiMaterialEngine);

#ifdef CS_USE_NEW_RENDERER
  csMaterial::engine = engine;

  nameDiffuseParam = engine->Strings->Request (CS_MATERIAL_VARNAME_DIFFUSE);
  nameAmbientParam = engine->Strings->Request (CS_MATERIAL_VARNAME_AMBIENT);
  nameReflectParam = engine->Strings->Request (CS_MATERIAL_VARNAME_REFLECTION);
  nameFlatColorParam = engine->Strings->Request (CS_MATERIAL_VARNAME_FLATCOLOR);
  nameDiffuseTexture = engine->Strings->Request (CS_MATERIAL_TEXTURE_DIFFUSE);
#endif // CS_USE_NEW_RENDERER

  SetTextureWrapper (w);
  SetFlatColor (csRGBcolor (255, 255, 255));
  SetDiffuse (CS_DEFMAT_DIFFUSE);
  SetAmbient (CS_DEFMAT_AMBIENT);
  SetReflection (CS_DEFMAT_REFLECTION);
  shadersCustomized = false;

#ifndef CS_USE_NEW_RENDERER
  num_texture_layers = 0;
#endif
}

csMaterial::~csMaterial ()
{
  SCF_DESTRUCT_EMBEDDED_IBASE (scfiMaterialEngine);
  SCF_DESTRUCT_IBASE ();
}

#ifdef CS_USE_NEW_RENDERER
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
#endif

#ifndef CS_USE_NEW_RENDERER
csRGBcolor& csMaterial::GetFlatColor ()
{ 
  return flat_color; 
}
  
float csMaterial::GetDiffuse () 
{ 
  return diffuse; 
}

void csMaterial::SetDiffuse (float val) 
{ 
  diffuse = val; 
}

float csMaterial::GetAmbient () 
{ 
  return ambient; 
}

void csMaterial::SetAmbient (float val) 
{ 
  ambient = val; 
}

float csMaterial::GetReflection () 
{ 
  return reflection; 
}

void csMaterial::SetReflection (float val) 
{ 
  reflection = val; 
}

#else

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
  iTextureHandle* texh = GetTexture (nameDiffuseTexture);
  if (useTextureMean && texh)
  {
    texh->GetMeanColor (oColor.red, oColor.green, oColor.blue);
  }
  else
  {
    oColor = GetFlatColor ();
  }
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
#endif


void csMaterial::SetTextureWrapper (iTextureWrapper *tex)
{
#ifdef CS_USE_NEW_RENDERER
  SetTextureWrapper (nameDiffuseTexture, tex);
#else
  texture = tex;
#endif
}


iTextureWrapper* csMaterial::GetTextureWrapper (csStringID name)
{
#ifdef CS_USE_NEW_RENDERER
  iTextureWrapper* tex;
  GetVar (name)->GetValue (tex);
  return tex;
  //return texWrappers.Get (name);
#else
  if (name == nameDiffuseTexture) return texture;
  if (name == nameTextureLayer1) return texture_layer_wrappers[0];
  if (name == nameTextureLayer2) return texture_layer_wrappers[1];
  if (name == nameTextureLayer3) return texture_layer_wrappers[2];
  if (name == nameTextureLayer4) return texture_layer_wrappers[3];
  return 0;
#endif
}

#ifdef CS_USE_NEW_RENDERER

void csMaterial::SetTextureWrapper (csStringID name, iTextureWrapper* tex)
{
  csShaderVariable* var = GetVar (name, true);
  var->SetValue (tex);
  /*texWrappers.Put (name, tex);
  texHandles.Put (name, tex ? tex->GetTextureHandle () : 0);*/
}

#endif

#ifndef CS_USE_NEW_RENDERER
void csMaterial::AddTextureLayer (
  iTextureWrapper *txtwrap,
  uint mode,
  float uscale,
  float vscale,
  float ushift,
  float vshift)
{
  if (num_texture_layers >= 4) return ;
  texture_layer_wrappers[num_texture_layers] = txtwrap;
  texture_layers[num_texture_layers].mode = mode;
  texture_layers[num_texture_layers].uscale = uscale;
  texture_layers[num_texture_layers].vscale = vscale;
  texture_layers[num_texture_layers].ushift = ushift;
  texture_layers[num_texture_layers].vshift = vshift;
  num_texture_layers++;
}
#endif

void csMaterial::SetShader (csStringID type, iShader* shd)
{
#ifdef CS_USE_NEW_RENDERER
  if (!shadersCustomized)
  {
    shaders.PutFirst (engine->default_shadertype, 0);
    shadersCustomized = true;
  }

  shaders.PutFirst (type, shd);
#else
  (void)type;
  (void)shd;
#endif
}

iShader* csMaterial::GetShader(csStringID type)
{
#ifdef CS_USE_NEW_RENDERER
  return shaders.Get (type, 0);
#else
  return 0;
#endif
}

iTextureHandle *csMaterial::GetTexture ()
{
#ifndef CS_USE_NEW_RENDERER
  return texture ? texture->GetTextureHandle () : 0;
#else
  iTextureWrapper* tex;
  GetVar (nameDiffuseTexture)->GetValue (tex);
  return tex->GetTextureHandle ();

  /*iTextureHandle* texture = texHandles.Get (nameDiffuseTexture);
  return texture;*/
#endif
}

iTextureHandle* csMaterial::GetTexture (csStringID name)
{
#ifdef CS_USE_NEW_RENDERER
  iTextureWrapper* tex;
  csShaderVariable* var = GetVar (name);
  if (var)
  {
    var->GetValue (tex);
    return tex ? tex->GetTextureHandle () : 0;
  }
  //return texHandles.Get (name);
#else
  if (name == nameDiffuseTexture)
    return texture ? texture->GetTextureHandle () : 0;
  if (name == nameTextureLayer1) return texture_layers[0].txt_handle;
  if (name == nameTextureLayer2) return texture_layers[1].txt_handle;
  if (name == nameTextureLayer3) return texture_layers[2].txt_handle;
  if (name == nameTextureLayer4) return texture_layers[3].txt_handle;
#endif
  return 0;
}

#ifndef CS_USE_NEW_RENDERER
int csMaterial::GetTextureLayerCount ()
{
  return num_texture_layers;
}

csTextureLayer *csMaterial::GetTextureLayer (int idx)
{
  if (idx >= 0 && idx < num_texture_layers)
  {
    texture_layers[idx].txt_handle = texture_layer_wrappers[idx]
    	->GetTextureHandle ();
    return &texture_layers[idx];
  }
  else
    return 0;
}

void csMaterial::GetFlatColor (csRGBpixel &oColor, bool useTextureMean)
{
  oColor = flat_color;
  if (texture && useTextureMean)
  {
    iTextureHandle *th = texture->GetTextureHandle ();
    if (th) th->GetMeanColor (oColor.red, oColor.green, oColor.blue);
  }
}


void csMaterial::SetFlatColor (const csRGBcolor& col)
{ 
  flat_color = col; 
}

void csMaterial::GetReflection (
  float &oDiffuse,
  float &oAmbient,
  float &oReflection)
{
  oDiffuse = diffuse;
  oAmbient = ambient;
  oReflection = reflection;
}

void csMaterial::SetReflection (float oDiffuse, float oAmbient,
  float oReflection)
{
  diffuse = oDiffuse;
  ambient = oAmbient;
  reflection = oReflection;
}
#endif

void csMaterial::Visit ()
{
#ifdef CS_USE_NEW_RENDERER
  // @@@ Implement me!!!!!!!!!!
#else
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
#ifdef CS_USE_NEW_RENDERER
  // @@@ Implement me!!!!!!!!!!
  return false;
#else
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

csMaterialWrapper::csMaterialWrapper (iMaterialHandle *ith) : csObject()
{
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiMaterialWrapper);

  handle = ith;
}

csMaterialWrapper::csMaterialWrapper (csMaterialWrapper &w) : csObject(w)
{
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiMaterialWrapper);

  material = w.material;
  matEngine = w.matEngine;
  handle = w.handle;
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

void csMaterialWrapper::SetMaterialHandle (iMaterialHandle *m)
{
  material = 0;
  matEngine = 0;

  handle = m;
}

void csMaterialWrapper::Register (iTextureManager *txtmgr)
{
  handle = txtmgr->RegisterMaterial (material);
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
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE(iMaterialList)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csMaterialList::MaterialList)
  SCF_IMPLEMENTS_INTERFACE(iMaterialList)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

csMaterialList::csMaterialList () :
  csRefArrayObject<iMaterialWrapper> (16, 16)
{
  SCF_CONSTRUCT_IBASE (0);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiMaterialList);
}

csMaterialList::~csMaterialList()
{
  SCF_DESTRUCT_EMBEDDED_IBASE (scfiMaterialList);
  SCF_DESTRUCT_IBASE ();
}

iMaterialWrapper *csMaterialList::NewMaterial (iMaterial *material)
{
  iMaterialWrapper *tm = &
    (new csMaterialWrapper (material))->scfiMaterialWrapper;
  Push (tm);
  tm->DecRef ();
  return tm;
}

iMaterialWrapper *csMaterialList::NewMaterial (iMaterialHandle *ith)
{
  iMaterialWrapper *tm = &(new csMaterialWrapper (ith))->scfiMaterialWrapper;
  Push (tm);
  tm->DecRef ();
  return tm;
}

iMaterialWrapper *csMaterialList::MaterialList::NewMaterial (
  iMaterial *material)
{
  return scfParent->NewMaterial (material);
}

iMaterialWrapper *csMaterialList::MaterialList::NewMaterial (
  iMaterialHandle *ith)
{
  return scfParent->NewMaterial (ith);
}

int csMaterialList::MaterialList::GetCount () const
{
  return scfParent->Length ();
}

iMaterialWrapper *csMaterialList::MaterialList::Get (int n) const
{
  return scfParent->Get (n);
}

int csMaterialList::MaterialList::Add (iMaterialWrapper *obj)
{
  return scfParent->Push (obj);
}

bool csMaterialList::MaterialList::Remove (iMaterialWrapper *obj)
{
  return scfParent->Delete (obj);
}

bool csMaterialList::MaterialList::Remove (int n)
{
  return scfParent->DeleteIndex (n);
}

void csMaterialList::MaterialList::RemoveAll ()
{
  scfParent->DeleteAll ();
}

int csMaterialList::MaterialList::Find (iMaterialWrapper *obj) const
{
  return scfParent->Find (obj);
}

iMaterialWrapper *csMaterialList::MaterialList::FindByName (
  const char *Name) const
{
  return scfParent->FindByName (Name);
}
