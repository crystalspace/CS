/*
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
#include "itxtmgr.h"

IMPLEMENT_IBASE (csMaterial)
  IMPLEMENTS_INTERFACE (iMaterial)
IMPLEMENT_IBASE_END

csMaterial::csMaterial ()
{
  CONSTRUCT_IBASE (NULL);
  // set defaults
  // black flat shaded.
  flat_color.Set(0.f,0.f,0.f);
  texture = NULL;
  diffuse = 0.7;
  ambient = 0.0;
  reflection = 0.0;
}

csMaterial::csMaterial (csTextureHandle *txt)
{
  csMaterial ();
  texture = txt;
}

csMaterial::~csMaterial () 
{
//  delete texture;
}

iTextureHandle* csMaterial::GetTexture ()
{
  return GetTextureHandle ()->GetTextureHandle ();
}

//---------------------------------------------------------------------------

IMPLEMENT_CSOBJTYPE (csMaterialHandle, csObject);

csMaterialHandle::csMaterialHandle (iMaterial* material) :
  csObject (), handle (NULL)
{
  csMaterialHandle::material = material;
  material->IncRef ();
}

csMaterialHandle::csMaterialHandle (csMaterialHandle &th) :
  csObject (), handle (NULL)
{
  (material = th.material)->IncRef ();
  handle = th.GetMaterialHandle ();
  SetName (th.GetName ());
}

csMaterialHandle::csMaterialHandle (iMaterialHandle *ith) :
  csObject (), material (NULL)
{
  ith->IncRef ();
  handle = ith;
}

csMaterialHandle::~csMaterialHandle ()
{
  if (handle)
    handle->DecRef ();
  if (material)
    material->DecRef ();
}

void csMaterialHandle::SetMaterial (iMaterial *material)
{
  if (csMaterialHandle::material)
    csMaterialHandle::material->DecRef ();
  csMaterialHandle::material = material;
  material->IncRef ();
}

void csMaterialHandle::Register (iTextureManager *txtmgr)
{
  handle = txtmgr->RegisterMaterial (material);
}

//-------------------------------------------------------- csMaterialList -----//

csMaterialList::~csMaterialList ()
{
  DeleteAll ();
}

csMaterialHandle* csMaterialList::NewMaterial (iMaterial* material)
{
  csMaterialHandle *tm = new csMaterialHandle (material);
  Push (tm);
  return tm;
}

csMaterialHandle* csMaterialList::NewMaterial (iMaterialHandle *ith)
{
  csMaterialHandle *tm = new csMaterialHandle (ith);
  Push (tm);
  return tm;
}
