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

csWSTexture::csWSTexture (const char *iName, ImageFile *iImage,
  bool i2D, bool i3D)
{
  image = iImage;
  for2D = i2D;
  for3D = i3D;
  istransp = false;
  Name = strnew (iName);
  Handle = NULL;
}

csWSTexture::~csWSTexture ()
{
  if (Name)
    delete [] Name;
//@@todo: sigsegv here? who freed the texture handle
//  if (Handle)
//    Handle->Release ();
}

void csWSTexture::SetTransparent (int iR, int iG, int iB)
{
  istransp = true;
  tr = iR; tg = iG; tb = iB;
}

void csWSTexture::Register (ITextureManager *iTextureManager)
{
  iTextureManager->RegisterTexture (GetIImageFileFromImageFile (image),
    &Handle, for3D, for2D);
  if (istransp)
    Handle->SetTransparent (tr, tg, tb);
}

void csWSTexture::SetName (const char *iName)
{
  if (Name)
    delete [] Name;
  Name = strnew (iName);
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
  return strcmp (((csWSTexture *)Item)->GetName (), (char *)Key);
}
