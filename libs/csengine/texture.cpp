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
#include "iimage.h"
#include "itxtmgr.h"

//---------------------------------------------------------------------------

IMPLEMENT_IBASE (csTextureWrapper)
  IMPLEMENTS_EMBEDDED_INTERFACE (iTextureWrapper)
IMPLEMENT_IBASE_END

IMPLEMENT_EMBEDDED_IBASE (csTextureWrapper::TextureWrapper)
  IMPLEMENTS_INTERFACE (iTextureWrapper)
IMPLEMENT_EMBEDDED_IBASE_END

IMPLEMENT_CSOBJTYPE (csTextureWrapper,csPObject);

csTextureWrapper::csTextureWrapper (iImage* Image) :
  csPObject (), handle (NULL), flags (CS_TEXTURE_3D)
{
  CONSTRUCT_IBASE (NULL);
  CONSTRUCT_EMBEDDED_IBASE (scfiTextureWrapper);
  use_callback = NULL;
  (image = Image)->IncRef ();
  key_col_r = -1;
  if(image->HasKeycolor ())
    image->GetKeycolor( key_col_r, key_col_g, key_col_b );
  csEngine::current_engine->AddToCurrentRegion (this);
}

csTextureWrapper::csTextureWrapper (csTextureWrapper &th) :
  csPObject (), handle (NULL)
{
  CONSTRUCT_IBASE (NULL);
  CONSTRUCT_EMBEDDED_IBASE (scfiTextureWrapper);
  use_callback = NULL;
  flags = th.flags;
  (image = th.image)->IncRef ();
  key_col_r = th.key_col_r;
  key_col_g = th.key_col_g;
  key_col_b = th.key_col_b;
  handle = th.GetTextureHandle ();
  SetName (th.GetName ());
  if (handle)
    SetKeyColor (key_col_r, key_col_g, key_col_b);
  csEngine::current_engine->AddToCurrentRegion (this);
}

csTextureWrapper::csTextureWrapper (iTextureHandle *ith) :
  csPObject (), image (NULL)
{
  CONSTRUCT_IBASE (NULL);
  CONSTRUCT_EMBEDDED_IBASE (scfiTextureWrapper);
  use_callback = NULL;
  ith->IncRef ();
  flags = ith->GetFlags ();
  if (ith->GetKeyColor ())
  {
    UByte r, g, b;
    ith->GetKeyColor (r, g, b);
    SetKeyColor ((int)r, (int)g, (int)b);
  }
  handle = ith;
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

//------------------------------------------------------- csTextureList -----//

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
