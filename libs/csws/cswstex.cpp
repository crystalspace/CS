/*
    Crystal Space Windowing System: Windowing System Texture class
    Copyright (C) 1998,1999 by Andrew Zabolotny <bit@eltech.ru>

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

#include "sysdef.h"
#include "csws/cswstex.h"
#include "csutil/util.h"
#include "itexture.h"
#include "itxtmgr.h"

//--//--//--//--//--//--//--//--//--//--//--//-- Windowing system texture --//--

csWSTexture::csWSTexture (const char *iName, iImageFile *iImage,
  bool i2D, bool i3D)
{
  (Image = iImage)->IncRef ();
  for2D = i2D;
  for3D = i3D;
  IsTransp = false;
  Name = strnew (iName);
  FileName = strnew (iImage->GetName ());
  Handle = NULL;
  TexMan = NULL;
}

csWSTexture::~csWSTexture ()
{
  Unregister ();
  if (Name)
    delete [] Name;
  if (FileName)
    delete [] FileName;
  if (Image)
    Image->DecRef ();
  if (Handle)
    Handle->DecRef ();
}

void csWSTexture::SetTransparent (int iR, int iG, int iB)
{
  IsTransp = (iR >= 0) && (iG >= 0) && (iB >= 0);
  tr = iR; tg = iG; tb = iB;
}

void csWSTexture::Register (iTextureManager *iTexMan)
{
  Unregister ();
  TexMan = iTexMan;
  Handle = iTexMan->RegisterTexture (Image, for3D, for2D);
  if (IsTransp)
    Handle->SetTransparent (tr, tg, tb);
  Handle->IncRef ();
}

void csWSTexture::Unregister ()
{
  if (Handle)
  {
    TexMan->UnregisterTexture (Handle);
    TexMan = NULL;
    Handle->DecRef ();
    Handle = NULL;
  }
}

void csWSTexture::SetName (const char *iName)
{
  if (Name)
    delete [] Name;
  Name = strnew (iName);
}

void csWSTexture::SetFileName (const char *iFileName)
{
  if (FileName)
    delete [] FileName;
  FileName = strnew (iFileName);
}

csWSTexVector::csWSTexVector () : csVector (16, 16)
{
}

csWSTexVector::~csWSTexVector ()
{
  DeleteAll ();
}

bool csWSTexVector::FreeItem (csSome Item)
{
  if (Item)
    delete (csWSTexture *)Item;
  return true;
}

int csWSTexVector::CompareKey (csSome Item, csConstSome Key, int Mode) const
{
  (void) Mode;
  return strcmp (((csWSTexture *)Item)->GetName (), (char *)Key);
}
