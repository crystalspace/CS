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

#include "csutil/stringquote.h"

#include "igraphic/image.h"
#include "ivideo/txtmgr.h"

#include "texture.h"
#include "engine.h"

CS_LEAKGUARD_IMPLEMENT (csTextureWrapper);

//---------------------------------------------------------------------------

csTextureWrapper::csTextureWrapper (csEngine* engine, iImage *Image)
  : scfImplementationType (this), engine (engine),
  flags(CS_TEXTURE_3D)
{
  image = Image;
  keep_image = engine->csEngine::GetDefaultKeepImage();
  texClass = 0;
  keyColorDirty = true;
}

csTextureWrapper::csTextureWrapper (csEngine* engine, iTextureHandle *ith) 
  : scfImplementationType (this), engine (engine)
{
  keep_image = engine->csEngine::GetDefaultKeepImage();
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

  keyColorDirty = true;
}

csTextureWrapper::csTextureWrapper (csEngine* engine)
  : scfImplementationType (this), engine (engine),
  flags(CS_TEXTURE_3D)
{
  keep_image = engine->csEngine::GetDefaultKeepImage();
  texClass = 0;
}

void csTextureWrapper::SelfDestruct ()
{
  engine->GetTextureList ()->Remove (static_cast<iTextureWrapper*>(this));
}

csTextureWrapper::csTextureWrapper (const csTextureWrapper &t) :
  iBase(), scfImplementationType (this), engine (t.engine),
  flags(CS_TEXTURE_3D)
{
  handle = t.handle;
  image = t.image;
  keep_image = t.keep_image;
  if (!handle)
    texClass = csStrNew (t.texClass);
  else
    texClass = 0;

  keyColorDirty = true;
}

csTextureWrapper::~csTextureWrapper ()
{

  delete[] texClass;
}

void csTextureWrapper::SetImageFile (iImage *Image)
{
  image = Image;

  if (image)
    keyColorDirty = true;
}

void csTextureWrapper::SetTextureHandle (iTextureHandle *tex)
{
  image = 0;

  handle = tex;

  flags = handle->GetFlags ();
  keyColorDirty = true;
}

void csTextureWrapper::SetKeyColor (int red, int green, int blue)
{
  if (handle)
  {
    if (red >= 0)
      handle->SetKeyColor (red, green, blue);
    else
      handle->SetKeyColor (false);
  }
  key_col_r = red;
  key_col_g = green;
  key_col_b = blue;
  keyColorDirty = false;
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
      engine->Warn ("Inefficient texture image %s dimensions!\n"
        "The width (%d) and height (%d) should be a power of two.",
        CS::Quote::Single (GetName ()), Width, Height);
    }
  }

  csRef<scfString> fail_reason;
  fail_reason.AttachNew (new scfString ());
  handle = txtmgr->RegisterTexture (image, flags, fail_reason);
  if (handle)
  {
    if (keyColorDirty)
      UpdateKeyColorFromHandle ();
    SetKeyColor (key_col_r, key_col_g, key_col_b);
    handle->SetTextureClass (texClass);
    delete[] texClass; texClass = 0; 
  }
  else
  {
    engine->Error ("Error creating texture: %s", fail_reason->GetData ());
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

csTextureList::csTextureList (csEngine* engine) : scfImplementationType (this),
  csRefArrayObject<iTextureWrapper> (16), engine (engine)
{
}

csTextureList::~csTextureList()
{
}

iTextureWrapper *csTextureList::NewTexture (iImage *image)
{
  csRef<iTextureWrapper> tm;
  tm.AttachNew (new csTextureWrapper (engine, image));
  CS::Threading::ScopedWriteLock lock(texLock);
  Push (tm);
  return tm;
}

csPtr<iTextureWrapper> csTextureList::CreateTexture (iImage *image)
{
  csRef<iTextureWrapper> tm;
  tm.AttachNew (new csTextureWrapper (engine, image));
  return csPtr<iTextureWrapper>(tm);
}


iTextureWrapper *csTextureList::NewTexture (iTextureHandle *ith)
{
  csRef<iTextureWrapper> tm;
  tm.AttachNew (new csTextureWrapper (engine, ith));
  CS::Threading::ScopedWriteLock lock(texLock);
  Push (tm);
  return tm;
}

csPtr<iTextureWrapper> csTextureList::CreateTexture (iTextureHandle *ith)
{
  csRef<iTextureWrapper> tm;
  tm.AttachNew (new csTextureWrapper (engine, ith));
  return csPtr<iTextureWrapper>(tm);
}

int csTextureList::GetCount () const
{
  CS::Threading::ScopedReadLock lock(texLock);
  return (int)this->GetSize ();
}

iTextureWrapper *csTextureList::Get (int n) const
{
  CS::Threading::ScopedReadLock lock(texLock);
  return csRefArrayObject<iTextureWrapper>::Get (n);
}

int csTextureList::Add (iTextureWrapper *obj)
{
  CS::Threading::ScopedWriteLock lock(texLock);
  return (int)this->Push (obj);
}

void csTextureList::AddBatch (csRef<iTextureLoaderIterator> itr, bool precache)
{
  CS::Threading::ScopedWriteLock lock(texLock);
  while(itr->HasNext())
  {
    iTextureWrapper* tex = itr->Next();
    Push(tex);
    if(precache && tex->GetTextureHandle())
    {
      tex->GetTextureHandle()->Precache();
    }
  }
}

bool csTextureList::Remove (iTextureWrapper *obj)
{
  CS::Threading::ScopedWriteLock lock(texLock);
  return this->Delete (obj);
}

bool csTextureList::Remove (int n)
{
  CS::Threading::ScopedWriteLock lock(texLock);
  return this->DeleteIndex (n);
}

void csTextureList::RemoveAll ()
{
  CS::Threading::ScopedWriteLock lock(texLock);
  this->DeleteAll ();
}

int csTextureList::Find (iTextureWrapper *obj) const
{
  CS::Threading::ScopedReadLock lock(texLock);
  return (int)csRefArrayObject<iTextureWrapper>::Find (obj);
}

iTextureWrapper *csTextureList::FindByName (const char *Name) const
{
  CS::Threading::ScopedReadLock lock(texLock);
  return csRefArrayObject<iTextureWrapper>::FindByName (Name);
}
