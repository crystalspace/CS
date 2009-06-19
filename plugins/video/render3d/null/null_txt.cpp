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
#include "null_txt.h"
#include "csgfx/inv_cmap.h"
#include "csgfx/quantize.h"
#include "csutil/scanstr.h"
#include "iutil/cfgfile.h"
#include "iutil/event.h"
#include "iutil/eventq.h"
#include "iutil/objreg.h"
#include "iutil/string.h"
#include "igraphic/image.h"

using namespace CS::Threading;

//--------------------------------------------------- csTextureHandleNull ---//

csTextureHandleNull::csTextureHandleNull (csTextureManagerNull *txtmgr,
  iImage *image, int flags) : csTextureHandle (txtmgr, flags)
{
  texman = txtmgr;

  prepared = true;
  orig_w = image->GetWidth();
  orig_h = image->GetHeight();
  orig_d = image->GetDepth();
  if (flags & CS_TEXTURE_3D)
  {
    AdjustSizePo2 (orig_w, orig_h, orig_d, w, h, d);
  }
  else
  {
    w = orig_w;
    h = orig_h;
    d = orig_d;
  }
}

csTextureHandleNull::csTextureHandleNull (csTextureManagerNull *txtmgr,
  int w, int h, int d, int flags) : csTextureHandle (txtmgr, flags),
  orig_w (w), orig_h (h), orig_d (d)
{
  texman = txtmgr;

  prepared = true;
  if (flags & CS_TEXTURE_3D)
  {
    AdjustSizePo2 (orig_w, orig_h, orig_d, this->w, this->h, this->d);
  }
  else
  {
    this->w = orig_w;
    this->h = orig_h;
    this->d = orig_d;
  }
}

csTextureHandleNull::~csTextureHandleNull ()
{
  texman->UnregisterTexture (this);
}

//----------------------------------------------- csTextureManagerNull ---//

csTextureManagerNull::csTextureManagerNull (iObjectRegistry *object_reg,
  iGraphics2D *iG2D, iConfigFile *config) : csTextureManager (object_reg, iG2D)
{
  read_config (config);
  G2D = iG2D;
}

csTextureManagerNull::~csTextureManagerNull ()
{
  Clear ();
}

void csTextureManagerNull::SetPixelFormat (csPixelFormat &PixelFormat)
{
  pfmt = PixelFormat;
}

csPtr<iTextureHandle> csTextureManagerNull::RegisterTexture (iImage* image,
  int flags, iString* fail_reason)
{
  if (!image)
  {
    if (fail_reason) fail_reason->Replace (
      "No image given to RegisterTexture!");
    return 0;
  }

  csTextureHandleNull *txt = new csTextureHandleNull (this, image, flags);

  MutexScopedLock lock(texturesLock);
  textures.Push (txt);

  return csPtr<iTextureHandle> (txt);
}

csPtr<iTextureHandle> csTextureManagerNull::CreateTexture (int w, int h, int d,
      csImageType imagetype, const char* format, int flags,
      iString* fail_reason)
{
  (void)imagetype;
  (void)format;
  csTextureHandleNull *txt = new csTextureHandleNull (this, w, h, d, flags);

  MutexScopedLock lock(texturesLock);
  textures.Push (txt);

  return csPtr<iTextureHandle> (txt);
}

void csTextureManagerNull::UnregisterTexture (csTextureHandleNull* handle)
{
  MutexScopedLock lock(texturesLock);
  size_t idx = textures.Find (handle);
  if (idx != csArrayItemNotFound) textures.DeleteIndexFast (idx);
}

csPtr<iSuperLightmap> csTextureManagerNull::CreateSuperLightmap (int /*w*/,
  int /*h*/)
{
  // @@@ implement a "NullRendererLightmap"
  return 0;
}
  
void csTextureManagerNull::GetMaxTextureSize (int& w, int& h, int& aspect)
{
  w = h = 2048;
  aspect = 32768;
}
