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
#include "csengine/material.h"
#include "csengine/engine.h"
#include "ivideo/txtmgr.h"
#include "ivideo/texture.h"
#include "iengine/texture.h"

SCF_IMPLEMENT_IBASE (csMaterial)
  SCF_IMPLEMENTS_INTERFACE (iMaterial)
SCF_IMPLEMENT_IBASE_END

csMaterial::csMaterial () :
  texture(NULL),
  num_texture_layers (0),
  diffuse(CS_DEFMAT_DIFFUSE),
  ambient(CS_DEFMAT_AMBIENT),
  reflection(CS_DEFMAT_REFLECTION)
{
  SCF_CONSTRUCT_IBASE (NULL);
  flat_color.Set (255, 255, 255); // Default state is white, flat-shaded.
}

csMaterial::csMaterial (iTextureWrapper* w) :
  texture(w),
  num_texture_layers (0),
  diffuse(CS_DEFMAT_DIFFUSE),
  ambient(CS_DEFMAT_AMBIENT),
  reflection(CS_DEFMAT_REFLECTION)
{
  SCF_CONSTRUCT_IBASE (NULL);
  flat_color.Set (255, 255, 255); // Default state is white, flat-shaded.
  if (texture)
    texture->IncRef ();
}

csMaterial::~csMaterial () 
{
  if (texture)
    texture->DecRef ();

  int i;
  for (i=0; i<num_texture_layers; i++)
    if (texture_layer_wrappers[i])
      texture_layer_wrappers[i]->DecRef ();
}

void csMaterial::SetTextureWrapper (iTextureWrapper *tex)
{
  if (texture)
    texture->DecRef();
  texture = tex;
  if (texture)
    texture->IncRef();
}

void csMaterial::AddTextureLayer (iTextureWrapper* txtwrap, UInt mode,
      	float uscale, float vscale, float ushift, float vshift)
{
  if (num_texture_layers >= 4) return;
  texture_layer_wrappers[num_texture_layers] = txtwrap;
  texture_layers[num_texture_layers].mode = mode;
  texture_layers[num_texture_layers].uscale = uscale;
  texture_layers[num_texture_layers].vscale = vscale;
  texture_layers[num_texture_layers].ushift = ushift;
  texture_layers[num_texture_layers].vshift = vshift;
  num_texture_layers++;

  txtwrap->IncRef ();
}

iTextureHandle *csMaterial::GetTexture ()
{
  return texture ? texture->GetTextureHandle () : NULL;
}

int csMaterial::GetTextureLayerCount ()
{
  return num_texture_layers;
}

csTextureLayer* csMaterial::GetTextureLayer (int idx)
{
  if (idx >= 0 && idx < num_texture_layers)
  {
    texture_layers[idx].txt_handle = texture_layer_wrappers[idx]->
    	GetTextureHandle ();
    return &texture_layers[idx];
  }
  else return NULL;
}

void csMaterial::GetFlatColor (csRGBpixel &oColor)
{
  oColor = flat_color;
  if (texture)
  {
    iTextureHandle *th = texture->GetTextureHandle ();
    if (th) th->GetMeanColor (oColor.red, oColor.green, oColor.blue);
  }
}

void csMaterial::GetReflection (float &oDiffuse, float &oAmbient,
  float &oReflection)
{
  oDiffuse = diffuse;
  oAmbient = ambient;
  oReflection = reflection;
}

//---------------------------------------------------------------------------

SCF_IMPLEMENT_IBASE_EXT (csMaterialWrapper)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iMaterialWrapper)
  SCF_IMPLEMENTS_INTERFACE (csMaterialWrapper)
SCF_IMPLEMENT_IBASE_EXT_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csMaterialWrapper::MaterialWrapper)
  SCF_IMPLEMENTS_INTERFACE (iMaterialWrapper)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

csMaterialWrapper::csMaterialWrapper (iMaterial* m) :
  csObject (), handle (NULL)
{
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiMaterialWrapper);
  material = m;
  material->IncRef ();

  // @@@ ??????
  csEngine::current_engine->AddToCurrentRegion (this);
}

csMaterialWrapper::csMaterialWrapper (iMaterialHandle *ith) :
  csObject (), material (NULL)
{
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiMaterialWrapper);

  handle = ith;
  handle->IncRef ();

  // @@@ ??????
  csEngine::current_engine->AddToCurrentRegion (this);
}

csMaterialWrapper::~csMaterialWrapper ()
{
  if (handle)
    handle->DecRef ();
  if (material)
    material->DecRef ();
}

void csMaterialWrapper::SetMaterial (iMaterial *m)
{
  if (material)
    material->DecRef ();
  material = m;
  material->IncRef ();
}

void csMaterialWrapper::SetMaterialHandle (iMaterialHandle *m)
{
  if (material)
    material->DecRef ();
  if (handle)
    handle->DecRef ();

  material = NULL;
  handle = m;
  handle->IncRef ();
}

void csMaterialWrapper::Register (iTextureManager *txtmgr)
{
  if (handle)
    handle->DecRef ();
  handle = txtmgr->RegisterMaterial (material);
}

void csMaterialWrapper::Visit ()
{
  // @@@ This is not very clean! We shouldn't cast from iMaterial to csMaterial.
  // @@@ This is also not up-to-date because it doesn't deal with layers
  csMaterial* mat = (csMaterial*)material;
  if (mat && mat->GetTextureWrapper ())
    mat->GetTextureWrapper ()->Visit ();
}

//------------------------------------------------------ csMaterialList -----//

SCF_IMPLEMENT_IBASE (csMaterialList)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iMaterialList)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csMaterialList::MaterialList)
  SCF_IMPLEMENTS_INTERFACE (iMaterialList)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

csMaterialList::csMaterialList () : csMaterialListHelper (16, 16)
{
  SCF_CONSTRUCT_IBASE (NULL);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiMaterialList);
}

iMaterialWrapper* csMaterialList::NewMaterial (iMaterial* material)
{
  iMaterialWrapper *tm = &(new csMaterialWrapper (material))->scfiMaterialWrapper;
  Push (tm);
  tm->DecRef ();
  return tm;
}

iMaterialWrapper* csMaterialList::NewMaterial (iMaterialHandle *ith)
{
  iMaterialWrapper *tm = &(new csMaterialWrapper (ith))->scfiMaterialWrapper;
  Push (tm);
  tm->DecRef ();
  return tm;
}

iMaterialWrapper* csMaterialList::MaterialList::NewMaterial (iMaterial* material)
  { return scfParent->NewMaterial (material); }
iMaterialWrapper* csMaterialList::MaterialList::NewMaterial (iMaterialHandle *ith)
  { return scfParent->NewMaterial (ith); }
int csMaterialList::MaterialList::GetCount () const
  { return scfParent->Length (); }
iMaterialWrapper *csMaterialList::MaterialList::Get (int n) const
  { return scfParent->Get (n); }
int csMaterialList::MaterialList::Add (iMaterialWrapper *obj)
  { return scfParent->Push (obj); }
bool csMaterialList::MaterialList::Remove (iMaterialWrapper *obj)
  { return scfParent->Delete (obj); }
bool csMaterialList::MaterialList::Remove (int n)
  { return scfParent->Delete (n); }
void csMaterialList::MaterialList::RemoveAll ()
  { scfParent->DeleteAll (); }
int csMaterialList::MaterialList::Find (iMaterialWrapper *obj) const
  { return scfParent->Find (obj); }
iMaterialWrapper *csMaterialList::MaterialList::FindByName (const char *Name) const
  { return scfParent->FindByName (Name); }
