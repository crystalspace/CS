/*
    Copyright (C) 1998 by Jorrit Tyberghein

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
#include <math.h>
#include "plugins/engine/3d/texture.h"
#include "plugins/engine/3d/engine.h"
#include "igraphic/image.h"
#include "ivideo/txtmgr.h"

CS_LEAKGUARD_IMPLEMENT (csTextureWrapper);

//---------------------------------------------------------------------------

csTextureWrapper::csTextureWrapper (iImage *Image)
  : scfImplementationType (this),
  flags(CS_TEXTURE_3D)
{
  image = Image;
  keep_image = false;
  texClass = 0;
  UpdateKeyColorFromImage ();
}

csTextureWrapper::csTextureWrapper (iTextureHandle *ith) 
  : scfImplementationType (this)
{
  keep_image = false;
  texClass = 0;

  handle = ith;
  if (handle)
  {
    flags = ith->GetFlags ();
  }
  else
  {
    flags = 0;
  }

  UpdateKeyColorFromHandle ();
}

csTextureWrapper::csTextureWrapper (const csTextureWrapper &t) :
  iBase(), scfImplementationType (this),
  flags(CS_TEXTURE_3D)
{
  handle = t.handle;
  image = t.image;
  keep_image = t.keep_image;
  if (!handle)
    texClass = csStrNew (t.texClass);
  else
    texClass = 0;

  UpdateKeyColorFromImage ();
}

csTextureWrapper::~csTextureWrapper ()
{

  delete[] texClass;
}

void csTextureWrapper::SetImageFile (iImage *Image)
{
  image = Image;

  if (image)
    UpdateKeyColorFromImage ();
}

void csTextureWrapper::SetTextureHandle (iTextureHandle *tex)
{
  image = 0;

  handle = tex;

  flags = handle->GetFlags ();
  UpdateKeyColorFromHandle ();
}

void csTextureWrapper::SetKeyColor (int red, int green, int blue)
{
  if (handle)
    if (red >= 0)
      handle->SetKeyColor (red, green, blue);
    else
      handle->SetKeyColor (false);
  key_col_r = red;
  key_col_g = green;
  key_col_b = blue;
}

void csTextureWrapper::Register (iTextureManager *txtmgr)
{
  if (handle) return;

  // if we have no image, we cannot register it.
  if (!image) return ;

  // Now we check the size of the loaded image. Having an image, that
  // is not a power of two will result in strange errors while
  // rendering. It is by far better to check the format of all textures
  // already while loading them.
  if (flags & CS_TEXTURE_3D)
  {
    int Width = image->GetWidth ();
    int Height = image->GetHeight ();

    if (!csIsPowerOf2 (Width) || !csIsPowerOf2 (Height))
    {
      csEngine::currentEngine->Warn (
          "Inefficient texture image '%s' dimensions!\nThe width (%d) and height (%d) should be a power of two.\n",
          GetName (),
          Width,
          Height);
    }
  }

  handle = txtmgr->RegisterTexture (image, flags);
  if (handle)
  {
    SetKeyColor (key_col_r, key_col_g, key_col_b);
    handle->SetTextureClass (texClass);
    delete[] texClass; texClass = 0; 
  }

  if (!keep_image)
    SetImageFile (0);
}

void csTextureWrapper::SetTextureClass (const char* className)
{
  if (handle) 
    handle->SetTextureClass (className);
  else
  {
    delete[] texClass;
    texClass = csStrNew (className);
  }
}

const char* csTextureWrapper::GetTextureClass ()
{
  if (handle) 
    return handle->GetTextureClass();
  else
    return texClass;
}

//------------------------------------------------------- csTextureList -----//
SCF_IMPLEMENT_IBASE(csTextureList)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE(iTextureList)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csTextureList::TextureList)
  SCF_IMPLEMENTS_INTERFACE(iTextureList)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

csTextureList::csTextureList () :
  csRefArrayObject<iTextureWrapper> (16, 16)
{
  SCF_CONSTRUCT_IBASE (0);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiTextureList);
}

csTextureList::~csTextureList()
{
  SCF_DESTRUCT_EMBEDDED_IBASE (scfiTextureList);
  SCF_DESTRUCT_IBASE ();
}

iTextureWrapper *csTextureList::NewTexture (iImage *image)
{
  iTextureWrapper *tm = new csTextureWrapper (image);
  Push (tm);
  tm->DecRef ();
  return tm;
}

iTextureWrapper *csTextureList::NewTexture (iTextureHandle *ith)
{
  iTextureWrapper *tm = new csTextureWrapper (ith);
  Push (tm);
  tm->DecRef ();
  return tm;
}

iTextureWrapper *csTextureList::TextureList::NewTexture (iImage *image)
{
  return scfParent->NewTexture (image);
}

iTextureWrapper *csTextureList::TextureList::NewTexture (iTextureHandle *ith)
{
  return scfParent->NewTexture (ith);
}

int csTextureList::TextureList::GetCount () const
{
  return (int)scfParent->Length ();
}

iTextureWrapper *csTextureList::TextureList::Get (int n) const
{
  return scfParent->Get (n);
}

int csTextureList::TextureList::Add (iTextureWrapper *obj)
{
  return (int)scfParent->Push (obj);
}

bool csTextureList::TextureList::Remove (iTextureWrapper *obj)
{
  return scfParent->Delete (obj);
}

bool csTextureList::TextureList::Remove (int n)
{
  return scfParent->DeleteIndex (n);
}

void csTextureList::TextureList::RemoveAll ()
{
  scfParent->DeleteAll ();
}

int csTextureList::TextureList::Find (iTextureWrapper *obj) const
{
  return (int)scfParent->Find (obj);
}

iTextureWrapper *csTextureList::TextureList::FindByName (
  const char *Name) const
{
  return scfParent->FindByName (Name);
}
