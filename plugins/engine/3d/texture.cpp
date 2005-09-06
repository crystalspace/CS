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
#include "csutil/debug.h"
#include "igraphic/image.h"
#include "ivideo/txtmgr.h"

CS_LEAKGUARD_IMPLEMENT (csTextureWrapper);

//---------------------------------------------------------------------------
SCF_IMPLEMENT_IBASE_EXT(csTextureWrapper)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE(iTextureWrapper)
  SCF_IMPLEMENTS_INTERFACE(csTextureWrapper)
SCF_IMPLEMENT_IBASE_EXT_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csTextureWrapper::TextureWrapper)
  SCF_IMPLEMENTS_INTERFACE(iTextureWrapper)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

csTextureWrapper::csTextureWrapper (iImage *Image) :
    csObject(),
    flags(CS_TEXTURE_3D)
{
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiTextureWrapper);
  DG_TYPE (this, "csTextureWrapper");
  image = Image;
  keep_image = false;
  texClass = 0;
  DG_LINK (this, image);
  UpdateKeyColorFromImage ();
}

csTextureWrapper::csTextureWrapper (iTextureHandle *ith) : csObject()
{
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiTextureWrapper);
  DG_TYPE (this, "csTextureWrapper");

  keep_image = false;
  texClass = 0;

  handle = ith;
  if (handle)
  {
    flags = ith->GetFlags ();
    DG_LINK (this, handle);
  }
  else
  {
    flags = 0;
  }

  UpdateKeyColorFromHandle ();
}

csTextureWrapper::csTextureWrapper (csTextureWrapper &t) :
  iBase(),
  csObject(t),
  flags(CS_TEXTURE_3D)
{
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiTextureWrapper);
  DG_TYPE (this, "csTextureWrapper");
  handle = t.handle;
  DG_LINK (this, handle);
  image = t.image;
  DG_LINK (this, image);
  keep_image = t.keep_image;
  if (!handle)
    texClass = csStrNew (t.texClass);
  else
    texClass = 0;

  UpdateKeyColorFromImage ();
}

csTextureWrapper::~csTextureWrapper ()
{
  if (handle)
    DG_UNLINK (this, handle);
  if (image)
    DG_UNLINK (this, image);
  delete[] texClass;
  SCF_DESTRUCT_EMBEDDED_IBASE (scfiTextureWrapper);
}

void csTextureWrapper::SetImageFile (iImage *Image)
{
  if (Image)
  {
    DG_LINK (this, Image);
  }

  if (image)
  {
    DG_UNLINK (this, image);
  }

  image = Image;

  if (image)
    UpdateKeyColorFromImage ();
}

void csTextureWrapper::SetTextureHandle (iTextureHandle *tex)
{
  if (image)
  {
    DG_UNLINK (this, image);
    image = 0;
  }

  if (handle)
  {
    DG_UNLINK (this, handle);
  }

  handle = tex;
  DG_LINK (this, handle);

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
    DG_LINK (this, handle);
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

iObject *csTextureWrapper::TextureWrapper::QueryObject ()
{
  return scfParent;
}

void csTextureWrapper::TextureWrapper::SetImageFile (iImage *Image)
{
  scfParent->SetImageFile (Image);
}

iImage *csTextureWrapper::TextureWrapper::GetImageFile ()
{
  return scfParent->GetImageFile ();
}

void csTextureWrapper::TextureWrapper::SetTextureHandle (iTextureHandle *tex)
{
  scfParent->SetTextureHandle (tex);
}

void csTextureWrapper::TextureWrapper::SetKeyColor (
  int red,
  int green,
  int blue)
{
  scfParent->SetKeyColor (red, green, blue);
}

void csTextureWrapper::TextureWrapper::GetKeyColor (
  int &red,
  int &green,
  int &blue) const
{
  scfParent->GetKeyColor (red, green, blue);
}

void csTextureWrapper::TextureWrapper::SetFlags (int flags)
{
  scfParent->SetFlags (flags);
}

int csTextureWrapper::TextureWrapper::GetFlags () const
{
  return scfParent->GetFlags ();
}

void csTextureWrapper::TextureWrapper::Register (iTextureManager *txtmng)
{
  scfParent->Register (txtmng);
}

void csTextureWrapper::TextureWrapper::SetUseCallback (
  iTextureCallback *callback)
{
  scfParent->SetUseCallback (callback);
}

iTextureCallback *csTextureWrapper::TextureWrapper::GetUseCallback () const
{
  return scfParent->GetUseCallback ();
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
  iTextureWrapper *tm = &(new csTextureWrapper (image))->scfiTextureWrapper;
  Push (tm);
  tm->DecRef ();
  return tm;
}

iTextureWrapper *csTextureList::NewTexture (iTextureHandle *ith)
{
  iTextureWrapper *tm = &(new csTextureWrapper (ith))->scfiTextureWrapper;
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
