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
  TranspChanged = false;
}

csWSTexture::~csWSTexture ()
{
  Unregister ();
  CHK (delete [] Name);
  CHK (delete [] FileName);
  if (Image)
    Image->DecRef ();
  if (Handle)
    Handle->DecRef ();
}

void csWSTexture::SetTransparent (int iR, int iG, int iB)
{
  IsTransp = (iR >= 0) && (iG >= 0) && (iB >= 0);
  if (IsTransp)
  {
    tr = iR; tg = iG; tb = iB;
    TranspChanged = true;
  }
  if (Handle)
    Handle->SetTransparent (iR, iG, iB);
}

void csWSTexture::SetTransparent (bool iTransparent)
{
  IsTransp = iTransparent;
  if (Handle)
    if (IsTransp)
      Handle->SetTransparent (tr, tg, tb);
    else
      Handle->SetTransparent (-1, -1, -1);
}

void csWSTexture::FixTransparency ()
{
  if (!IsTransp || !TranspChanged || !Image)
    return;

  TranspChanged = false;

  RGBPixel *src = Image->GetImageData ();
  int size = Image->GetSize ();
  RGBPixel color;
  int colordist = 0x7fffffff;
  while (size--)
  {
    int dR = src->red - tr;
    int dG = src->green - tg;
    int dB = src->blue - tb;
    int dist = 299 * dR * dR + 587 * dG * dG + 114 * dB * dB;
    if (colordist > dist)
    {
      colordist = dist;
      color = *src;
    }
    src++;
  }
  tr = color.red;
  tg = color.green;
  tb = color.blue;
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

void csWSTexture::Refresh ()
{
  if (!TexMan || !Handle)
    return;
  FixTransparency ();
  if (IsTransp)
    Handle->SetTransparent (tr, tg, tb);
  else
    Handle->SetTransparent (-1, -1, -1);
  TexMan->MergeTexture (Handle);
}

void csWSTexture::SetName (const char *iName)
{
  CHK (delete [] Name);
  Name = strnew (iName);
}

void csWSTexture::SetFileName (const char *iFileName)
{
  CHK (delete [] FileName);
  FileName = strnew (iFileName);
}

int csWSTexture::GetWidth ()
{
  if (Image)
    return Image->GetWidth ();
  else if (Handle)
  {
    int bw, bh;
    Handle->GetBitmapDimensions (bw, bh);
    return bw;
  }
  return 0;
}

int csWSTexture::GetHeight ()
{
  if (Image)
    return Image->GetHeight ();
  else if (Handle)
  {
    int bw, bh;
    Handle->GetBitmapDimensions (bw, bh);
    return bh;
  }
  return 0;
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
  CHK (delete (csWSTexture *)Item);
  return true;
}

int csWSTexVector::CompareKey (csSome Item, csConstSome Key, int Mode) const
{
  (void) Mode;
  return strcmp (((csWSTexture *)Item)->GetName (), (char *)Key);
}
