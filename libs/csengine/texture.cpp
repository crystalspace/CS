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

#include "sysdef.h"
#include "csengine/texture.h"
#include "csgfxldr/csimage.h"
#include "csobject/nameobj.h"
#include "itxtmgr.h"

//---------------------------------------------------------------------------

CSOBJTYPE_IMPL(csTextureHandle,csObject);

csTextureHandle::csTextureHandle (ImageFile* image) : 
  csObject (), for_2d (false), for_3d (true)
{
  ifile = image;
  txt_handle = NULL;
  transp_r = -1;
}

csTextureHandle::~csTextureHandle ()
{
}

void csTextureHandle::SetTextureHandle (ITextureHandle* h)
{
  txt_handle = h;
  if (transp_r != -1) txt_handle->SetTransparent (transp_r, transp_g, transp_b);
}

void csTextureHandle::SetTransparent (int red, int green, int blue)
{
  if (txt_handle)
    txt_handle->SetTransparent (red, green, blue);
  transp_r = red;
  transp_g = green;
  transp_b = blue;
}

//---------------------------------------------------------------------------

csTextureList::csTextureList ()
{
  max_textures = 10;
  num_textures = 0;
  CHK (textures = new csTextureHandle* [10]);
}

csTextureList::~csTextureList ()
{
  Clear ();
  CHK (delete [] textures);
}

void csTextureList::Clear ()
{
  if (textures)
    for (int i = 0; i < num_textures; i++)
      CHKB (delete textures[i]);
  CHK (delete [] textures);
  max_textures = 10;
  num_textures = 0;
  CHK (textures = new csTextureHandle* [10]);
}

csTextureHandle* csTextureList::NewTexture (ImageFile* image)
{
  CHK (csTextureHandle* tm = new csTextureHandle (image));
  //if (tm->loaded_correctly ())
    AddTexture (tm);
  return tm;
}

void csTextureList::AddTexture (csTextureHandle* tm)
{
  if (num_textures >= max_textures)
  {
    int max_textures = num_textures + 8;
    CHK (csTextureHandle** newtextures = new csTextureHandle* [max_textures]);
    if (textures)
    {
      memcpy (newtextures, textures, num_textures * sizeof (csTextureHandle*));
      CHK (delete [] textures);
    }
    textures = newtextures;
  }
  textures [num_textures++] = tm;
}

int csTextureList::GetTextureIdx (const char* name)
{
  int i;
  for (i = 0 ; i < num_textures ; i++)
    if (!strcmp (csNameObject::GetName(*(textures[i])), name)) return i;
  return -1;
}

csTextureHandle* csTextureList::GetTextureMM (const char* name)
{
  int idx = GetTextureIdx (name);
  if (idx == -1) return NULL;
  else return textures[idx];
}

//---------------------------------------------------------------------------
