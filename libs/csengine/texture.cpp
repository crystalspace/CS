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
#include "csengine/world.h"
#include "iimage.h"
#include "itxtmgr.h"

//---------------------------------------------------------------------------

IMPLEMENT_CSOBJTYPE (csTextureHandle,csObject);

csTextureHandle::csTextureHandle (iImage* Image) :
  csObject (), handle (NULL), flags (CS_TEXTURE_3D)
{
  (image = Image)->IncRef ();
  transp_r = -1;
}

csTextureHandle::csTextureHandle (csTextureHandle &th) :
  csObject (), handle (NULL)
{
  flags = th.flags;
  (image = th.image)->IncRef ();
  transp_r = th.transp_r;
  transp_g = th.transp_g;
  transp_b = th.transp_b;
  handle = th.GetTextureHandle ();
  SetName (th.GetName ());
  if (handle)
    SetTransparent (transp_r, transp_g, transp_b);
}

csTextureHandle::~csTextureHandle ()
{
  if (handle)
    handle->DecRef ();
  if (image)
    image->DecRef ();
}

void csTextureHandle::SetImageFile (iImage *Image)
{
  if (image)
    image->DecRef ();
  (image = Image)->IncRef ();
}

void csTextureHandle::SetTransparent (int red, int green, int blue)
{
  if (handle)
    if (red >= 0)
      handle->SetTransparent (red, green, blue);
    else
      handle->SetTransparent (false);
  transp_r = red;
  transp_g = green;
  transp_b = blue;
}

void csTextureHandle::Register (iTextureManager *txtmgr)
{
  if (handle) handle->DecRef ();

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
        "Inefficient texture image '%s' dimenstions!\n"
        "The width (%d) and height (%d) should be a power of two.\n",
        GetName (), Width, Height);
  }

  handle = txtmgr->RegisterTexture (image, flags);
  if (handle)
    SetTransparent (transp_r, transp_g, transp_b);
}

//-------------------------------------------------------- csTextureList -----//

csTextureList::~csTextureList ()
{
  DeleteAll ();
}

csTextureHandle *csTextureList::NewTexture (iImage *image)
{
  CHK (csTextureHandle *tm = new csTextureHandle (image));
  Push (tm);
  return tm;
}
