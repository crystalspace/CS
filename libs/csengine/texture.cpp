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

#include <math.h>

#include "cssysdef.h"
#include "csengine/texture.h"
#include "csengine/engine.h"
#include "igraphic/image.h"
#include "ivideo/txtmgr.h"

//---------------------------------------------------------------------------

IMPLEMENT_IBASE (csTextureWrapper)
  IMPLEMENTS_EMBEDDED_INTERFACE (iTextureWrapper)
  IMPLEMENTS_INTERFACE (csTextureWrapper)
IMPLEMENT_IBASE_END

IMPLEMENT_EMBEDDED_IBASE (csTextureWrapper::TextureWrapper)
  IMPLEMENTS_INTERFACE (iTextureWrapper)
IMPLEMENT_EMBEDDED_IBASE_END

IMPLEMENT_CSOBJTYPE (csTextureWrapper,csPObject);

csTextureWrapper::csTextureWrapper (iImage* Image) :
  csPObject (), handle (NULL), flags (CS_TEXTURE_3D), use_callback (NULL)
{
  CONSTRUCT_IBASE (NULL);
  CONSTRUCT_EMBEDDED_IBASE (scfiTextureWrapper);

  (image = Image)->IncRef ();
  UpdateKeyColorFromImage ();

  // @@@ ?????
  csEngine::current_engine->AddToCurrentRegion (this);
}

csTextureWrapper::csTextureWrapper (iTextureHandle *ith) :
  csPObject (), image (NULL), use_callback (NULL)
{
  CONSTRUCT_IBASE (NULL);
  CONSTRUCT_EMBEDDED_IBASE (scfiTextureWrapper);

  (handle = ith)->IncRef ();
  flags = ith->GetFlags ();
  UpdateKeyColorFromHandle ();

  // @@@ ?????
  csEngine::current_engine->AddToCurrentRegion (this);
}

csTextureWrapper::~csTextureWrapper ()
{
  if (handle)
    handle->DecRef ();
  if (image)
    image->DecRef ();
}

void csTextureWrapper::SetImageFile (iImage *Image)
{
  if (image)
    image->DecRef ();
  (image = Image)->IncRef ();

  UpdateKeyColorFromImage ();
}

void csTextureWrapper::SetTextureHandle (iTextureHandle *tex)
{
  if (image)
    image->DecRef ();
  if (handle)
    handle->DecRef ();

  image = NULL;
  (handle = tex)->IncRef ();
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
  // smgh 22/07/00
  //if (handle) handle->DecRef ();

  // if we have no image, we cannot register it.
  if (!image)
    return;

  // release old handle
  if (handle)
    handle->DecRef ();

  // Now we check the size of the loaded image. Having an image, that
  // is not a power of two will result in strange errors while
  // rendering. It is by far better to check the format of all textures
  // already while loading them.
  if (flags & CS_TEXTURE_3D)
  {
    int Width  = image->GetWidth ();
    int Height = image->GetHeight ();

    if (!IsPowerOf2 (Width) || !IsPowerOf2 (Height))
      CsPrintf (MSG_WARNING,
        "Inefficient texture image '%s' dimensions!\n"
        "The width (%d) and height (%d) should be a power of two.\n",
        GetName (), Width, Height);
  }

  handle = txtmgr->RegisterTexture (image, flags);
  if (handle)
    SetKeyColor (key_col_r, key_col_g, key_col_b);
}

iObject *csTextureWrapper::TextureWrapper::QueryObject()
{ return scfParent; }
void csTextureWrapper::TextureWrapper::SetImageFile (iImage *Image)
{ scfParent->SetImageFile (Image); }
iImage* csTextureWrapper::TextureWrapper::GetImageFile ()
{ return scfParent->GetImageFile (); }
void csTextureWrapper::TextureWrapper::SetTextureHandle (iTextureHandle *tex)
{ scfParent->SetTextureHandle (tex); }
iTextureHandle* csTextureWrapper::TextureWrapper::GetTextureHandle ()
{ return scfParent->GetTextureHandle (); }
void csTextureWrapper::TextureWrapper::SetKeyColor (int red, int green, int blue)
{ scfParent->SetKeyColor (red, green, blue); }
void csTextureWrapper::TextureWrapper::GetKeyColor (int &red, int &green, int &blue)
{ scfParent->GetKeyColor (red, green, blue); }
void csTextureWrapper::TextureWrapper::SetFlags (int flags)
{ scfParent->SetFlags (flags); }
int csTextureWrapper::TextureWrapper::GetFlags ()
{ return scfParent->GetFlags (); }
void csTextureWrapper::TextureWrapper::Register (iTextureManager *txtmng)
{ scfParent->Register (txtmng); }
void csTextureWrapper::TextureWrapper::SetUseCallback (csTextureCallback* callback, void* data)
{ scfParent->SetUseCallback (callback, data); }
csTextureCallback* csTextureWrapper::TextureWrapper::GetUseCallback ()
{ return scfParent->GetUseCallback (); }
void* csTextureWrapper::TextureWrapper::GetUseData ()
{ return scfParent->GetUseData (); }
void csTextureWrapper::TextureWrapper::Visit ()
{ scfParent->Visit (); }

//------------------------------------------------------- csTextureList -----//

IMPLEMENT_IBASE (csTextureList)
  IMPLEMENTS_EMBEDDED_INTERFACE (iTextureList)
IMPLEMENT_IBASE_END

IMPLEMENT_EMBEDDED_IBASE (csTextureList::TextureList)
  IMPLEMENTS_INTERFACE (iTextureList)
IMPLEMENT_EMBEDDED_IBASE_END

csTextureList::csTextureList () : csNamedObjVector (16, 16)
{
  CONSTRUCT_IBASE (NULL);
  CONSTRUCT_EMBEDDED_IBASE (scfiTextureList);
}

csTextureList::~csTextureList ()
{
  DeleteAll ();
}

csTextureWrapper *csTextureList::NewTexture (iImage *image)
{
  csTextureWrapper *tm = new csTextureWrapper (image);
  Push (tm);
  return tm;
}

csTextureWrapper *csTextureList::NewTexture (iTextureHandle *ith)
{
  csTextureWrapper *tm = new csTextureWrapper (ith);
  Push (tm);
  return tm;
}

iTextureWrapper *csTextureList::TextureList::NewTexture (iImage *image)
{
  return &scfParent->NewTexture(image)->scfiTextureWrapper;
}

iTextureWrapper *csTextureList::TextureList::NewTexture (iTextureHandle *ith)
{
  return &scfParent->NewTexture(ith)->scfiTextureWrapper;
}

long csTextureList::TextureList::GetNumTextures () const
{
  return scfParent->Length();
}

iTextureWrapper *csTextureList::TextureList::Get (int idx) const
{
  return &scfParent->Get(idx)->scfiTextureWrapper;
}

iTextureWrapper *csTextureList::TextureList::FindByName (const char* iName) const
{
  return &scfParent->FindByName(iName)->scfiTextureWrapper;
}
