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
#include "csengine/texture.h"
#include "csengine/engine.h"
#include "ivideo/txtmgr.h"

IMPLEMENT_IBASE (csMaterial)
  IMPLEMENTS_INTERFACE (iMaterial)
IMPLEMENT_IBASE_END

csMaterial::csMaterial () :
  texture(0),
  num_texture_layers (0),
  texture_layer_wrapper (NULL),
  diffuse(CS_DEFMAT_DIFFUSE),
  ambient(CS_DEFMAT_AMBIENT),
  reflection(CS_DEFMAT_REFLECTION)
{
  CONSTRUCT_IBASE (NULL);
  flat_color.Set (255, 255, 255); // Default state is white, flat-shaded.
}

csMaterial::csMaterial (csTextureWrapper* w) :
  texture(w),
  num_texture_layers (0),
  texture_layer_wrapper (NULL),
  diffuse(CS_DEFMAT_DIFFUSE),
  ambient(CS_DEFMAT_AMBIENT),
  reflection(CS_DEFMAT_REFLECTION)
{
  CONSTRUCT_IBASE (NULL);
  flat_color.Set (255, 255, 255); // Default state is white, flat-shaded.
}

csMaterial::~csMaterial () 
{
//  delete texture;
}

iTextureHandle *csMaterial::GetTexture ()
{
  return texture ? texture->GetTextureHandle () : NULL;
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

void csMaterial::AddTextureLayer (csTextureWrapper* txtwrap, UInt mode,
      	int uscale, int vscale, int ushift, int vshift)
{
  if (num_texture_layers >= 1) return;
  num_texture_layers = 1;
  texture_layer_wrapper = txtwrap;
  texture_layer.mode = mode;
  texture_layer.uscale = uscale;
  texture_layer.vscale = vscale;
  texture_layer.ushift = ushift;
  texture_layer.vshift = vshift;
}

csTextureLayer* csMaterial::GetTextureLayer (int idx)
{
  if (num_texture_layers == 1)
  {
    texture_layer.txt_handle = texture_layer_wrapper->GetTextureHandle ();
    return &texture_layer;
  }
  else return NULL;
}

//---------------------------------------------------------------------------

IMPLEMENT_IBASE_EXT (csMaterialWrapper)
  IMPLEMENTS_EMBEDDED_INTERFACE (iMaterialWrapper)
IMPLEMENT_IBASE_EXT_END

IMPLEMENT_EMBEDDED_IBASE (csMaterialWrapper::MaterialWrapper)
  IMPLEMENTS_INTERFACE (iMaterialWrapper)
IMPLEMENT_EMBEDDED_IBASE_END

IMPLEMENT_CSOBJTYPE (csMaterialWrapper, csPObject);

csMaterialWrapper::csMaterialWrapper (iMaterial* material) :
  csPObject (), handle (NULL)
{
  CONSTRUCT_EMBEDDED_IBASE (scfiMaterialWrapper);
  csMaterialWrapper::material = material;
  material->IncRef ();
  csEngine::current_engine->AddToCurrentRegion (this);
}

csMaterialWrapper::csMaterialWrapper (csMaterialWrapper &th) :
  csPObject (), handle (NULL)
{
  CONSTRUCT_EMBEDDED_IBASE (scfiMaterialWrapper);
  (material = th.material)->IncRef ();
  handle = th.GetMaterialHandle ();
  SetName (th.GetName ());
  csEngine::current_engine->AddToCurrentRegion (this);
}

csMaterialWrapper::csMaterialWrapper (iMaterialHandle *ith) :
  csPObject (), material (NULL)
{
  CONSTRUCT_EMBEDDED_IBASE (scfiMaterialWrapper);
  ith->IncRef ();
  handle = ith;
  csEngine::current_engine->AddToCurrentRegion (this);
}

csMaterialWrapper::~csMaterialWrapper ()
{
  if (handle)
    handle->DecRef ();
  if (material)
    material->DecRef ();
}

void csMaterialWrapper::SetMaterial (iMaterial *material)
{
  if (csMaterialWrapper::material)
    csMaterialWrapper::material->DecRef ();
  csMaterialWrapper::material = material;
  material->IncRef ();
}

void csMaterialWrapper::Register (iTextureManager *txtmgr)
{
  handle = txtmgr->RegisterMaterial (material);
}

void csMaterialWrapper::Visit ()
{
  // @@@ This is not very clean! We shouldn't cast from iMaterial to csMaterial.
  csMaterial* mat = (csMaterial*)material;
  if (mat && mat->GetTextureWrapper ())
    mat->GetTextureWrapper ()->Visit ();
}

//------------------------------------------------------ csMaterialList -----//

IMPLEMENT_IBASE (csMaterialList)
  IMPLEMENTS_EMBEDDED_INTERFACE (iMaterialList)
IMPLEMENT_IBASE_END

IMPLEMENT_EMBEDDED_IBASE (csMaterialList::MaterialList)
  IMPLEMENTS_INTERFACE (iMaterialList)
IMPLEMENT_EMBEDDED_IBASE_END

csMaterialList::csMaterialList () : csNamedObjVector (16, 16)
{
  CONSTRUCT_IBASE (NULL);
  CONSTRUCT_EMBEDDED_IBASE (scfiMaterialList);
}

csMaterialList::~csMaterialList ()
{
  DeleteAll ();
}

csMaterialWrapper* csMaterialList::NewMaterial (iMaterial* material)
{
  csMaterialWrapper *tm = new csMaterialWrapper (material);
  Push (tm);
  return tm;
}

csMaterialWrapper* csMaterialList::NewMaterial (iMaterialHandle *ith)
{
  csMaterialWrapper *tm = new csMaterialWrapper (ith);
  Push (tm);
  return tm;
}
