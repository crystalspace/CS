/*
    Copyright (C) 2005 Dan Hardfeldt, Seth Yastrov and Jelle Hellemans

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
#include "csgfx/imagememory.h"
#include "iengine/texture.h"
#include "imap/loader.h"
#include "iutil/objreg.h"
#include "ceguiimports.h"
#include "ceguitexture.h"

CS_PLUGIN_NAMESPACE_BEGIN(cegui)
{
  //----------------------------------------------------------------------------//
  Texture::Texture (CEGUI::Renderer* owner, iObjectRegistry *reg) 
  {
    renderer = owner;
    obj_reg = reg;
    hTxt = 0;
  }

  //----------------------------------------------------------------------------//
  Texture::~Texture ()
  {
  }

  //----------------------------------------------------------------------------//
  const CEGUI::Size& Texture::getSize() const
  {
    return size;
  }

  //----------------------------------------------------------------------------//
  const CEGUI::Size& Texture::getOriginalDataSize() const
  {
    return dataSize;
  }

  //----------------------------------------------------------------------------//
  const CEGUI::Vector2& Texture::getTexelScaling() const
  {
    return texelScaling;
  }

  //----------------------------------------------------------------------------//
  void Texture::loadFromFile (const CEGUI::String &filename, 
                                const CEGUI::String& /*resourceGroup*/)
  {
    csRef<iLoader> loader = csQueryRegistry<iLoader> (obj_reg);
    if (!loader)
      return;

    iTextureWrapper* txt = loader->LoadTexture(filename.c_str(), filename.c_str());
    if(!txt)
      return;

    hTxt = txt->GetTextureHandle();
    hTxt->SetTextureClass ("cegui");

    updateCachedSizeValues();
    updateCachedScaleValues();
  }

  //----------------------------------------------------------------------------//
  void Texture::loadFromMemory (const void *buffPtr, 
                                  const CEGUI::Size& buffer_size, 
                                  CEGUI::Texture::PixelFormat pixFmt)
  {
    csRef<iGraphics3D> g3d = csQueryRegistry<iGraphics3D> (obj_reg);
    if (!g3d)
      return;

    csRef<csImageMemory> image;
    // this should never happen as CEGUI itself will only ask for RGBA
    if (pixFmt != CEGUI::Texture::PF_RGBA)
      return;
    // TODO: need to round to the closer 'int'?
    image.AttachNew(new csImageMemory ((int) buffer_size.d_width, (int)buffer_size.d_height,
				       buffPtr, CS_IMGFMT_TRUECOLOR | CS_IMGFMT_ALPHA));

    iTextureManager* txtmgr = g3d->GetTextureManager();
    if (txtmgr)
    {
      /* Hack: assume memory textures are for fonts only; disable filtering
      * to have them look a bit crisper */
      hTxt = txtmgr->RegisterTexture (image, CS_TEXTURE_2D | CS_TEXTURE_NOFILTER);
      hTxt->SetTextureClass ("cegui");
    }

    size.d_width = image->GetWidth();
    size.d_height = image->GetHeight();
    dataSize = buffer_size;
    updateCachedScaleValues();
  }

  //----------------------------------------------------------------------------//
  void Texture::saveToMemory(void* buffer)
  {
    // @@@: Not implemented yet
  }

  //----------------------------------------------------------------------------//
  CEGUI::Renderer* Texture::getRenderer () const
  {
    return renderer;
  }

  //----------------------------------------------------------------------------//
  void Texture::updateCachedSizeValues()
  {
    int w, h;
    hTxt->GetRendererDimensions(w, h);
    size.d_width = w;
    size.d_height = h;
    dataSize = size;
  }

  //----------------------------------------------------------------------------//
  void Texture::updateCachedScaleValues()
  {
    //
    // calculate what to use for x scale
    //
    const float orgW = dataSize.d_width;
    const float texW = size.d_width;

    // if texture and original data width are the same, scale is based
    // on the original size.
    // if texture is wider (and source data was not stretched), scale
    // is based on the size of the resulting texture.
    texelScaling.d_x = 1.0f / ((orgW == texW) ? orgW : texW);

    //
    // calculate what to use for y scale
    //
    const float orgH = dataSize.d_height;
    const float texH = size.d_height;

    // if texture and original data height are the same, scale is based
    // on the original size.
    // if texture is taller (and source data was not stretched), scale
    // is based on the size of the resulting texture.
    texelScaling.d_y = 1.0f / ((orgH == texH) ? orgH : texH);
  }

  //----------------------------------------------------------------------------//
  iTextureHandle* Texture::GetTexHandle () const
  {
    return hTxt;
  }

  //----------------------------------------------------------------------------//
  void Texture::SetTexHandle(iTextureHandle* handle)
  {
    hTxt = handle;

    if (hTxt)
      updateCachedSizeValues();
    else
      size = dataSize = CEGUI::Size(0, 0);

    updateCachedScaleValues();
  }

  //----------------------------------------------------------------------------//

} CS_PLUGIN_NAMESPACE_END(cegui)
