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
#include "csutil/debug.h"
#include "iutil/cfgfile.h"
#include "iutil/event.h"
#include "iutil/eventq.h"
#include "iutil/objreg.h"
#include "igraphic/image.h"
#include "igraphic/imgvec.h"

//--------------------------------------------------- csTextureHandleNull ---//

csTextureHandleNull::csTextureHandleNull (csTextureManagerNull *txtmgr,
  iImage *image, int flags) : csTextureHandle (txtmgr, image, flags)
{
  (texman = txtmgr)->IncRef ();

  prepared = true;
  if (flags & CS_TEXTURE_3D)
    AdjustSizePo2 ();
  FreeImage(); // Bye bye. Don't need you any more
}

csTextureHandleNull::~csTextureHandleNull ()
{
  texman->UnregisterTexture (this);
  texman->DecRef ();
}

csTexture *csTextureHandleNull::NewTexture (iImage *Image, bool /*ismipmap*/)
{
  return new csTextureNull (this, Image);
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

void csTextureManagerNull::read_config (iConfigFile *config)
{
  csTextureManager::read_config (config);
}

void csTextureManagerNull::Clear ()
{
  csTextureManager::Clear ();
}

csPtr<iTextureHandle> csTextureManagerNull::RegisterTexture (iImage* image,
  int flags)
{
  if (!image) return 0;

  csTextureHandleNull *txt = new csTextureHandleNull (this, image, flags);
  textures.Push (txt);
  return csPtr<iTextureHandle> (txt);
}

void csTextureManagerNull::UnregisterTexture (csTextureHandleNull* handle)
{
  size_t idx = textures.Find (handle);
  if (idx != csArrayItemNotFound) textures.DeleteIndexFast (idx);
}

csPtr<iSuperLightmap> csTextureManagerNull::CreateSuperLightmap (int w, int h)
{
  // @@@ implement a "NullRendererLightmap"
  return 0;
}
  
void csTextureManagerNull::GetMaxTextureSize (int& w, int& h, int& aspect)
{
  w = h = 2048;
  aspect = 32768;
}

void csTextureManagerNull::GetLightmapRendererCoords (
  int slmWidth, int slmHeight, int lm_x1, int lm_y1, int lm_x2, int lm_y2,
  float& lm_u1, float& lm_v1, float &lm_u2, float& lm_v2)
{
  lm_u1 = lm_x1;
  lm_v1 = lm_y1;
  lm_u2 = lm_x2;
  lm_v2 = lm_y2;
}
