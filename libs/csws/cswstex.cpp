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

#include "cssysdef.h"
#include "csws/cswstex.h"
#include "csutil/util.h"
#include "ivideo/texture.h"
#include "iengine/texture.h"
#include "ivideo/txtmgr.h"

//--//--//--//--//--//--//--//--//--//--//--//-- Windowing system texture --//--

csWSTexture::csWSTexture (const char *iName, iImage *inImage, int iFlags)
{
  (Image = inImage)->IncRef ();
  Flags = iFlags;
  HasKey = false;
  Name = csStrNew (iName);
  FileName = csStrNew (Image->GetName ());
  Handle = NULL;
  KeyChanged = false;
  tr = tg = tb = 255;
}

csWSTexture::~csWSTexture ()
{
  Unregister ();
  delete [] Name;
  delete [] FileName;
  if (Image)
    Image->DecRef ();
  if (Handle)
    Handle->DecRef ();
}

void csWSTexture::SetKeyColor (int iR, int iG, int iB)
{
  HasKey = true;
  KeyChanged = true;
  tr = iR; tg = iG; tb = iB;
  if (Handle)
  {
    Handle->SetKeyColor (tr, tg, tb);
    Handle->SetKeyColor (HasKey);
  }
}

void csWSTexture::SetKeyColor (bool iEnable)
{
  HasKey = iEnable;
  if (Handle)
  {
    Handle->SetKeyColor (tr, tg, tb);
    Handle->SetKeyColor (HasKey);
  }
}

void csWSTexture::FixKeyColor ()
{
  if (!HasKey || !KeyChanged || !Image)
    return;

  KeyChanged = false;

  csRGBpixel color;
  csRGBpixel *src;
  int size;
  if ((Image->GetFormat () & CS_IMGFMT_MASK) == CS_IMGFMT_TRUECOLOR)
  {
    src = (csRGBpixel *)Image->GetImageData ();
    size = Image->GetSize ();
  }
  else
  {
    src = Image->GetPalette ();
    size = 256;
  }
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
  Handle = iTexMan->RegisterTexture (Image, Flags);
  SetKeyColor (HasKey);
}

void csWSTexture::Unregister ()
{
  if (Handle)
  {
    Handle->DecRef ();
    Handle = NULL;
  }
}

void csWSTexture::Refresh ()
{
  if (!Handle)
    return;
  FixKeyColor ();
  SetKeyColor (HasKey);
  Handle->Prepare ();
}

void csWSTexture::SetName (const char *iName)
{
  delete [] Name;
  Name = csStrNew (iName);
}

void csWSTexture::SetFileName (const char *iFileName)
{
  delete [] FileName;
  FileName = csStrNew (iFileName);
}

int csWSTexture::GetWidth ()
{
  if (Image)
    return Image->GetWidth ();
  else if (Handle)
  {
    int bw, bh;
    Handle->GetMipMapDimensions (0, bw, bh);
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
    Handle->GetMipMapDimensions (0, bw, bh);
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
  delete (csWSTexture *)Item;
  return true;
}

int csWSTexVector::CompareKey (csSome Item, csConstSome Key, int Mode) const
{
  (void) Mode;
  return strcmp (((csWSTexture *)Item)->GetName (), (char *)Key);
}
