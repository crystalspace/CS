/*
    Copyright (C) 2005 Dan Hardfeldt and Seth Yastrov

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

#include "csgfx/memimage.h"
#include "iengine/texture.h"
#include "imap/loader.h"
#include "iutil/objreg.h"

#include "ceguitexture.h"

csCEGUITexture::csCEGUITexture (CEGUI::Renderer* owner, iObjectRegistry *reg) 
  : CEGUI::Texture (owner)
{
  renderer = owner;
  obj_reg = reg;
  hTxt = 0;
}
csCEGUITexture::~csCEGUITexture ()
{
}

CEGUI::ushort csCEGUITexture::getWidth () const
{
  int w = 0, h = 0;
  if (!hTxt) 
  {
    return 0;
  }

  hTxt->GetRendererDimensions(w,h);
  return w;
}

CEGUI::ushort csCEGUITexture::getHeight () const
{
  int w = 0, h = 0;
  if (!hTxt) 
  {
    return 0;
  }

  hTxt->GetRendererDimensions(w,h);
  return h;
}

void csCEGUITexture::loadFromFile (const CEGUI::String &filename, 
                                   const CEGUI::String &resourceGroup)
{
  csRef<iLoader> loader = CS_QUERY_REGISTRY(obj_reg, iLoader);
  if (!loader)
    return;

  iTextureWrapper* txt = loader->LoadTexture(filename.c_str(), filename.c_str());
  if(!txt)
    return;

  hTxt = txt->GetTextureHandle();
}

void csCEGUITexture::loadFromMemory (const void *buffPtr, 
  CEGUI::uint buffWidth, CEGUI::uint buffHeight)
{
  csRef<iGraphics3D> g3d = CS_QUERY_REGISTRY(obj_reg, iGraphics3D);
  if (!g3d)
    return;

  csRef<csImageMemory> image;
  image.AttachNew(new csImageMemory(buffWidth, buffHeight, buffPtr, 
    CS_IMGFMT_TRUECOLOR | CS_IMGFMT_ALPHA, new csRGBpixel(255,255,255)));
  iTextureManager* txtmgr = g3d->GetTextureManager();

  if (txtmgr)
  {
    hTxt = txtmgr->RegisterTexture (image, CS_TEXTURE_2D);
  }
}

CEGUI::Renderer* csCEGUITexture::getRenderer () const
{
  return renderer;
}

iTextureHandle* csCEGUITexture::GetTexHandle () 
{
  return hTxt;
}
