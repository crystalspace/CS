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
#include "isomater.h"
#include "ivideo/txtmgr.h"
#include "ivideo/texture.h"

SCF_IMPLEMENT_IBASE (csIsoMaterial)
  SCF_IMPLEMENTS_INTERFACE (iMaterial)
SCF_IMPLEMENT_IBASE_END

csIsoMaterial::csIsoMaterial () :
  texture(0),
  diffuse(CS_DEFMAT_DIFFUSE),
  ambient(CS_DEFMAT_AMBIENT),
  reflection(CS_DEFMAT_REFLECTION)
{
  SCF_CONSTRUCT_IBASE (NULL);
  flat_color.Set (255, 255, 255); // Default state is white, flat-shaded.
}

csIsoMaterial::csIsoMaterial (iTextureHandle* w) :
  texture(w),
  diffuse(CS_DEFMAT_DIFFUSE),
  ambient(CS_DEFMAT_AMBIENT),
  reflection(CS_DEFMAT_REFLECTION)
{
  SCF_CONSTRUCT_IBASE (NULL);
  flat_color.Set (255, 255, 255); // Default state is white, flat-shaded.
}

csIsoMaterial::~csIsoMaterial ()
{
//  delete texture;
}

iTextureHandle *csIsoMaterial::GetTexture ()
{
  return texture;
}

void csIsoMaterial::GetFlatColor (csRGBpixel &oColor)
{
  oColor = flat_color;
  if (texture)
  {
    iTextureHandle *th = texture;
    if (th) th->GetMeanColor (oColor.red, oColor.green, oColor.blue);
  }
}

void csIsoMaterial::GetReflection (float &oDiffuse, float &oAmbient,
  float &oReflection)
{
  oDiffuse = diffuse;
  oAmbient = ambient;
  oReflection = reflection;
}

//---------------------------------------------------------------------------

SCF_IMPLEMENT_IBASE_EXT (csIsoMaterialWrapper)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iMaterialWrapper)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iIsoMaterialWrapperIndex)
SCF_IMPLEMENT_IBASE_EXT_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csIsoMaterialWrapper::MaterialWrapper)
  SCF_IMPLEMENTS_INTERFACE (iMaterialWrapper)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csIsoMaterialWrapper::IsoMaterialWrapperIndex)
  SCF_IMPLEMENTS_INTERFACE (iIsoMaterialWrapperIndex)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

csIsoMaterialWrapper::csIsoMaterialWrapper (iMaterial* material) :
  csObject (), handle (NULL)
{
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiMaterialWrapper);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiIsoMaterialWrapperIndex);
  csIsoMaterialWrapper::material = material;
  material->IncRef ();
  index = 0;
  //csEngine::current_engine->AddToCurrentRegion (this);
}

csIsoMaterialWrapper::csIsoMaterialWrapper (csIsoMaterialWrapper &th) :
  csObject (), handle (NULL)
{
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiMaterialWrapper);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiIsoMaterialWrapperIndex);
  (material = th.material)->IncRef ();
  handle = th.GetMaterialHandle ();
  SetName (th.GetName ());
  index = th.index;
  //csEngine::current_engine->AddToCurrentRegion (this);
}

csIsoMaterialWrapper::csIsoMaterialWrapper (iMaterialHandle *ith) :
  csObject (), material (NULL)
{
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiMaterialWrapper);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiIsoMaterialWrapperIndex);
  ith->IncRef ();
  handle = ith;
  index = 0;
  //csEngine::current_engine->AddToCurrentRegion (this);
}

csIsoMaterialWrapper::~csIsoMaterialWrapper ()
{
  if (handle)
    handle->DecRef ();
  if (material)
    material->DecRef ();
}

void csIsoMaterialWrapper::SetMaterialHandle (iMaterialHandle *m)
{
  if (handle)
    handle->DecRef ();
  if (material)
    material->DecRef ();

  material = NULL;
  handle = m;
  handle->IncRef ();
}

void csIsoMaterialWrapper::SetMaterial (iMaterial *material)
{
  if (csIsoMaterialWrapper::material)
    csIsoMaterialWrapper::material->DecRef ();
  csIsoMaterialWrapper::material = material;
  material->IncRef ();
}

void csIsoMaterialWrapper::Register (iTextureManager *txtmgr)
{
  handle = txtmgr->RegisterMaterial (material);
}

void csIsoMaterialWrapper::Visit ()
{
  // @@@ This is not very clean! We shouldn't cast from iMaterial to csMaterial.
  //csMaterial* mat = (csMaterial*)material;
  //if (mat && mat->GetTextureWrapper ())
    //mat->GetTextureWrapper ()->Visit ();
}

//------------------------------------------------------ csMaterialList -----//

SCF_IMPLEMENT_IBASE (csIsoMaterialList)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iMaterialList)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csIsoMaterialList::MaterialList)
  SCF_IMPLEMENTS_INTERFACE (iMaterialList)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

csIsoMaterialList::csIsoMaterialList () : csNamedObjVector (16, 16)
{
  SCF_CONSTRUCT_IBASE (NULL);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiMaterialList);
  lastindex = 0;
}

csIsoMaterialList::~csIsoMaterialList ()
{
  DeleteAll ();
}

int csIsoMaterialList::GetNewIndex()
{
  int i = lastindex;
  while(i < Length())
  {
    if(Get(i)==NULL)
    {
      lastindex = i+1;
      return i;
    }
    i++;
  }
  /// no indices free
  Push(NULL);
  lastindex = Length();
  return lastindex-1;
}

void csIsoMaterialList::RemoveIndex(int i)
{
  CS_ASSERT (i >= 0);
  if(i>=Length()) return;

  iMaterialWrapper* mw = (iMaterialWrapper*)(*this)[i];
  mw->DecRef ();
  (*this)[i] = NULL;

  if(i==Length()-1)
  {
    (void)Pop(); // pop last element from the list
    if(Length()<lastindex) lastindex = Length();
    return;
  }
  /// remove from middle of list
  if(i<lastindex) lastindex = i;
}

csIsoMaterialWrapper* csIsoMaterialList::NewMaterial (iMaterial* material)
{
  csIsoMaterialWrapper *tm = new csIsoMaterialWrapper (material);
  int i = GetNewIndex();
  (*this)[i] = tm;
  tm->SetIndex(i);
  return tm;
}

csIsoMaterialWrapper* csIsoMaterialList::NewMaterial (iMaterialHandle *ith)
{
  csIsoMaterialWrapper *tm = new csIsoMaterialWrapper (ith);
  int i = GetNewIndex();
  (*this)[i] = tm;
  tm->SetIndex(i);
  return tm;
}
