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
  diffuse(CS_DEFMAT_DIFFUSE),
  ambient(CS_DEFMAT_AMBIENT),
  reflection(CS_DEFMAT_REFLECTION)
{
  SCF_CONSTRUCT_IBASE (0);
  flat_color.Set (255, 255, 255); // Default state is white, flat-shaded.
}

csIsoMaterial::csIsoMaterial (iTextureHandle* w) :
  diffuse(CS_DEFMAT_DIFFUSE),
  ambient(CS_DEFMAT_AMBIENT),
  reflection(CS_DEFMAT_REFLECTION)
{
  SCF_CONSTRUCT_IBASE (0);
  flat_color.Set (255, 255, 255); // Default state is white, flat-shaded.
  texture = w;
}

csIsoMaterial::~csIsoMaterial ()
{
  SCF_DESTRUCT_IBASE();
}

iTextureHandle *csIsoMaterial::GetTexture ()
{
  return texture;
}

iTextureHandle *csIsoMaterial::GetTexture (csStringID)
{
  return texture;
}

void csIsoMaterial::GetFlatColor (csRGBpixel &oColor, bool useTextureMean)
{
  oColor = flat_color;
  if (texture && useTextureMean)
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
  csObject ()
{
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiMaterialWrapper);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiIsoMaterialWrapperIndex);
  csIsoMaterialWrapper::material = material;
  index = 0;
}

csIsoMaterialWrapper::csIsoMaterialWrapper (csIsoMaterialWrapper &th) :
  csObject ()
{
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiMaterialWrapper);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiIsoMaterialWrapperIndex);
  material = th.material;
  handle = th.GetMaterialHandle ();
  SetName (th.GetName ());
  index = th.index;
}

csIsoMaterialWrapper::csIsoMaterialWrapper (iMaterialHandle *ith) :
  csObject ()
{
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiMaterialWrapper);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiIsoMaterialWrapperIndex);
  handle = ith;
  index = 0;
}

csIsoMaterialWrapper::~csIsoMaterialWrapper ()
{
  SCF_DESTRUCT_EMBEDDED_IBASE (scfiIsoMaterialWrapperIndex);
  SCF_DESTRUCT_EMBEDDED_IBASE (scfiMaterialWrapper);
}

void csIsoMaterialWrapper::SetMaterialHandle (iMaterialHandle *m)
{
  material = 0;
  handle = m;
}

void csIsoMaterialWrapper::SetMaterial (iMaterial *material)
{
  csIsoMaterialWrapper::material = material;
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

csIsoMaterialList::csIsoMaterialList () :
	csRefArray<csIsoMaterialWrapper> (16, 16)
{
  SCF_CONSTRUCT_IBASE (0);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiMaterialList);
  lastindex = 0;
}

csIsoMaterialList::~csIsoMaterialList ()
{
  DeleteAll ();
  SCF_DESTRUCT_EMBEDDED_IBASE (scfiMaterialList);
  SCF_DESTRUCT_IBASE();
}

int csIsoMaterialList::GetNewIndex()
{
  int i = lastindex;
  while(i < Length())
  {
    if(Get(i)==0)
    {
      lastindex = i+1;
      return i;
    }
    i++;
  }
  /// no indices free
  Push(0);
  lastindex = Length();
  return lastindex-1;
}

void csIsoMaterialList::RemoveIndex(int i)
{
  CS_ASSERT (i >= 0);
  if(i>=Length()) return;

  (*this)[i] = 0;

  if(i==Length()-1)
  {
    csRef<csIsoMaterialWrapper> w = Pop(); // pop last element from the list
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

csIsoMaterialWrapper* csIsoMaterialList::FindByName (const char* name)
{
  int i;
  for (i = 0 ; i < Length () ; i++)
  {
    csIsoMaterialWrapper* w = (*this)[i];
    if (!strcmp (w->GetName (), name))
    {
      return w;
    }
  }
  return 0;
}

csIsoMaterialWrapper* csIsoMaterialList::NewMaterial (iMaterialHandle *ith)
{
  csIsoMaterialWrapper *tm = new csIsoMaterialWrapper (ith);
  int i = GetNewIndex();
  (*this)[i] = tm;
  tm->SetIndex(i);
  return tm;
}
